#include "at24qtxx.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "ws2812b.h"
#include "rand.h"
#include "string.h"
#include "ws2812b_conf.h"
#include "canApp.h"

extern TimerHandle_t g_led_off_timer;
static TimerHandle_t xTimers;
static bool touchButtonSta;
static void vTimerCallback(TimerHandle_t xTimer) {
	uint8_t i;
	static uint8_t debouching;
	uint8_t stat = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9);

	if (stat && !touchButtonSta) {
		if (++debouching == 3) {
			touchButtonSta = true;
			debouching = 0;
		}
	} else if (!stat && touchButtonSta) {

		if (++debouching == 3) {
			touchButtonSta = false;
			debouching = 0;
		}
	} else {
		debouching = 0;
	}
	if (touchButtonSta) {
		static uint8_t count;
		if (count++ == 10) {
			count = 0;
			if (plate_status == 1) {
				if(xTimerIsTimerActive(g_led_off_timer) == pdFALSE) {
					xTimerStop(g_led_off_timer, 100);
				}
				plate_status = 0;

				while (!ws2812b_IsReady())
					;  // wait
				for (i = 0; i <= NUM_GRB_LEDS; i++) {
					leds[i].b = 0;
					leds[i].g = 0;
					leds[i].r = 0;
//				leds[i].b = rand() / 255;
//				leds[i].g = rand() / 255;
//				leds[i].r = rand() / 255;
				}
				ws2812b_SendRGB(leds, NUM_GRB_LEDS);
				can_frame_t frame;
				frame.dataByte0 = 0;
				frame.dataByte1 = plate_color;
				CanAppSendMsg(&frame);

			}
		}
	}
}

void at24qt_intit(void) {
	xTimers = xTimerCreate("at24qtchecker", pdMS_TO_TICKS(20), pdTRUE,
			(void*) 0, vTimerCallback);
	xTimerStart(xTimers, 0);
}

bool at24qt_getOutPutStatus(void) {
	return touchButtonSta;
}
void at24qt_setMode(at24qtxxMode_t mode) {
	switch ((uint32_t) mode) {
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
