#include "displayTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "RGBMatrix.h"
#include "maxtrixApp.h"
static QueueSetHandle_t xQueue;
static TaskHandle_t displayTaskHanle;
uint8_t currentGameMode = 1;
uint8_t currentGameLevel = 1;
static void xTask(void *pPara)
{
    uint8_t data = 0;
    for(;;)
    {
		xQueueReceive(xQueue, &data, pdMS_TO_TICKS(10));
		if(data == 0x5A) {
			vTaskSuspend(displayTaskHanle);
		}
        RGBProcessor();
    }
}

bool CreateDisplayTask(void)
{
	xQueue = xQueueCreate(2, 1);
    if(xQueue == NULL)
    {
        return false;
    }
	if(pdPASS !=  xTaskCreate(xTask, "Dispaly", 128, NULL, 4, &displayTaskHanle))
    {
        return false;
    }
    return true;
}

void DisplayRefalsh(void)
{
    uint8_t trash = 0;
	xQueueSendFromISR(xQueue, &trash, NULL);
}

uint8_t DisplaySuspend(uint32_t waittime) {
	uint8_t data = 0x5A;
	TaskStatus_t status;
	vTaskGetInfo(displayTaskHanle, &status, pdTRUE, eInvalid);
	if(status.eCurrentState == eSuspended) {
		return 1;
	}
	while(waittime -= 10) {
		xQueueSend(xQueue, &data, 0);
		vTaskGetInfo(displayTaskHanle, &status, pdTRUE, eInvalid);
		if(status.eCurrentState == eSuspended) {
			return 1;
		}
	}
	return 0;
}

void DisplayResume(void) {
	vTaskResume(displayTaskHanle);
}

