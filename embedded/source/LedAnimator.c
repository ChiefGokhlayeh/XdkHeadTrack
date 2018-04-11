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
#define BCDS_MODULE_ID	APP_MODULE_LEDANIMATOR

#include "XdkLedAnimator.h"

#include "BCDS_Assert.h"
#include "BCDS_BSP_LED.h"
#include "BSP_BoardType.h"
#include "FreeRTOS.h"
#include "projdefs.h"
#include "portmacro.h"
#include "timers.h"
#include "XdkLogger.h"

#define BOOL_TO_CMD(b)	((b) ? (BSP_LED_COMMAND_ON) : (BSP_LED_COMMAND_OFF))

static void HandleAnimationTimer(TimerHandle_t timer);
static Retcode_T HandleAnimationTick(void);

static TimerHandle_t AnimationTimer = NULL;
static const LedAnimator_Animation_T* Animation = NULL;
static uint32_t CurrentStepIndex = 0;

static inline Retcode_T StopTimer(void)
{
	BaseType_t stopped = xTimerStop(AnimationTimer, portMAX_DELAY);
	if (pdTRUE != stopped)
	{
		return RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_RTOS_QUEUE_ERROR);
	}
	else
	{
		return RETCODE_OK;
	}
}

static inline Retcode_T StartTimer(void)
{
	BaseType_t started = xTimerStart(AnimationTimer, portMAX_DELAY);
	if (pdTRUE != started)
	{
		return RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_RTOS_QUEUE_ERROR);
	}
	else
	{
		return RETCODE_OK;
	}
}

static void HandleAnimationTimer(TimerHandle_t timer)
{
	BCDS_UNUSED(timer);

	Retcode_T rc = RETCODE_OK;

	rc = HandleAnimationTick();
	if (RETCODE_OK != rc)
	{
		Retcode_RaiseError(rc);
	}
}

static Retcode_T HandleAnimationTick(void)
{
	Retcode_T rc = RETCODE_OK;

	assert(Animation != NULL);
	assert(Animation->Steps != NULL);
	assert(Animation->StepCount > CurrentStepIndex);

	BaseType_t periodChanged = xTimerChangePeriod(AnimationTimer,
			pdMS_TO_TICKS(Animation->Steps[CurrentStepIndex].HoldTime), 0U);
	if (pdTRUE != periodChanged)
	{
		rc = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_RTOS_QUEUE_ERROR);
	}

	if (RETCODE_OK == rc)
	{
		rc = BSP_LED_Switch(BSP_XDK_LED_R,
				BOOL_TO_CMD(Animation->Steps[CurrentStepIndex].Red));
	}

	if (RETCODE_OK == rc)
	{
		rc = BSP_LED_Switch(BSP_XDK_LED_O,
				BOOL_TO_CMD(Animation->Steps[CurrentStepIndex].Orange));
	}

	if (RETCODE_OK == rc)
	{
		rc = BSP_LED_Switch(BSP_XDK_LED_Y,
				BOOL_TO_CMD(Animation->Steps[CurrentStepIndex].Yellow));
	}

	if (RETCODE_OK == rc)
	{
		CurrentStepIndex = (CurrentStepIndex + 1) % Animation->StepCount;

		/* If the animation has bencompleted, check how to handle the loop */
		if (0U == CurrentStepIndex)
		{
			switch (Animation->LoopBehavior)
			{
			case LED_ANIMATOR_LOOP_HOLD_LAST:
				rc = StopTimer();
				break;
			case LED_ANIMATOR_LOOP_CONTINUE:
				/* Do nothing. Let the loop continue. */
				break;
			default:
				rc = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_INCONSITENT_STATE);
				break;
			}
		}
	}

	return rc;
}

Retcode_T LedAnimator_Initialize(void)
{
	Retcode_T rc = RETCODE_OK;

	rc = BSP_LED_Connect();

	if (RETCODE_OK == rc)
	{
		rc = BSP_LED_EnableAll();
	}

	if (RETCODE_OK == rc)
	{
		if (NULL == AnimationTimer)
		{
			AnimationTimer = xTimerCreate("BLINK", pdMS_TO_TICKS(1), pdTRUE,
			NULL, HandleAnimationTimer);
			if (NULL == AnimationTimer)
			{
				rc = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_OUT_OF_RESOURCES);
			}
		}
	}

	return rc;
}

Retcode_T LedAnimator_PlayAnimation(const LedAnimator_Animation_T* animation)
{
	Retcode_T rc = RETCODE_OK;

	if (NULL == AnimationTimer)
	{
		rc = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED);
		goto exit;
	}

	if (NULL == animation || NULL == animation->Steps
			|| LED_ANIMATOR_LOOP_MAX <= animation->LoopBehavior)
	{
		rc = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
		goto exit;
	}

	if (RETCODE_OK == rc)
	{
		rc = LedAnimator_StopAllAnimations();
	}

	if (RETCODE_OK == rc)
	{
		Animation = animation;
		CurrentStepIndex = 0;

		rc = HandleAnimationTick();
	}

	if (RETCODE_OK == rc)
	{
		rc = StartTimer();
	}

	exit: return rc;
}

Retcode_T LedAnimator_StopAllAnimations(void)
{
	Retcode_T rc = RETCODE_OK;

	if (NULL == AnimationTimer)
	{
		rc = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED);
		goto exit;
	}

	if (RETCODE_OK == rc)
	{
		rc = StopTimer();
	}

	if (RETCODE_OK == rc)
	{
		Animation = NULL;
		CurrentStepIndex = 0;
	}

	exit: return rc;
}

Retcode_T LedAnimator_Deinitialize(void)
{
	Retcode_T rc = RETCODE_OK;

	rc = LedAnimator_StopAllAnimations();

	if (RETCODE_OK == rc && NULL != AnimationTimer)
	{
		BaseType_t deleted = xTimerDelete(AnimationTimer, portMAX_DELAY);
		if (pdTRUE != deleted)
		{
			rc = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_RTOS_QUEUE_ERROR);
		}
	}

	if (RETCODE_OK == rc)
	{
		AnimationTimer = NULL;
	}

	return rc;
}
