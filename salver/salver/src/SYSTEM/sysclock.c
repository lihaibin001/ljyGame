#include "sysclock.h"

#define RCC_AHBPeriphClock (RCC_AHBPeriph_DMA1)

#define RCC_APB2PeriphClock ( RCC_APB2Periph_AFIO \
                              | RCC_APB2Periph_GPIOA \
                              | RCC_APB2Periph_GPIOB \
                              | RCC_APB2Periph_GPIOC \
                              | RCC_APB2Periph_GPIOD \
                              | RCC_APB2Periph_GPIOE \
                              | RCC_APB2Periph_GPIOF \
                              | RCC_APB2Periph_GPIOG )
#define RCC_APB1PeriphClock ( RCC_APB1Periph_TIM2 \
                              | RCC_APB1Periph_TIM4 \
                              | RCC_APB1Periph_CAN1 )

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