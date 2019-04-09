#include "CanApp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sys.h"
#include "maxtrixApp.h"
#include "sync.h"

#include <string.h>
#define MSG_BUFF_AMOUT 8

#ifdef CAN_APP_DEBUG
#define DEBUG(...) printf(...)
#else
#define DEBUG(...)
#endif

#ifdef CAN_ERROR_DEBUG
#define ERROR_DEBUG(...) printf(...)
#else
#define ERROR_DEBUG(...)
#endif

typedef void (*pvoidFunc)(void);

#define CAN_APP_CONTROLLER canControllerIdx1
#define MSG_STATUS_EMPTY 0
#define MSG_STATUS_SETUP 1
#define MSG_STATUS_TRANSFERING 2
#define MSG_STATUS_TRANSFER_1 3
#define MSG_SATTUS_TRANSFER_2 4

typedef struct {
	can_frame_t frame;
	uint32_t timestamp;
	uint8_t status;
} CanMsgBuf_t;

typedef struct {
	CanControllerIdx_t controller;
	CanBaud_t baud;
} CanAppHalder_t;

typedef enum {
	CanAppEvtGetMsg = 0, CanAppTransComplete, CanAppBusOff, CanAppEvtNum,
} CanAppEvt_t;

static uint8_t msgSeq;
static CanMsgBuf_t msgBuf[MSG_BUFF_AMOUT];
static QueueHandle_t xQueue;
static SemaphoreHandle_t xSemphore;
static uint32_t selfId;

const CanAppHalder_t CanAppHandler[] =
		{ { canControllerIdx1, CAN_Baud_500K, }, };
const canFirlter_t canFirlter[] = { { CAN_ID_STANDRD, 1 },
		{ CAN_ID_STANDRD, 2 }, { CAN_ID_STANDRD, 3 }, { CAN_ID_STANDRD, 4 }, {
		CAN_ID_STANDRD, 5 }, { CAN_ID_STANDRD, 6 }, { CAN_ID_STANDRD, 7 }, {
				CAN_ID_STANDRD, 8 }, { CAN_ID_STANDRD, 0x21 }, { CAN_ID_STANDRD,
				0x22 }, { CAN_ID_STANDRD, 0x23 }, { CAN_ID_STANDRD, 0x24 }, {
				CAN_ID_STANDRD, 0x25 }, { CAN_ID_STANDRD, 0x26 }, {
		CAN_ID_STANDRD, 0x27 }, { CAN_ID_STANDRD, 0x28 },

};
const canFIrlterList_t firlterList = { canFirlter, sizeof(canFirlter)
		/ sizeof(canFirlter[0]), };

static void CanAppReceiveMsgHandler(void);
static void CanAppTxCompleteHalder(void);
static void CanAppBuffOffHanlder(void);
static void canAppCb(CanControllerIdx_t controller, uint8_t it_flag);

const static pvoidFunc CanAppEvtHandler[CanAppEvtNum] = {
		CanAppReceiveMsgHandler, CanAppTxCompleteHalder, CanAppBuffOffHanlder, };

static void CanAppReceiveMsgHandler(void) {
	can_frame_t frame;
	RET_t status = CanGet_MSG(CAN_APP_CONTROLLER, &frame);
	uint16_t data;
	if (status == RET_OK) {
		switch (frame.dataByte0) {
		case PROTOCAL_LED_ON :
			data = (uint16_t) (frame.id << 8 | frame.dataByte1);
			ps_send_event(PS_EVT_SLAVE_EVT, data);
			break;
		case PROTOCAL_GAME_OVER :
			break;
		case PROTOCAL_SELF_TESET :
			ps_send_event(PS_EVT_TEST, (int16_t) frame.id);
			break;
		}
	} else {
		DEBUG("[CanApp] receive error: %d\r\n", status);
	}
	//CanAppSendMsg(&frame);
}

static void CanAppTxCompleteHalder(void) {
	xSemaphoreGive(xSemphore);
}

static void CanAppBuffOffHanlder(void) {
	CanDeinit(CAN_APP_CONTROLLER);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb,
			&firlterList);
}

RET_t CanAppSendMsg(can_frame_t *pFrame) {
	uint8_t i;
	if (pFrame == NULL) {
		return RET_PARAM_ERR;
	}

	for (i = 0; i < MSG_BUFF_AMOUT; i++) {
		if (msgBuf[i].status == MSG_STATUS_EMPTY) {
			memcpy(&msgBuf[i].frame, pFrame, sizeof(can_frame_t));
			msgBuf[i].frame.dataByte7 = msgSeq++;
			msgBuf[i].timestamp = xTaskGetTickCount();
			if (RET_OK == CanSend_MSG(CAN_APP_CONTROLLER, pFrame)) {
				msgBuf[i].status = MSG_STATUS_TRANSFERING;
			} else {
				msgBuf[i].status = MSG_STATUS_SETUP;
			}
			return RET_OK;
		}
	}
	return RET_ERR;
//	uint8_t tryCnt = 3;
//	pFrame->format = CAN_ID_STANDRD;
//	pFrame->length = 8;
//	pFrame->type = CAN_TYPE_DATA;
//	pFrame->id = selfId;

//	while (tryCnt--) {
//		status = CanSend_MSG(CAN_APP_CONTROLLER, pFrame);
//		if (status == RET_OK) {
//			if (xSemaphoreTake(xSemphore, pdMS_TO_TICKS(100)) == pdPASS) {
//				return RET_OK;
//			}
//		} else {
//			ERROR_DEBUG("[CanApp] Send message error: %d\r\n", status);
//			vTaskDelay(pdMS_TO_TICKS(10));
//		}
//	}
//	return status;
}

static void canAppCb(CanControllerIdx_t controller, uint8_t it_flag) {
	CanAppEvt_t evt;
	switch (it_flag) {
	case CAN_RX_DATA :
		evt = CanAppEvtGetMsg;
		break;
	case CAN_TX_COMPLETE : {
		uint i;
		for (i = 0; i < MSG_STATUS_SETUP; i++) {
			if (msgBuf[i].status == MSG_STATUS_SETUP) {
				if (RET_OK == CanSend_MSG(CAN_APP_CONTROLLER, &msgBuf[i].frame)) {
//					msgBuf[i].status = MSG_STATUS_TRANSFERING;
					msgBuf[i].status = MSG_STATUS_EMPTY;
				}
				break;
			}
		}
		return;
	}
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
		if (pdPASS == xQueueReceive(xQueue, &event, pdMS_TO_TICKS(0xFFFFFFFF))) {
			if (event >= CanAppEvtNum) {
				continue;
			}
			if (CanAppEvtHandler[event]) {
				CanAppEvtHandler[event]();
			}
		} else {
		}
	}
}

void CanAppInit(void) {
	selfId = 0x40;
	xQueue = xQueueCreate(3, 1);
	xSemphore = xSemaphoreCreateBinary();
	xTaskCreate(xTask, "CanApp", 128, NULL, 1, NULL);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb,
			&firlterList);
}
