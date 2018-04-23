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
#ifndef XDKHEADTRACK_H_
#define XDKHEADTRACK_H_

#include "BCDS_Basics.h"
#include "BCDS_CmdProcessor.h"

#define APP_CMD_PROCESSOR_PRIO		(UINT32_C(1))
#define APP_CMD_PROCESSOR_STACK		(UINT16_C(700))
#define APP_CMD_PROCESSOR_QUEUE_LEN	(UINT32_C(10))

enum HeadTrack_CommunicationMode_E
{
	HEAD_TRACK_COMMUNICATION_MODE_SERIAL, HEAD_TRACK_COMMUNICATION_MODE_BLE,

	HEAD_TRACK_COMMUNICATION_MODE_MAX
};
typedef enum HeadTrack_CommunicationMode_E HeadTrack_CommunicationMode_T;

void HeadTrack_InitSystem(void* cmdProcessorHandle, uint32_t param2);

Retcode_T HeadTrack_Run(void);

Retcode_T HeadTrack_Stop(void);

Retcode_T HeadTrack_Calibrate(void);

Retcode_T HeadTrack_ChangeCommunicationMode(
		HeadTrack_CommunicationMode_T commMode);

#endif /* XDKHEADTRACK_H_ */
