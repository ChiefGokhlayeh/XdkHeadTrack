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
#ifndef XDKBLEUI_H_
#define XDKBLEUI_H_

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"

#include "BCDS_CmdProcessor.h"

#pragma pack(push, 1)
struct BleUi_TrackingData_S
{
	float W;
	float X;
	float Y;
	float Z;
	bool UseForCalibration;
};
#pragma pack(pop)
typedef struct BleUi_TrackingData_S BleUi_TrackingData_T;

Retcode_T BleUi_Initialize(const CmdProcessor_T* cmdProcessor);

Retcode_T BleUi_SendTrackingData(const BleUi_TrackingData_T* data);

Retcode_T BleUi_Deinitialize(void);

#endif /* XDKBLEUI_H_ */
