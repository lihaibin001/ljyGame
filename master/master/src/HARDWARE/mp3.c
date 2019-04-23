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
#include "timers.h"
#include "queue.h"
#include "debug.h"

#include <string.h>

typedef struct {
	uint8_t cmd;
	uint16_t data;
} mp3_tx_t;

typedef struct {
	uint8_t cmd;
	uint16_t data;
} Msg_t;

#define RX_BUF_SZIE 128
#define TX_BUF_SZIE 12

static bool is_ready;
static uint8_t status;
//static TimerHandle_t xTimers;
static uint8_t rx_buff[RX_BUF_SZIE];
static uint8_t rx_idx;
static uint8_t rx_idx;
static uint8_t tx_buff[TX_BUF_SZIE][10];
static uint8_t tx_buff_remained = 0;
static uint8_t tx_buff_in;
static uint8_t tx_buff_out;
static uint32_t command_timeout;
static uint8_t command_try_cnt;
static QueueSetHandle_t xQueue;

static void mp3_doSum(uint8_t *pStr, uint8_t len);
static uint16_t mp3_calcSum(uint8_t *pStr, uint8_t len);
static void mp3_process_rx_data(uint8_t *pData);

static void mp3_process_rx_data(uint8_t *pData) {
	DEBUG_INFO("MP3 Rx:%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
			(int )pData[0], (int )pData[1], (int )pData[2], (int )pData[3],
			(int )pData[4], (int )pData[5], (int )pData[6], (int )pData[7],
			(int )pData[8], (int )pData[9]);
	switch (pData[3]) {
	case 0x90:
	case 0x3F: //TODO protocol? we want 0x02 or 0x03, but we receive 0x0a
		if (pData[6] == 2 || pData[6] == 3) {
			is_ready = true;
		}
		break;
	case 0x41:
		if (tx_buff_remained == 0) {
			return;
		}
		tx_buff_out++;
		tx_buff_remained--;
		command_try_cnt = 0;
		command_timeout = 0;
		if (tx_buff_out == TX_BUF_SZIE) {
			tx_buff_out = 0;
		}
		break;
	}
}
static void mp3_rx_check(void) {
	uint8_t i;
	while (Uart_Get_Char(UART_MP3_CHANNEL, &rx_buff[rx_idx])) {
		if (++rx_idx == RX_BUF_SZIE) {
			break;
		}
	}
	if (rx_idx >= 9) {
		uint8_t remain = rx_idx;
		for (i = 0; i <= rx_idx - 9; i++) {
			if (rx_buff[i] == 0x7E && rx_buff[i + 9] == 0xEF) {
				uint16_t check_sum = mp3_calcSum(&rx_buff[i + 1], 6);
				if (check_sum == ((rx_buff[i + 7] << 8) | rx_buff[i + 8])) {
					mp3_process_rx_data(&rx_buff[i]);
					remain = rx_idx - (i + 9);
					i += 9;
				}
			}
		}
		rx_idx = remain;
		memcpy(rx_buff, &rx_buff[i], remain);
	}
}

static void mp3_tx_check(void) {
	if (tx_buff_remained == 0) {
		return;
	}
	if (command_timeout < xTaskGetTickCount()) {
		DEBUG_INFO(
				"MP3 Tx:%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
				tx_buff[tx_buff_out][0], tx_buff[tx_buff_out][1],
				tx_buff[tx_buff_out][2], tx_buff[tx_buff_out][3],
				tx_buff[tx_buff_out][4], tx_buff[tx_buff_out][5],
				tx_buff[tx_buff_out][6], tx_buff[tx_buff_out][7],
				tx_buff[tx_buff_out][8], tx_buff[tx_buff_out][9]);
		UART_Transmit(UART_MP3_CHANNEL, tx_buff[tx_buff_out], 10);
		command_timeout = xTaskGetTickCount() + 100;
		command_try_cnt++;
		if (command_try_cnt == 3) {
			tx_buff_out++;
			tx_buff_remained--;
			if (tx_buff_out == TX_BUF_SZIE) {
				tx_buff_out = 0;
			}
		}

	}
}

static void xTask(void *pPara) {
	(void) pPara;
	uint32_t pre_tick;
	Msg_t msg;
	uint8_t i;
	vTaskDelay(100);
	mp3_send_command(0x0b, 0);
	for (i = 0; i < 50; i++) {
		mp3_rx_check();
		if (is_ready == true) {
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
	is_ready = true;

	mp3_play_starting_music();
	pre_tick = xTaskGetTickCount();
	for (;;) {
		if (xQueueReceive(xQueue, &msg, pdMS_TO_TICKS(20)) == pdPASS) {
			if (tx_buff_remained < TX_BUF_SZIE) {

				uint8_t cmd[10] = { 0x7E, 0xff, 0x06, msg.cmd, 0x01,
						(uint8_t) (msg.data >> 8), (uint8_t) msg.data, 0, 0,
						0xEF };
				mp3_doSum(&cmd[1], 6);
				memcpy(tx_buff[tx_buff_in++], cmd, 10);
				if (tx_buff_in == TX_BUF_SZIE) {
					tx_buff_in = 0;
				}
				tx_buff_remained++;
			}
		}

		if (pre_tick + 50 < xTaskGetTickCount()) {
			mp3_tx_check();
			pre_tick = xTaskGetTickCount();
		}
		mp3_rx_check();
	}
}

void mp3_init(void) {
	DEBUG_INFO("mp3 init\r\n");
	Uart_Initialize(UART_MP3_CHANNEL);
	xQueue = xQueueCreate(5, sizeof(Msg_t));
	if (xQueue == NULL) {
		return;
	}
	if (pdPASS != xTaskCreate(xTask, "MP3", 128, NULL, 4, NULL)) {
		return;
	}
}

RET_t mp3_send_command(uint8_t command, uint16_t data) {
	Msg_t msg;
	msg.cmd = command;
	msg.data = data;
	if (pdPASS == xQueueSend(xQueue, &msg, pdMS_TO_TICKS(10))) {
		return RET_OK;
	}
	return RET_ERR;
}

bool mp3_is_device_ready(void) {
	return is_ready;
}

mp3_status_t mp3_get_status(void) {
	return (mp3_status_t) (status);
}

void mp3_play_starting_music(void) {
	mp3_send_command(0x16, 0x0000);
	mp3_send_command(0x06, 0x000F);
	mp3_send_command(0x08, 0x0001);
	mp3_send_command(0x19, 0);
	status = mp3_playing;
}
static void mp3_doSum(uint8_t *pStr, uint8_t len) {
	uint16_t xorsum = 0;
	uint8_t i;
	for (i = 0; i < len; i++) {
		xorsum += pStr[i];
	}
	xorsum = 0 - xorsum;
	pStr[i] = (uint8_t) (xorsum >> 8);
	pStr[i + 1] = (uint8_t) (xorsum & 0xFF);
}

static uint16_t mp3_calcSum(uint8_t *pStr, uint8_t len) {
	uint16_t xorsum = 0;
	uint8_t i;
	for (i = 0; i < len; i++) {
		xorsum += pStr[i];
	}
	return 0 - xorsum;
}

