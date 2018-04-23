/* Copyright 2018 Andreas Baulig
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "XdkApp.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID	APP_MODULE_HEADTRACK

#include "XdkHeadTrack.h"

#include <stdio.h>

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_Rotation.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "XdkSensorHandle.h"

#include "XdkBleUi.h"
#include "XdkButtonUi.h"
#include "XdkLedAnimator.h"
#include "XdkLogger.h"

#define APP_POLL_ROTATION_TASK_STACK_SIZE	(UINT32_C(300))
#define APP_POLL_ROTATION_TASK_PRIO			(UINT32_C(4))

#define HEAD_TRACK_DEFAULT_COMMUNICATION_MODE	(HEAD_TRACK_COMMUNICATION_MODE_SERIAL)

static const LedAnimator_Step_T InitializingSteps[] =
{
{ true, false, false, 500 },
{ false, false, false, 500 } };

static const LedAnimator_Animation_T InitializingAnimation =
{ InitializingSteps, 2, LED_ANIMATOR_LOOP_CONTINUE };

static const LedAnimator_Step_T TrackingOverSerialSteps[] =
{
{ true, false, false, 1 } };

static const LedAnimator_Animation_T TrackingOverSerialAnimation =
{ TrackingOverSerialSteps, 1, LED_ANIMATOR_LOOP_HOLD_LAST };

static const LedAnimator_Step_T TrackingOverBleSteps[] =
{
{ false, true, false, 1 } };

static const LedAnimator_Animation_T TrackingOverBleAnimation =
{ TrackingOverBleSteps, 1, LED_ANIMATOR_LOOP_HOLD_LAST };

static const LedAnimator_Step_T IdleSteps[] =
{
{ true, false, false, 500 },
{ false, false, false, 2000 } };

static const LedAnimator_Animation_T IdleAnimation =
{ IdleSteps, 2, LED_ANIMATOR_LOOP_CONTINUE };

static void RunPollRotationLoop(void* param1);
static inline Retcode_T SendViaBle(const Rotation_QuaternionData_T* rawRotation,
bool useForCalibration);
static inline Retcode_T SendViaSerial(
		const Rotation_QuaternionData_T* rawRotation, bool useForCalibration);
static Retcode_T UpdateLedAnimationToMode(void);

static const CmdProcessor_T* AppCmdProcessor = NULL;
static TaskHandle_t PollRotationTask = NULL;
static SemaphoreHandle_t PollRotationRunSignal = NULL;
static bool IsPollRotationEnabled = false;
static bool IsCalibrationRequested = false;
static HeadTrack_CommunicationMode_T CommunicationMode =
HEAD_TRACK_DEFAULT_COMMUNICATION_MODE;

static inline Retcode_T SendViaBle(const Rotation_QuaternionData_T* rawRotation,
bool useForCalibration)
{
	BleUi_TrackingData_T bleData;
	bleData.W = rawRotation->w;
	bleData.X = rawRotation->x;
	bleData.Y = rawRotation->y;
	bleData.Z = rawRotation->z;
	bleData.UseForCalibration = useForCalibration;
	return BleUi_SendTrackingData(&bleData);
}

static Retcode_T UpdateLedAnimationToMode(void)
{
	Retcode_T rc = RETCODE_OK;
	switch (CommunicationMode)
	{
	case HEAD_TRACK_COMMUNICATION_MODE_SERIAL:
		rc = LedAnimator_PlayAnimation(&TrackingOverSerialAnimation);
		break;
	case HEAD_TRACK_COMMUNICATION_MODE_BLE:
		rc = LedAnimator_PlayAnimation(&TrackingOverBleAnimation);
		break;
	default:
		rc = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
		break;
	}
	return rc;
}

static inline Retcode_T SendViaSerial(
		const Rotation_QuaternionData_T* rawRotation, bool useForCalibration)
{
	if (useForCalibration)
	{
		printf(">>CALI: %f %f %f %f\n", rawRotation->w, rawRotation->x,
				rawRotation->y, rawRotation->z);
	}
	else
	{
		printf(">>QUAT: %f %f %f %f\n", rawRotation->w, rawRotation->x,
				rawRotation->y, rawRotation->z);
	}
	return RETCODE_OK;
}

static void RunPollRotationLoop(void* param1)
{
	BCDS_UNUSED(param1);
	Retcode_T rc = RETCODE_OK;
	Rotation_QuaternionData_T rawRotation;
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();

	while (1)
	{
		while (!IsPollRotationEnabled)
		{
			(void) xSemaphoreTake(PollRotationRunSignal, portMAX_DELAY);
		}

		rc = Rotation_readQuaternionValue(&rawRotation);

		if (IsCalibrationRequested)
		{
			IsCalibrationRequested = false;
			switch (CommunicationMode)
			{
			case HEAD_TRACK_COMMUNICATION_MODE_SERIAL:
				rc = SendViaSerial(&rawRotation, true);
				break;
			case HEAD_TRACK_COMMUNICATION_MODE_BLE:
				rc = SendViaBle(&rawRotation, true);
				break;
			default:
				Retcode_RaiseError(
						RETCODE(RETCODE_SEVERITY_FATAL,
								RETCODE_INCONSITENT_STATE));
				break;
			}
		}

		if (RETCODE_OK == rc)
		{
			switch (CommunicationMode)
			{
			case HEAD_TRACK_COMMUNICATION_MODE_SERIAL:
				SendViaSerial(&rawRotation, false);
				break;
			case HEAD_TRACK_COMMUNICATION_MODE_BLE:
				rc = SendViaBle(&rawRotation, false);
				break;
			default:
				Retcode_RaiseError(
						RETCODE(RETCODE_SEVERITY_FATAL,
								RETCODE_INCONSITENT_STATE));
				break;
			}
			vTaskDelayUntil(&pxPreviousWakeTime, pdMS_TO_TICKS(20));
		}

		if (RETCODE_OK != rc)
		{
			Retcode_RaiseError(rc);
		}
	}
}

void HeadTrack_InitSystem(void* cmdProcessorHandle, uint32_t param2)
{
	BCDS_UNUSED(param2);
	assert(NULL != cmdProcessorHandle);

	AppCmdProcessor = cmdProcessorHandle;

	Retcode_T rc = RETCODE_OK;

	rc = Logger_Initialize();

	if (RETCODE_OK == rc)
	{
		rc = LedAnimator_Initialize();
	}

	if (RETCODE_OK == rc)
	{
		rc = LedAnimator_PlayAnimation(&InitializingAnimation);
	}

	if (RETCODE_OK == rc)
	{
		rc = ButtonUi_Initialize(AppCmdProcessor);
	}

	if (RETCODE_OK == rc)
	{
		rc = BleUi_Initialize(AppCmdProcessor);
	}

	if (RETCODE_OK == rc)
	{
		rc = Rotation_init(xdkRotationSensor_Handle);
	}

	if (RETCODE_OK == rc)
	{
		if (NULL == PollRotationRunSignal)
		{
			PollRotationRunSignal = xSemaphoreCreateBinary();
			if (NULL == PollRotationRunSignal)
			{
				rc = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_OUT_OF_RESOURCES);
			}
		}
	}

	if (RETCODE_OK == rc)
	{
		if (NULL == PollRotationTask)
		{
			BaseType_t taskCreated = xTaskCreate(RunPollRotationLoop,
					"POLL_ROTATION", APP_POLL_ROTATION_TASK_STACK_SIZE, NULL,
					APP_POLL_ROTATION_TASK_PRIO, PollRotationTask);
			if (pdTRUE != taskCreated)
			{
				rc = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_OUT_OF_RESOURCES);
			}
		}
	}

	if (RETCODE_OK == rc)
	{
		rc = HeadTrack_Run();
	}

	if (RETCODE_OK != rc)
	{
		Retcode_RaiseError(rc);
	}
}

Retcode_T HeadTrack_Run(void)
{
	Retcode_T rc = RETCODE_OK;

	IsPollRotationEnabled = true;
	(void) xSemaphoreGive(PollRotationRunSignal);

	if (RETCODE_OK == rc)
	{
		rc = UpdateLedAnimationToMode();
	}

	return rc;
}

Retcode_T HeadTrack_Stop(void)
{
	Retcode_T rc = RETCODE_OK;

	IsPollRotationEnabled = false;

	rc = LedAnimator_PlayAnimation(&IdleAnimation);

	return rc;
}

Retcode_T HeadTrack_Calibrate(void)
{
	Retcode_T rc = RETCODE_OK;

	IsCalibrationRequested = true;

	return rc;
}

Retcode_T HeadTrack_ChangeCommunicationMode(
		HeadTrack_CommunicationMode_T commMode)
{
	Retcode_T rc = RETCODE_OK;

	CommunicationMode = commMode;

	if (IsPollRotationEnabled)
	{
		rc = UpdateLedAnimationToMode();
	}

	return rc;
}
