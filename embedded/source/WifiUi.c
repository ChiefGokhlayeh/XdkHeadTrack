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
#define BCDS_MODULE_ID APP_MODULE_WIFIUI

#include "XdkWifiUi.h"

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_NetworkConfig.h"
#include "BCDS_WlanConnect.h"

#include "simplelink.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define WIFI_UI_CONNECT_TIMEOUT	(pdMS_TO_TICKS(30000))

static const CmdProcessor_T* CmdProcessor;
static SemaphoreHandle_t ConnectionStatusChangedSignal;

static inline Retcode_T CreateSignal(SemaphoreHandle_t* signal);
static void HandleWlanConnectEvent(WlanConnect_Status_T event);
static Retcode_T SetupWifi(void);
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

static void HandleWlanConnectEvent(WlanConnect_Status_T event)
{
	switch (event)
	{
	case WLAN_CONNECTED:
	case WLAN_DISCONNECTED:
		(void) xSemaphoreGive(ConnectionStatusChangedSignal);
		break;
	default:
		break;
	}
}

static Retcode_T SetupWifi(void)
{
	Retcode_T rc = RETCODE_OK;

	rc = WlanConnect_Init();

	if (RETCODE_OK == rc)
	{
		rc = NetworkConfig_SetIpDhcp(NULL);
	}

	if (RETCODE_OK == rc)
	{
		/* TODO: Currenctly only supporting WPS - push button method, as it's the
		 * mosty handy one and doesn't need any configuration.
		 * A configuration manager should be implemented that reads the Wifi
		 * config from a file on the SD card.
		 */
		rc = WlanConnect_WPS_PBC(HandleWlanConnectEvent);
	}

	if (RETCODE_OK == rc)
	{
		rc = WaitForSignal(ConnectionStatusChangedSignal,
		WIFI_UI_CONNECT_TIMEOUT);
	}

	return rc;
}

Retcode_T WifiUi_Initialize(const CmdProcessor_T* cmdProcessor)
{
	Retcode_T rc = RETCODE_OK;

	rc = CreateSignal(&ConnectionStatusChangedSignal);

	if (RETCODE_OK == rc)
	{
		rc = SetupWifi();
	}

	if (RETCODE_OK == rc)
	{
		CmdProcessor = cmdProcessor;
	}

	return rc;
}

Retcode_T WifiUi_SendTrackingData(const WifiUi_TrackingData_T* data)
{
	BCDS_UNUSED(data);
	Retcode_T rc = RETCODE_OK;

	rc = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);

	return rc;
}

Retcode_T WifiUi_Deinitialize(void)
{
	Retcode_T rc = RETCODE_OK;

	rc = WlanConnect_Disconnect(NULL);

	if (RETCODE_OK == rc)
	{
		rc = WlanConnect_DeInit();
	}

	return rc;
}
