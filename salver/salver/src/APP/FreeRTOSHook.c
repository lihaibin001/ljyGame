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
#include "ws2812b.h"
#include "at24qtxx.h"
#include "led.h"
void vApplicationDaemonTaskStartupHook( void )
{
	sysclock_init();
	gpioInit();
    CanAppInit();
    ws2812b_Init();
    at24qt_intit();
    LED_Init();
}

void vApplicationIdleHook( void )
{

}

void vApplicationTickHook( void )
{


}
