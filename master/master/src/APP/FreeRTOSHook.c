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
	RGBClearBuff();
	sysclock_init();
	gpioInit();
    CreateDisplayTask();
    ps_task_create();
    CanAppInit();
    keyDetectInit();
    RGBSetupRGBMatrixPorts();
    maxtriAppInit();
}

void vApplicationIdleHook( void )
{

}

void vApplicationTickHook( void )
{
    DisplayRefalsh();
}
