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
#define BCDS_MODULE_ID	APP_MODULE_BUTTONUI

#include "XdkButtonUi.h"

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_BSP_Button.h"
#include "BSP_BoardType.h"

#include "XdkHeadTrack.h"

static const CmdProcessor_T* CmdProcessor;

static void HandleButton1Interrupt(uint32_t status);
static void HandleButton1Event(void* param1, uint32_t status);

static void HandleButton1Interrupt(uint32_t status)
{
	Retcode_T rc = CmdProcessor_EnqueueFromIsr((CmdProcessor_T*) CmdProcessor,
	HandleButton1Event, NULL, status);
	if (RETCODE_OK != rc)
	{
		Retcode_RaiseErrorFromIsr(rc);
	}
}

static void HandleButton1Event(void* param1, uint32_t status)
{
	BCDS_UNUSED(param1);

	Retcode_T rc = RETCODE_OK;

	switch (status)
	{
	case BSP_XDK_BUTTON_PRESS:
		break;
	case BSP_XDK_BUTTON_RELEASE:
		rc = HeadTrack_Calibrate();
		break;
	default:
		break;
	}

	if (RETCODE_OK != rc)
	{
		Retcode_RaiseError(rc);
	}
}

Retcode_T ButtonUi_Initialize(const CmdProcessor_T* cmdProcessor)
{
	Retcode_T rc = RETCODE_OK;

	CmdProcessor = cmdProcessor;

	rc = BSP_Button_Connect();

	if (RETCODE_OK == rc)
	{
		rc = BSP_Button_Enable((uint32_t) BSP_XDK_BUTTON_1,
				HandleButton1Interrupt);
	}

	return rc;
}

Retcode_T ButtonUi_Deinitialize(void)
{
	Retcode_T rc = RETCODE_OK;

	rc = BSP_Button_Disable((uint32_t) BSP_XDK_BUTTON_1);

	if (RETCODE_OK == rc)
	{
		rc = BSP_Button_Disconnect();
	}

	return rc;
}
