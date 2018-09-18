#include "at24qtxx.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "ws2812b.h"
#include "rand.h"
#include "string.h"
#define NUM_LEDS    16

RGB_t leds[NUM_LEDS];

static TimerHandle_t xTimers;
static bool touchButtonSta;
static void vTimerCallback(TimerHandle_t xTimer)
{
	uint8_t i;
	static uint8_t debouching;
	uint8_t stat = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9);

	if(stat)
	{
		if(touchButtonSta == true)
		{
			debouching = 0;
		}
		else
		{
			if(++debouching == 3)
			{
				touchButtonSta = true;
				debouching = 0;
			}
		}
	}
	else
	{
		if(touchButtonSta == false)
		{
			while(!ws2812b_IsReady());  // wait
			for(i = 0; i <= 15; i++)
			{
				leds[i].b = 0;
				leds[i].g = 0;
				leds[i].r = 0;
			}
			ws2812b_SendRGB(leds, NUM_LEDS);
			debouching = 0;
		}
		else
		{
			if(++debouching == 3)
			{
				touchButtonSta = false;
				debouching = 0;
			}
		}
	}
	if(touchButtonSta)
	{
		static uint8_t count;
		if(count++ == 10)
		{
			count = 0;
			while(!ws2812b_IsReady());  // wait
			for(i = 0; i <= 15; i++)
			{
				leds[i].b = rand() / 255;
				leds[i].g = rand() / 255;
				leds[i].r = rand() / 255;
			}
			ws2812b_SendRGB(leds, NUM_LEDS);
		}
	}
}

void at24qt_intit(void)
{
	xTimers = xTimerCreate("at24qtchecker", pdMS_TO_TICKS(20), pdTRUE, (void*)0, vTimerCallback);
	xTimerStart(xTimers, 0);
}

bool at24qt_getOutPutStatus(void)
{
	return  touchButtonSta;
}
void at24qt_setMode(at24qtxxMode_t mode)
{
	switch((uint32_t)mode)
	{
		case at24qtxx_fastMode:
			PCout(8) = 1;
			break;
		case at24qtxx_lowpowerMode:
			PCout(8) = 0;
			break;
		default:
			break;
	}
	//PCout(8) = mode;
}
