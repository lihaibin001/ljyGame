#ifndef __MATIRX_CONFIG_H
#define __MATIRX_CONFIG_H

#define LED_PORT 	GPIOC
#define LED_P		GPIO_Pin_13
#define LED_RCCPB	RCC_APB2Periph_GPIOC

#define MTX_PORT0           GPIOD
#define MTX_PORT1           GPIOB
#define MTX_PORT_SELSET     GPIOE
#define MTX_PORT_CONTROL    GPIOE

#define MTX_RCCP0   RCC_APB2Periph_GPIOD
#define MTX_RCCP1   RCC_APB2Periph_GPIOB
#define MTX_RCCPS   RCC_APB2Periph_GPIOE

#define MTX_PR0		GPIO_Pin_3
#define MTX_PG0		GPIO_Pin_4
#define MTX_PB0		GPIO_Pin_7

#define MTX_PR1		GPIO_Pin_5
#define MTX_PG1		GPIO_Pin_6
#define MTX_PB1		GPIO_Pin_7

#define MTX_PA		GPIO_Pin_0
#define MTX_PB		GPIO_Pin_1
#define MTX_PC		GPIO_Pin_2
#define MTX_PD		GPIO_Pin_3

#define MTX_PSTB	GPIO_Pin_5
#define MTX_POE		GPIO_Pin_6
#define MTX_PCLK	GPIO_Pin_4

#define DISP_ON         MTX_PORT_CONTROL->BRR = MTX_POE;
#define DISP_OFF        MTX_PORT_CONTROL->BSRR = MTX_POE;
#define CLK_TOGGLE      MTX_PORT_CONTROL->BSRR = MTX_PCLK; MTX_PORT_CONTROL->BRR = MTX_PCLK;
#define STROBE          MTX_PORT_CONTROL->BSRR = MTX_PSTB; MTX_PORT_CONTROL->BRR = MTX_PSTB;

#define MATRIX_WIDTH    64
#define MATRIX_HEIGHT   32
#define MATRIX_MODULE   32
#define MATRIX_SIZE     MATRIX_WIDTH*MATRIX_HEIGHT

#endif
