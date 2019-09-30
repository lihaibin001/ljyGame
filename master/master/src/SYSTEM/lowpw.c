#include "lowpw.h"
#include "profile.h"
#include "keyDetect.h"
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "delay.h"
void EXTI_Configuration(void) {

	EXTI_InitTypeDef EXTI_InitStructure = { 0 };
	GPIO_InitTypeDef GPIO_InitStructure = { 0 };
	NVIC_InitTypeDef NVIC_InitStructure = { 0 };
	/* Enable GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);

	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}


extern void sysclok_deinit(void);
static void lowpower_config(void) {
	__disable_irq();
	sysclok_deinit();
	__enable_irq();
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	EXTI_Configuration();
}

static void SYSCLKConfig_STOP(void)
{
	ErrorStatus HSEStartUpStatus;

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {

    RCC_PLL2Cmd(ENABLE);

    /* Wait till PLL2 is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
    {
    }

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
}
void lwpw_entery_stop(void) {

	do {
		lowpower_config();
		PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
		SYSCLKConfig_STOP();
		Delay_ms(3000);
		if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
			break;
		}
	} while (1);
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

void EXTI15_10_IRQHandler(void) {
	if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
		/* Clear the  EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}
