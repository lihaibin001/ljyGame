#include "led.h"
#include "FreeRTOS.h"
#include "timers.h"

static TimerHandle_t xTimers;

static void vTimerCallback(TimerHandle_t xTimer)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_11, !GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_11));
}

void LED_Init(void)
{
	xTimers = xTimerCreate("at24qtchecker", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, vTimerCallback);
	xTimerStart(xTimers, 0);
}
 
