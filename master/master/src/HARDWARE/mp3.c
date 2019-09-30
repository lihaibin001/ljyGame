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
#include "profile.h"

#include <string.h>

typedef struct {
	uint8_t cmd;
	uint16_t data;
} mp3_tx_t;

typedef struct {
	uint8_t mp3_num;
	uint8_t cmd;
	uint16_t data;
} Msg_t;
#define RX_BUF_SZIE 128
#define TX_BUF_SZIE 12
typedef struct {
	uint8_t rx_buff[RX_BUF_SZIE];
	uint8_t tx_buff[TX_BUF_SZIE][10];
	uint8_t rx_idx;
	uint8_t tx_buff_in;
	uint8_t tx_buff_out;
	uint8_t command_try_cnt;
	uint32_t command_timeout;
	uint8_t tx_remained;
	uint8_t uart_channel;
}mp3_tag_t;



static bool is_ready;
static uint8_t status;
//static TimerHandle_t xTimers;
static QueueSetHandle_t xQueue;

static mp3_tag_t mp3_tag[2] =
		{
			{
				.uart_channel = UART_MP3_CHANNEL_2,
			},
			{
				.uart_channel = UART_MP3_CHANNEL_1,
			}
		};

static void mp3_doSum(uint8_t *pStr, uint8_t len);
static uint16_t mp3_calcSum(uint8_t *pStr, uint8_t len);
static void mp3_process_rx_data(uint8_t *pData, uint8_t mp3_num);

static void mp3_process_rx_data(uint8_t *pData, uint8_t mp3_num) {
	DEBUG_INFO("MP3 %d Rx:%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
			mp3_num,
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
		if (mp3_tag[mp3_num].tx_remained == 0) {
			return;
		}
		mp3_tag[mp3_num].tx_buff_out++;
		mp3_tag[mp3_num].tx_remained--;
		mp3_tag[mp3_num].command_try_cnt = 0;
		mp3_tag[mp3_num].command_timeout = 0;
		if (mp3_tag[mp3_num].tx_buff_out == TX_BUF_SZIE) {
			mp3_tag[mp3_num].tx_buff_out = 0;
		}
		break;
	}
}
static void mp3_rx_check(void) {
	uint8_t i, j;
	for(j=0; j<2; j++)
	{
		while (Uart_Get_Char(mp3_tag[j].uart_channel, &mp3_tag[j].rx_buff[mp3_tag[j].rx_idx])) {
			if (++mp3_tag[j].rx_idx == RX_BUF_SZIE) {
				break;
			}
		}
		if (mp3_tag[j].rx_idx >= 9) {
			uint8_t remain = mp3_tag[j].rx_idx;
			for (i = 0; i <= mp3_tag[j].rx_idx - 9; i++) {
				if (mp3_tag[j].rx_buff[i] == 0x7E && mp3_tag[j].rx_buff[i + 9] == 0xEF) {
					uint16_t check_sum = mp3_calcSum(&mp3_tag[j].rx_buff[i + 1], 6);
					if (check_sum == ((mp3_tag[j].rx_buff[i + 7] << 8) | mp3_tag[j].rx_buff[i + 8])) {
						mp3_process_rx_data(&mp3_tag[j].rx_buff[i], j);
						remain = mp3_tag[j].rx_idx - (i + 9);
						i += 9;
					}
				}
			}
			mp3_tag[j].rx_idx = remain;
			memcpy(mp3_tag[j].rx_buff, &mp3_tag[j].rx_buff[i], remain);
		}
	}
}

static void mp3_tx_check(void) {
	uint8_t j;
	for(j=0; j<2; j++) {
		if (mp3_tag[j].tx_remained == 0) {
			continue;
		} else {
			if (mp3_tag[j].command_timeout < xTaskGetTickCount()) {
				DEBUG_INFO(
						"MP3 %d Tx:%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
						j,
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][0],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][1],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][2],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][3],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][4],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][5],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][6],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][7],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][8],
						mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out][9]);
				UART_Transmit(mp3_tag[j].uart_channel, mp3_tag[j].tx_buff[mp3_tag[j].tx_buff_out], 10);
				mp3_tag[j].command_timeout = xTaskGetTickCount() + 100;
				mp3_tag[j].command_try_cnt++;
				if (mp3_tag[j].command_try_cnt == 3) {
					mp3_tag[j].command_try_cnt = 0;
					mp3_tag[j].tx_buff_out++;
					mp3_tag[j].tx_remained--;
					if (mp3_tag[j].tx_buff_out == TX_BUF_SZIE) {
						mp3_tag[j].tx_buff_out = 0;
					}
				}
			}
		}
	}
}

static void xTask(void *pPara) {
	(void) pPara;
	uint32_t pre_tick;
	Msg_t msg;
	vTaskDelay(pdMS_TO_TICKS(50));
	mp3_send_command(0x0b, 0, 0);
	mp3_send_command(0x0b, 0, 1);
	//mp3_play_starting_music();
	pre_tick = xTaskGetTickCount();
	for (;;) {
		if (xQueueReceive(xQueue, &msg, pdMS_TO_TICKS(20)) == pdPASS) {
			if (mp3_tag[msg.mp3_num].tx_remained < TX_BUF_SZIE) {

				uint8_t cmd[10] = { 0x7E, 0xff, 0x06, msg.cmd, 0x01,
						(uint8_t) (msg.data >> 8), (uint8_t) msg.data, 0, 0,
						0xEF };
				mp3_doSum(&cmd[1], 6);
				memcpy(mp3_tag[msg.mp3_num].tx_buff[mp3_tag[msg.mp3_num].tx_buff_in++], cmd, 10);
				if (mp3_tag[msg.mp3_num].tx_buff_in == TX_BUF_SZIE) {
					mp3_tag[msg.mp3_num].tx_buff_in = 0;
				}
				mp3_tag[msg.mp3_num].tx_remained++;
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
	Uart_Initialize(UART_MP3_CHANNEL_1);
	Uart_Initialize(UART_MP3_CHANNEL_2);
	xQueue = xQueueCreate(5, sizeof(Msg_t));
	if (xQueue == NULL) {
		return;
	}
	if (pdPASS != xTaskCreate(xTask, "MP3", 128, NULL, 4, NULL)) {
		return;
	}
}

RET_t mp3_send_command(uint8_t command, uint16_t data, uint8_t mp3_num) {
	Msg_t msg;
	msg.mp3_num = mp3_num;
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
	//mp3_send_command(0x16, 0x0000);
	mp3_send_command(0x06, profile_get_volume(), 0);
	mp3_send_command(0x08, 0x0001, 0);
	mp3_send_command(0x19, 0, 0);
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

