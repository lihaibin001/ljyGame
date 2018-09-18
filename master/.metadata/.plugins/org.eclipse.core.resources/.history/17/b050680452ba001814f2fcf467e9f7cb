#include "CanApp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sys.h"

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

typedef struct
{
	CanControllerIdx_t controller;
	CanBaud_t baud;
} CanAppHalder_t;

typedef enum
{
	CanAppEvtGetMsg = 0,
	CanAppTransComplete,
	CanAppBusOff,
	CanAppEvtNum,
} CanAppEvt_t;


static void CanAppReceiveMsgHandler(void);
static void CanAppTxCompleteHalder(void);
static void CanAppBuffOffHanlder(void);
static void CanAppSendMsg(can_frame_t *pFrame);
static void canAppCb(CanControllerIdx_t controller, uint8_t it_flag);


static QueueHandle_t xQueue;
static SemaphoreHandle_t xSemphore;
static uint32_t selfId;
const static pvoidFunc CanAppEvtHandler[CanAppEvtNum] =
{
	CanAppReceiveMsgHandler,
	CanAppTxCompleteHalder,
	CanAppBuffOffHanlder,
};
const CanAppHalder_t CanAppHandler[] =
{
	{
		canControllerIdx1,
		CAN_Baud_500K,
	},
};
const canFirlter_t canFirlter[] =
{
	{CAN_ID_STANDRD, 1},
	{CAN_ID_STANDRD, 2},
	{CAN_ID_STANDRD, 3},
	{CAN_ID_STANDRD, 4},
	{CAN_ID_STANDRD, 5},
	{CAN_ID_STANDRD, 6},
	{CAN_ID_STANDRD, 7},
	{CAN_ID_STANDRD, 8},
};
const canFIrlterList_t firlterList =
{
	canFirlter,
	sizeof(canFirlter) / sizeof(canFirlter[0]),
};


static void CanAppReceiveMsgHandler(void)
{
    can_frame_t frame;
    CanGet_MSG(CAN_APP_CONTROLLER, &frame);
    CanAppSendMsg(&frame);
}

static void CanAppTxCompleteHalder(void)
{
	xSemaphoreGiveFromISR(xSemphore, NULL);
}

static void CanAppBuffOffHanlder(void)
{
	CanDeinit(CAN_APP_CONTROLLER);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb, &firlterList);
}

void CanAppSendMsg(can_frame_t *pFrame)
{
    uint8_t tryCnt = 3;;
    pFrame->id = selfId;
    while(tryCnt--)
    {
        if(CanSend_MSG(CAN_APP_CONTROLLER, pFrame))
        {
            if(xSemaphoreTake(xSemphore, pdMS_TO_TICKS(100)) == pdPASS)
            {
                return;
            }
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    ERROR_DEBUG("[CanApp] Send msg failed\r\n");
}

static void canAppCb(CanControllerIdx_t controller, uint8_t it_flag)
{
	CanAppEvt_t evt;
	switch(it_flag)
	{
		case CAN_RX_DATA:
			evt = CanAppEvtGetMsg;
			break;
		case CAN_TX_COMPLETE:
			evt = CanAppTransComplete;
			break;
		case CAN_WAKEUP:
			break;
		case CAN_BUSSOFF_ERR:
			evt = CanAppBusOff;
			break;
		case CAN_PASSIVE_ERR:
			break;
		default:
			break;
	}
	xQueueSendFromISR(xQueue, &evt, NULL);
}

static void xTask(void *pParamter)
{
	uint8_t event;
	for(;;)
	{
		if(pdPASS == xQueueReceive(xQueue, &event, pdMS_TO_TICKS(500)))
		{
			if(event >= CanAppEvtNum)
			{
				continue;
			}
			CanAppEvtHandler[event]();
		}
		else
		{
            can_frame_t heartbeat =
            {
                .length = 8,
                .type = CAN_TYPE_DATA,
                .format = CAN_TYPE_DATA,
                .dataWord0 = 0,
                .dataWord1 = 0,
            };
            CanAppSendMsg((can_frame_t *)&heartbeat);
		}
	}
}

void CanAppInit(void)
{
	selfId = 1;
	xQueue = xQueueCreate(3, 1);
	xSemphore = xSemaphoreCreateBinary();
	xTaskCreate(xTask, "CanApp", 128, NULL, 4, NULL);
	CanInit(CanAppHandler[0].controller, CanAppHandler[0].baud, canAppCb, &firlterList);
}
