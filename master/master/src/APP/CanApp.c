#include "CanApp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sys.h"
#include "maxtrixApp.h"
#include "sync.h"

#include <string.h>
#define MSG_BUFF_AMOUT 10

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

typedef struct {
	CanControllerIdx_t controller;
	CanBaud_t baud;
} CanAppHalder_t;

typedef enum {
	CanAppEvtGetMsg = 0, CanAppTransComplete, CanAppBusOff, CanAppEvtNum,
} CanAppEvt_t;

typedef struct {
	can_frame_t *pFrame;
	volatile uint8_t in;
	volatile uint8_t out;
	volatile bool isTransfering;
	uint8_t item_cnt;
}MsgBuf_t;

static can_frame_t msgBuf[MSG_BUFF_AMOUT];
static MsgBuf_t msgList = {
		msgBuf,
		0,
		0,
		false,
		0,
};
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
	if(msgList.item_cnt != 0){
		msgList.item_cnt--;
	}
	if(msgList.item_cnt == 0) {
		msgList.isTransfering = false;
	} else {
		CanSend_MSG(CAN_APP_CONTROLLER, &msgList.pFrame[msgList.out++]);
		if(msgList.out == MSG_BUFF_AMOUT) {
			msgList.out = 0;
		}

	}
//	xSemaphoreGive(xSemphore);
}

static void CanAppBuffOffHanlder(void) {
	CanDeinit(CAN_APP_CONTROLLER);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb,
			&firlterList);
}

RET_t CanAppSendMsg(can_frame_t *pFrame) {
	pFrame->format = CAN_ID_STANDRD;
	pFrame->length = 8;
	pFrame->type = CAN_TYPE_DATA;
	pFrame->id = selfId;
	if(msgList.item_cnt == MSG_BUFF_AMOUT) {
		return RET_ERR;
	}
	memcpy(&msgList.pFrame[msgList.in++], pFrame, sizeof(can_frame_t));
	if(msgList.in == MSG_BUFF_AMOUT) {
		msgList.in = 0;
	}
	msgList.item_cnt++;
	return RET_OK;
}

static void canAppCb(CanControllerIdx_t controller, uint8_t it_flag) {
	CanAppEvt_t evt;
	switch (it_flag) {
	case CAN_RX_DATA :
		evt = CanAppEvtGetMsg;
		break;
	case CAN_TX_COMPLETE :
		return;
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
		if (pdPASS == xQueueReceive(xQueue, &event, pdMS_TO_TICKS(5))) {
			if (event >= CanAppEvtNum) {
				continue;
			}
			if (CanAppEvtHandler[event]) {
				CanAppEvtHandler[event]();
			}
		} else {
			if(msgList.item_cnt != 0){
				msgList.item_cnt--;
				CanSend_MSG(CAN_APP_CONTROLLER, &msgList.pFrame[msgList.out++]);
				if(msgList.out == MSG_BUFF_AMOUT) {
					msgList.out = 0;
				}
			}
		}
	}
}

void CanAppInit(void) {
	selfId = 0x40;
	xQueue = xQueueCreate(10, 1);
	xSemphore = xSemaphoreCreateBinary();
	xTaskCreate(xTask, "CanApp", 128, NULL, 1, NULL);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb,
			&firlterList);
}
