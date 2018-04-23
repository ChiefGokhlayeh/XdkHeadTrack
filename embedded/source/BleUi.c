/* Copyright 2018 freeg
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
#define BCDS_MODULE_ID	APP_MODULE_BLEUI

#include "XdkBleUi.h"

#include "XdkHeadTrack.h"
#include "XdkLogger.h"

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"

#include "BCDS_Ble.h"
#include "BCDS_BlePeripheral.h"
#include "BCDS_BidirectionalService.h"
#include "BCDS_CmdProcessor.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define BLE_UI_DEVICE_NAME		("XdkHeadTrack")
#define BLE_UI_STARTUP_TIMEOUT	(pdMS_TO_TICKS(2000))
#define BLE_UI_WAKEUP_TIMEOUT	(pdMS_TO_TICKS(1000))
#define BLE_UI_SEND_TIMEOUT		(pdMS_TO_TICKS(500))

static const CmdProcessor_T* CmdProcessor;

static SemaphoreHandle_t DataSentSignal;
static SemaphoreHandle_t PowerModeChangedSignal;
static SemaphoreHandle_t SleepStateChangedSignal;
static SemaphoreHandle_t ConnectionStateChangedSignal;

static bool IsBleStarted;
static bool IsBleAwake;
static bool IsBleConnected;

static Retcode_T SetupBle(void);
static void HandleBlePeripheralEvent(BlePeripheral_Event_T event, void* data);
static Retcode_T HandleServiceRegistryCallback(void);
static void HandleBleSentCallback(Retcode_T sendStatus);
static inline Retcode_T CreateSignal(SemaphoreHandle_t* signal);
static void HandleBleDataReceivedCallback(uint8_t* rxBuffer,
		uint8_t rxDataLength);
static inline Retcode_T WaitForSignal(SemaphoreHandle_t signal,
		TickType_t timeout);

static inline Retcode_T CreateSignal(SemaphoreHandle_t* signal)
{
	assert(NULL != signal);
	if (NULL == *signal)
	{
		*signal = xSemaphoreCreateBinary();
		if (NULL == *signal)
		{
			return RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_OUT_OF_RESOURCES);
		}
		else
		{
			return RETCODE_OK;
		}
	}
	else
	{
		return RETCODE_OK;
	}
}

static inline Retcode_T WaitForSignal(SemaphoreHandle_t signal,
		TickType_t timeout)
{
	return xSemaphoreTake(signal, timeout) == pdPASS ?
			RETCODE_OK : RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_TIMEOUT);
}

static void HandleBlePeripheralEvent(BlePeripheral_Event_T event, void* data)
{
	BCDS_UNUSED(data);
	switch (event)
	{
	case BLE_PERIPHERAL_STARTED:
		LOG_DEBUG("BLE Started");
		IsBleStarted = true;
		(void) xSemaphoreGive(PowerModeChangedSignal);
		break;
	case BLE_PERIPHERAL_SERVICES_REGISTERED:
		break;
	case BLE_PERIPHERAL_SLEEP_SUCCEEDED:
		LOG_DEBUG("BLE went to sleep");
		IsBleAwake = false;
		(void) xSemaphoreGive(SleepStateChangedSignal);
		break;
	case BLE_PERIPHERAL_WAKEUP_SUCCEEDED:
		LOG_DEBUG("BLE woke up");
		IsBleAwake = true;
		(void) xSemaphoreGive(SleepStateChangedSignal);
		break;
	case BLE_PERIPHERAL_CONNECTED:
		LOG_DEBUG("BLE connected");
		IsBleConnected = true;
		(void) xSemaphoreGive(ConnectionStateChangedSignal);
		HeadTrack_ChangeCommunicationMode(HEAD_TRACK_COMMUNICATION_MODE_BLE);
		break;
	case BLE_PERIPHERAL_DISCONNECTED:
		LOG_DEBUG("BLE disconnected");
		IsBleConnected = false;
		(void) xSemaphoreGive(ConnectionStateChangedSignal);
		HeadTrack_ChangeCommunicationMode(HEAD_TRACK_COMMUNICATION_MODE_SERIAL);
		break;
	case BLE_PERIPHERAL_ERROR:
		LOG_ERROR("BLE Error");
		Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE));
		break;
	default:
	case BLE_PERIPHERAL_EVENT_MAX:
		LOG_FATAL("Unexpected BLE event");
		Retcode_RaiseError(
				RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_INCONSITENT_STATE));
		break;
	}
}

static void HandleBleSentCallback(Retcode_T sendStatus)
{
	assert(NULL != DataSentSignal);
	if (RETCODE_OK != sendStatus)
	{
		Retcode_RaiseError(sendStatus);
	}
	(void) xSemaphoreGive(DataSentSignal);
}

static void HandleBleDataReceivedCallback(uint8_t* rxBuffer,
		uint8_t rxDataLength)
{
	BCDS_UNUSED(rxBuffer);
	BCDS_UNUSED(rxDataLength);
}

static Retcode_T HandleServiceRegistryCallback(void)
{
	Retcode_T rc = RETCODE_OK;

	rc = BidirectionalService_Init(HandleBleDataReceivedCallback,
			HandleBleSentCallback);
	if (RETCODE_OK == rc)
	{
		rc = BidirectionalService_Register();
	}

	return rc;
}

static Retcode_T SetupBle(void)
{
	Retcode_T rc = RETCODE_OK;

	rc = BlePeripheral_Initialize(HandleBlePeripheralEvent,
			HandleServiceRegistryCallback);

	if (RETCODE_OK == rc)
	{
		rc = BlePeripheral_SetDeviceName((uint8_t*) BLE_UI_DEVICE_NAME);
	}

	if (RETCODE_OK == rc)
	{
		rc = BlePeripheral_Start();
	}

	if (RETCODE_OK == rc)
	{
		rc = WaitForSignal(PowerModeChangedSignal, BLE_UI_STARTUP_TIMEOUT);
	}

	if (RETCODE_OK == rc)
	{
		rc = BlePeripheral_Wakeup();
	}

	if (RETCODE_OK == rc)
	{
		rc = WaitForSignal(SleepStateChangedSignal, BLE_UI_WAKEUP_TIMEOUT);
	}

	return rc;
}

Retcode_T BleUi_Initialize(const CmdProcessor_T* cmdProcessor)
{
	Retcode_T rc = RETCODE_OK;

	if (NULL == cmdProcessor)
	{
		rc = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
	}

	if (RETCODE_OK == rc)
	{
		rc = CreateSignal(&PowerModeChangedSignal);
	}

	if (RETCODE_OK == rc)
	{
		rc = CreateSignal(&SleepStateChangedSignal);
	}

	if (RETCODE_OK == rc)
	{
		rc = CreateSignal(&ConnectionStateChangedSignal);
	}

	if (RETCODE_OK == rc)
	{
		rc = CreateSignal(&DataSentSignal);
	}

	if (RETCODE_OK == rc)
	{
		rc = SetupBle();
	}

	if (RETCODE_OK == rc)
	{
		CmdProcessor = cmdProcessor;
	}

	return rc;
}

Retcode_T BleUi_SendTrackingData(const BleUi_TrackingData_T* data)
{
	Retcode_T rc = RETCODE_OK;

	if (IsBleConnected)
	{
		rc = BidirectionalService_SendData((uint8_t*) &data,
				sizeof(BleUi_TrackingData_T));
		if (RETCODE_OK == rc)
		{
			rc = WaitForSignal(DataSentSignal, BLE_UI_SEND_TIMEOUT);
		}
	}

	return rc;
}

Retcode_T BleUi_Deinitialize(void)
{
	Retcode_T rc = RETCODE_OK;

	CmdProcessor = NULL;

	rc = BlePeripheral_Deinitialize();

	return rc;
}
