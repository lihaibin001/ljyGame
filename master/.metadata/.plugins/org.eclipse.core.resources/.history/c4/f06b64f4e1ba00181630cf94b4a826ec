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
void vApplicationDaemonTaskStartupHook( void )
{
    CreateDisplayTask();
    CanAppInit();
    keyDetectInit();
    RGBSetupRGBMatrixPorts();
    maxtrixDisplayBootImage();
    
}

void vApplicationIdleHook( void )
{

}

void vApplicationTickHook( void )
{
    DisplayRefalsh();

}
