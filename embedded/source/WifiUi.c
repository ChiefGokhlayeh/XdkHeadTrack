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

enum
{
	WIFIUI_MODE_OPEN = 0,
	WIFIUI_MODE_WEP,
	WIFIUI_MODE_WPA,
	WIFIUI_MODE_WPSPB,
	WIFIUI_MODE_WPSPIN
};

#define WIFIUI_CONNECT_TIMEOUT	(pdMS_TO_TICKS(30000))

#ifndef APP_WIFI_MODE
/* To be set in makefile */
#define APP_WIFI_MODE	(WIFIUI_MODE_WPSPB)
#endif
#ifndef APP_WIFI_SSID
/* To be set in makefile */
#define APP_WIFI_SSID		("SSID")
#endif
#ifndef APP_WIFI_PASSPHRASE
/* To be set in makefile */
#define APP_WIFI_PASSPHRASE	("PASSWORD")
#endif

static const CmdProcessor_T* CmdProcessor;
static SemaphoreHandle_t ConnectionStatusChangedSignal;
static SemaphoreHandle_t NetworkStatusChangedSignal;

static inline Retcode_T CreateSignal(SemaphoreHandle_t* signal);
static void HanldeNetworkConfigEvent(NetworkConfig_IpStatus_T event);
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

static void HanldeNetworkConfigEvent(NetworkConfig_IpStatus_T event)
{
	switch (event)
	{
	case NETWORKCONFIG_IPV4_ACQUIRED:
	case NETWORKCONFIG_IPV6_ACQUIRED:
	case NETWORKCONFIG_IP_NOT_ACQUIRED:
		(void) xSemaphoreGive(NetworkStatusChangedSignal);
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
		rc = NetworkConfig_SetIpDhcp(HanldeNetworkConfigEvent);
	}

	if (RETCODE_OK == rc)
	{
		switch (APP_WIFI_MODE)
		{
		case WIFIUI_MODE_OPEN:
			rc = WlanConnect_Open((WlanConnect_SSID_T) APP_WIFI_SSID,
					HandleWlanConnectEvent);
			break;
		case WIFIUI_MODE_WEP:
			rc = WlanConnect_WEP_Open((WlanConnect_SSID_T) APP_WIFI_SSID,
					(WlanConnect_PassPhrase_T) APP_WIFI_PASSPHRASE,
					strlen(APP_WIFI_PASSPHRASE), HandleWlanConnectEvent);
			break;
		case WIFIUI_MODE_WPA:
			rc = WlanConnect_WPA((WlanConnect_SSID_T) APP_WIFI_SSID,
					(WlanConnect_PassPhrase_T) APP_WIFI_PASSPHRASE,
					HandleWlanConnectEvent);
			break;
		case WIFIUI_MODE_WPSPB:
			rc = WlanConnect_WPS_PBC(HandleWlanConnectEvent);
			break;
		case WIFIUI_MODE_WPSPIN:
			rc = WlanConnect_WPS_PIN(HandleWlanConnectEvent);
			break;
		}
	}

	if (RETCODE_OK == rc)
	{
		rc = WaitForSignal(ConnectionStatusChangedSignal,
		WIFIUI_CONNECT_TIMEOUT);
	}

	if (RETCODE_OK == rc)
	{
		uint32_t retry = 0;
		NetworkConfig_IpStatus_T ipStatus = NetworkConfig_GetIpStatus();
		while (RETCODE_OK == rc && ipStatus == NETWORKCONFIG_IP_NOT_ACQUIRED)
		{
			rc = WaitForSignal(NetworkStatusChangedSignal,
			WIFIUI_CONNECT_TIMEOUT);
			if (RETCODE_TIMEOUT == Retcode_GetCode(rc))
			{
				rc = RETCODE_OK;
			}

			ipStatus = NetworkConfig_GetIpStatus();

			if (RETCODE_OK == rc && 2U <= retry++)
			{
				rc = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_TIMEOUT);
			}
		}
	}

	return rc;
}

Retcode_T WifiUi_Initialize(const CmdProcessor_T* cmdProcessor)
{
	Retcode_T rc = RETCODE_OK;

	rc = CreateSignal(&ConnectionStatusChangedSignal);

	if (RETCODE_OK == rc)
	{
		rc = CreateSignal(&NetworkStatusChangedSignal);
	}

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
