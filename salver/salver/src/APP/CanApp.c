#include "CanApp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "sys.h"
#include "ws2812b.h"
#include "ws2812b_conf.h"
#ifdef CAN_APP_DEBUG
#define DEBUG(...) printf()
#else
#define DEBUG(...)
#endif

#ifdef CAN_ERROR_DEBUG
#define ERROR_DEBUG(...) printf()
#else
#define ERROR_DEBUG(...)
#endif

#define PROTOCAL_LED_ON			(uint8_t)0
#define PROTOCAL_LED_OFF		(uint8_t)1
#define PROTOCAL_SELF_TESET		(uint8_t)2

typedef void (*pvoidFunc)(void);

#define CAN_APP_CONTROLLER canControllerIdx1

typedef struct {
	CanControllerIdx_t controller;
	CanBaud_t baud;
} CanAppHalder_t;

typedef enum {
	CanAppEvtGetMsg = 0, CanAppTransComplete, CanAppBusOff, CanAppEvtNum,
} CanAppEvt_t;

uint8_t plate_color = 0;
uint8_t plate_status = 0;
TimerHandle_t g_led_off_timer;

static void CanAppReceiveMsgHandler(void);
static void CanAppTxCompleteHalder(void);
static void CanAppBuffOffHanlder(void);

static void canAppCb(CanControllerIdx_t controller, uint8_t it_flag);

static QueueHandle_t xQueue;
static SemaphoreHandle_t xSemphore;
static uint8_t selfId;
const static pvoidFunc CanAppEvtHandler[CanAppEvtNum] = {
		CanAppReceiveMsgHandler, CanAppTxCompleteHalder, CanAppBuffOffHanlder, };
const CanAppHalder_t CanAppHandler[] =
		{ { canControllerIdx1, CAN_Baud_500K, }, };
const canFirlter_t canFirlter[] = { { CAN_ID_STANDRD, 0x40 }, };
const canFIrlterList_t firlterList = { canFirlter, sizeof(canFirlter)
		/ sizeof(canFirlter[0]), };

static void CanAppReceiveMsgHandler(void) {
	can_frame_t frame;
	CanGet_MSG(CAN_APP_CONTROLLER, &frame);
	uint8_t i;
	if (frame.id == 0x40) {
		switch (frame.dataByte0) {
		case PROTOCAL_LED_ON :
			if (frame.dataByte1 == selfId) {
				if (frame.dataByte2 == 0) {
					for (i = 0; i <= NUM_GRB_LEDS; i++) {
						leds[i].b = 0;
						leds[i].g = 0xFF;
						leds[i].r = 0;
						plate_color = 1;
					}

				} else {
					for (i = 0; i <= NUM_GRB_LEDS; i++) {
						leds[i].b = 0;
						leds[i].g = 0;
						leds[i].r = 0xFF;
						plate_color = 0;
					}
				}
				while (!ws2812b_IsReady())
					;
				ws2812b_SendRGB(leds, NUM_GRB_LEDS);
				plate_status = 1;
				if(frame.dataByte3 != 0xFF) {
					xTimerChangePeriod(g_led_off_timer, frame.dataByte3 * 100, 100);
					xTimerStart(g_led_off_timer, 100);
				}
			}
			break;
		case PROTOCAL_LED_OFF :
			if (frame.dataByte1 == selfId) {
				if (frame.dataByte2 == 1) {
					for (i = 0; i <= NUM_GRB_LEDS; i++) {
						leds[i].b = 0;
						leds[i].g = 0;
						leds[i].r = 0;
					}

				}
				while (!ws2812b_IsReady())
					;
				ws2812b_SendRGB(leds, NUM_GRB_LEDS);
			}
			break;
		case PROTOCAL_SELF_TESET :
			if (frame.dataByte1 == selfId) {

				for (i = 0; i <= NUM_GRB_LEDS; i++) {
					leds[i].b = 0;
					leds[i].g = 0;
					leds[i].r = 0xFF;
				}
				while (!ws2812b_IsReady())
					;
				ws2812b_SendRGB(leds, NUM_GRB_LEDS);
				vTaskDelay(200);
				for (i = 0; i <= NUM_GRB_LEDS; i++) {
					leds[i].b = 0;
					leds[i].g = 0xFF;
					leds[i].r = 0;
				}
				while (!ws2812b_IsReady())
					;
				ws2812b_SendRGB(leds, NUM_GRB_LEDS);
				vTaskDelay(200);
				for (i = 0; i <= NUM_GRB_LEDS; i++) {
					leds[i].b = 0xFF;
					leds[i].g = 0;
					leds[i].r = 0;
				}
				while (!ws2812b_IsReady())
					;
				ws2812b_SendRGB(leds, NUM_GRB_LEDS);
				vTaskDelay(200);
				CanAppSendMsg(&frame);
				for (i = 0; i <= NUM_GRB_LEDS; i++) {
					leds[i].b = 0;
				}
				while (!ws2812b_IsReady())
					;
				ws2812b_SendRGB(leds, NUM_GRB_LEDS);
			}
			break;
		default:
			break;
		}
	}
}

static void CanAppTxCompleteHalder(void) {
	xSemaphoreGiveFromISR(xSemphore, NULL);
}

static void CanAppBuffOffHanlder(void) {
	CanDeinit(CAN_APP_CONTROLLER);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb,
			&firlterList);
}

void CanAppSendMsg(can_frame_t *pFrame) {
	uint8_t tryCnt = 3;
	pFrame->id = selfId;
	pFrame->length = 8;
	pFrame->type = 0;
	pFrame->format = 0;
	while (tryCnt--) {
		if (CanSend_MSG(CAN_APP_CONTROLLER, pFrame)) {
			if (xSemaphoreTake(xSemphore, pdMS_TO_TICKS(100)) == pdPASS) {
				return;
			}
		} else {
			vTaskDelay(pdMS_TO_TICKS(10));
		}
	}ERROR_DEBUG("[CanApp] Send msg failed\r\n");
}

static void canAppCb(CanControllerIdx_t controller, uint8_t it_flag) {
	CanAppEvt_t evt;
	switch (it_flag) {
	case CAN_RX_DATA :
		evt = CanAppEvtGetMsg;
		break;
	case CAN_TX_COMPLETE :
		evt = CanAppTransComplete;
		break;
	case CAN_WAKEUP :
		break;
	case CAN_BUSSOFF_ERR :
		evt = CanAppBusOff;
		break;
	case CAN_PASSIVE_ERR :
		break;
	default:
		break;
	}
	xQueueSendFromISR(xQueue, &evt, NULL);
}

static void xTask(void *pParamter) {
	uint8_t event;
	for (;;) {
		if (pdPASS == xQueueReceive(xQueue, &event, pdMS_TO_TICKS(500))) {
			if (event >= CanAppEvtNum) {
				continue;
			}
			CanAppEvtHandler[event]();
		} else {
//            can_frame_t heartbeat =
//            {
//                .length = 8,
//                .type = CAN_TYPE_DATA,
//                .format = CAN_TYPE_DATA,
//                .dataWord0 = 0,
//                .dataWord1 = 0,
//            };
//            CanAppSendMsg((can_frame_t *)&heartbeat);
		}
	}
}

void xTimerHandler(void *p) {
	can_frame_t frame;
	uint8_t i;
	if (plate_status == 1) {
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
		frame.dataByte1 = 0xff;
		frame.dataByte2 = 1;
		CanAppSendMsg(&frame);

	}
}

void CanAppInit(void) {
	selfId = PDin(8) | (PDin(9) << 1) | (PDin(10) << 2) | (PDin(11) << 3)
			| (PDin(12) << 4) | (PDin(13) << 5);
	xQueue = xQueueCreate(3, 1);
	xSemphore = xSemaphoreCreateBinary();
	xTaskCreate(xTask, "CanApp", 128, NULL, 3, NULL);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb,
			&firlterList);
	g_led_off_timer = xTimerCreate("ledoff", 0, pdFALSE, (void*) NULL,
			xTimerHandler);
}
