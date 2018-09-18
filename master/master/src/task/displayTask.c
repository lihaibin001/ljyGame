#include "displayTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "RGBMatrix.h"
#include "maxtrixApp.h"
static QueueSetHandle_t xQueue;
uint8_t currentGameMode = 1;
uint8_t currentGameLevel = 1;
static void xTask(void *pPara)
{
    uint8_t trush;
    for(;;)
    {
		if(pdPASS == xQueueReceive(xQueue, &trush, pdMS_TO_TICKS(500)))
		{
            RGBProcessor();
		}
    }
}

bool CreateDisplayTask(void)
{
	xQueue = xQueueCreate(5, 1);
    if(xQueue == NULL)
    {
        return false;
    }
	if(pdPASS !=  xTaskCreate(xTask, "Dispaly", 128, NULL, 3, NULL))
    {
        return false;
    }
    return true;
}

void DisplayRefalsh(void)
{
    uint8_t trash;
	xQueueSendFromISR(xQueue, &trash, NULL);    
}
