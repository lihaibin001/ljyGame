#include "FreeRTOS.h"
#include "task.h"
#include "peridic.h"
#include "eeprom.h"
#include "iic.h"
#include "sysclock.h"
#include "gpioInit.h"
#include "CanApp.h"
#include "sys.h"
#include "delay.h"
#include "RGBMatrix.h"
#include "displayTask.h"
#include "keyDetect.h"
#include "maxtrixApp.h"
#include "sync.h"

void vApplicationDaemonTaskStartupHook( void )
{
	//RGBClearBuff();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	sysclock_init();
	gpioInit();
    CanAppInit();
    keyDetectInit();
    RGBSetupRGBMatrixPorts();
    maxtriAppInit();
    CreateDisplayTask();
    ps_task_create();
}

void vApplicationIdleHook( void )
{
//	 RGBProcessor();
}

void vApplicationTickHook( void )
{
	rand();
    //DisplayRefalsh();
}
