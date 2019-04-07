/*
 * mp3.c
 *
 *  Created on: Apr 5, 2019
 *      Author: murphy
 */

#include "mp3.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
static void mp3_doSum(uint8_t *pStr, uint8_t len) ;

void mp3_init(void) {
	Uart_Initialize(UART_MP3_CHANNEL);
	vTaskDelay(500);
	mp3_send_command(0x25, 0x0101);
	vTaskDelay(100);
 	mp3_send_command(0x16, 0x0000);
	vTaskDelay(100);
	mp3_send_command(0x06, 0x000F);
	vTaskDelay(100);
	mp3_send_command(0x08, 0x0001);
	vTaskDelay(100);
	mp3_send_command(0x19, 0);
}

void mp3_send_command(uint8_t command, uint16_t data) {
	uint8_t cmd[10] = {0x7E, 0xff, 0x06, command, 0x01, (uint8_t)(data >> 8), (uint8_t) data, 0, 0, 0xEF};
	mp3_doSum(&cmd[1], 6);
	UART_Transmit(UART_MP3_CHANNEL, cmd, 10);
}

static void mp3_doSum(uint8_t *pStr, uint8_t len) {
	uint16_t xorsum = 0;
	uint8_t i;
	for(i = 0; i < len; i++) {
		xorsum += pStr[i];
	}
	xorsum = 0 - xorsum;
	pStr[i] = (uint8_t)(xorsum >> 8);
	pStr[i+1] = (uint8_t)(xorsum & 0xFF);
}
