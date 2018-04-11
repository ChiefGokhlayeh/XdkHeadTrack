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

#ifndef XDKLEDANIMATOR_H_
#define XDKLEDANIMATOR_H_

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"

struct LedAnimator_Step_S
{
	bool Red;
	bool Orange;
	bool Yellow;
	uint32_t HoldTime;
};
typedef struct LedAnimator_Step_S LedAnimator_Step_T;

enum LedAnimator_LoopBehavior_E
{
	LED_ANIMATOR_LOOP_HOLD_LAST,
	LED_ANIMATOR_LOOP_CONTINUE,
	LED_ANIMATOR_LOOP_MAX,
};
typedef enum LedAnimator_LoopBehavior_E LedAnimator_LoopBehavior_T;

struct LedAnimator_Animation_S
{
	const LedAnimator_Step_T* Steps;
	uint32_t StepCount;
	LedAnimator_LoopBehavior_T LoopBehavior;
};
typedef struct LedAnimator_Animation_S LedAnimator_Animation_T;

Retcode_T LedAnimator_Initialize(void);

Retcode_T LedAnimator_PlayAnimation(const LedAnimator_Animation_T* animation);

Retcode_T LedAnimator_StopAllAnimations(void);

Retcode_T LedAnimator_Deinitialize(void);

#endif /* XDKLEDANIMATOR_H_ */
