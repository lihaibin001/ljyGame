#include "htimer.h"
/**
  * @brief TIM3初始化
  * @param  无
  * @retval 无
  */
void TIM3_Init(uint16_t time)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	
	TIM_DeInit(TIM3);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);		
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);


	
	TIM_TimeBaseStructure.TIM_Period = time * 10 -1;
	TIM_TimeBaseStructure.TIM_Prescaler = 7199; 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 


	
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	
	
	TIM_Cmd(TIM3, ENABLE);
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	TIM_Cmd(TIM3, DISABLE);
}

/**
  * @brief 使能TIM3
  * @param  无
  * @retval 无
  */
void TIM3_Enable(void)
{
	TIM3->CR1 |= 0x01;
}

void TIM3_IRQHandler(void)   //TIM3中断
{
	if((TIM3->SR & 0x01) == 1)
	{
			TIM3->SR &= ~0x01;
			TIM3->CR1 &= ~0x01;
			CarCtr_GPIO_Reset();
//		TIM3->SR &= ~0x01;
//		TIM3->CR1 &= ~0x01;
//		UART4_Rx_Complete();
	}
}


