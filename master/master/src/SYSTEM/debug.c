/*
 * debug.c
 *
 *  Created on: Apr 15, 2019
 *      Author: murphy
 */

#include "debug.h"
#include "uart.h"

//int __io_putchar(int ch)
//{
//	//Uart_Put_Char(0, (uint8_t)ch);
//	return ch;
//}

int _write(int file, char *ptr, int len)
{
	return (int)UART_Transmit(0, (const uint8_t *)ptr, (uint8_t)len);
}


void debug_init(void) {
	Uart_Initialize(0);
}
