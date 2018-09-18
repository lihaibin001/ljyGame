#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include <stdint.h>
#include <delay.h>

#define delay_timer TIM3
#define RCC_APBPeriph_TIMER RCC_APB1Periph_TIM3
// Init timer for Microseconds delays
void _init_us() {
	// Enable clock for TIM
	RCC_APB1PeriphClockCmd(RCC_APBPeriph_TIMER,ENABLE);

	// Time base configuration
	TIM_TimeBaseInitTypeDef TIM;
	TIM_TimeBaseStructInit(&TIM);
	TIM.TIM_Prescaler = (SystemCoreClock/1000000)-1;
	TIM.TIM_Period = UINT16_MAX;
	TIM.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(delay_timer,&TIM);

	// Enable counter for TIM
	TIM_Cmd(delay_timer,ENABLE);
}

// Init and start timer for Milliseconds delays
void _init_ms() {
	// Enable clock for TIM
//	RCC_APB1PeriphClockCmd(RCC_APBPeriph_TIMER,ENABLE);

	// Time base configuration
	TIM_TimeBaseInitTypeDef TIM;
	TIM_TimeBaseStructInit(&TIM);
	TIM.TIM_Prescaler = (SystemCoreClock/4000)-1;
	TIM.TIM_Period = UINT16_MAX;
	TIM.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(delay_timer,&TIM);

	// Enable counter for TIM
	TIM_Cmd(delay_timer,ENABLE);
}

// Stop timer
void _stop_timer() {
	TIM_Cmd(delay_timer,DISABLE);
//	RCC_APB1PeriphClockCmd(RCC_APBPeriph_TIMER,DISABLE); // Powersavings?
}

// Do delay for nTime milliseconds
void Delay_ms(uint32_t mSecs) {
    #if 0
	// Init and start timer
	_init_ms();

	// Dummy loop with 16 bit count wrap around
	volatile uint32_t start = delay_timer->CNT;
	while((delay_timer->CNT-start) <= mSecs);

	// Stop timer
	_stop_timer();
    #endif
    while(mSecs--)
    {
        Delay_us(998);
    }
}

// Do delay for nTime microseconds
void Delay_us(uint32_t uSecs) {
	// Init and start timer
	_init_us();

	// Dummy loop with 16 bit count wrap around
	volatile uint32_t start = delay_timer->CNT;
	while((delay_timer->CNT-start) <= uSecs);

	// Stop timer
	_stop_timer();
}



































