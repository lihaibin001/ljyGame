#include "FreeRTOS.h"
#include "task.h"
//#include "peridic.h"
#include "eeprom.h"
#include "iic.h"
#include "gpioInit.h"
#include "CanApp.h"
#include "sys.h"
#include "delay.h"
#include "RGBMatrix.h"
#include "displayTask.h"
#include "keyDetect.h"
#include "maxtrixApp.h"
#include "sync.h"

#define RCC_AHBPeriphClock (RCC_AHBPeriph_DMA1)

#define RCC_APB2PeriphClock ( RCC_APB2Periph_AFIO \
                              | RCC_APB2Periph_GPIOA \
                              | RCC_APB2Periph_GPIOB \
                              | RCC_APB2Periph_GPIOC \
                              | RCC_APB2Periph_GPIOD \
                              | RCC_APB2Periph_GPIOE \
                              | RCC_APB2Periph_GPIOF \
                              | RCC_APB2Periph_GPIOG \
							  | RCC_APB2Periph_USART1)
#define RCC_APB1PeriphClock ( RCC_APB1Periph_TIM2 \
                              | RCC_APB1Periph_TIM4 \
                              | RCC_APB1Periph_CAN1 \
							  | RCC_APB1Periph_USART2 \
							  | RCC_APB1Periph_USART3 \
							  | RCC_APB1Periph_UART4)

void mn_nvic_config(void)
{
    NVIC_InitTypeDef  NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x6;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x6;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the USART3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x6;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the UART4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x6;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}



void sysclock_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriphClock, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2PeriphClock, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1PeriphClock, ENABLE);
}

void sysclok_deinit(void)
{
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriphClock, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2PeriphClock, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1PeriphClock, DISABLE);
}


void vApplicationDaemonTaskStartupHook( void )
{
	//RGBClearBuff();

	sysclock_init();
	mn_nvic_config();
	gpioInit();
	debug_init();
	mp3_init();
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
