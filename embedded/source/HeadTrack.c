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

#include "XdkButtonUi.h"
#include "XdkLedAnimator.h"
#include "XdkLogger.h"

#define APP_POLL_ROTATION_TASK_STACK_SIZE	(UINT32_C(300))
#define APP_POLL_ROTATION_TASK_PRIO			(UINT32_C(4))

static const LedAnimator_Step_T InitializingSteps[] =
{
{ true, false, false, 500 },
{ false, false, false, 500 } };

static const LedAnimator_Animation_T InitializingAnimation =
{ InitializingSteps, 2, LED_ANIMATOR_LOOP_CONTINUE };

static const LedAnimator_Step_T TrackingSteps[] =
{
{ true, false, false, 1 } };

static const LedAnimator_Animation_T TrackingAnimation =
{ TrackingSteps, 1, LED_ANIMATOR_LOOP_HOLD_LAST };

static const LedAnimator_Step_T IdleSteps[] =
{
{ true, false, false, 500 },
{ false, false, false, 2000 } };

static const LedAnimator_Animation_T IdleAnimation =
{ IdleSteps, 2, LED_ANIMATOR_LOOP_CONTINUE };

static void RunPollRotationLoop(void* param1);

static const CmdProcessor_T* AppCmdProcessor = NULL;
static TaskHandle_t PollRotationTask = NULL;
static SemaphoreHandle_t PollRotationRunSignal = NULL;
static bool IsPollRotationEnabled = false;
static bool IsCalibrationRequested = false;

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
			printf(">>CALI: %f %f %f %f\n", rawRotation.w, rawRotation.x,
					rawRotation.y, rawRotation.z);
		}

		if (RETCODE_OK == rc)
		{
			printf(">>QUAT: %f %f %f %f\n", rawRotation.w, rawRotation.x,
					rawRotation.y, rawRotation.z);
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
		rc = LedAnimator_PlayAnimation(&TrackingAnimation);
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
