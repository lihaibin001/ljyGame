#include "delay.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "debug.h"
#include "profile.h"
#include "uart.h"
#include "sys.h"
#include "crc16.h"
#include "string.h"

static QueueSetHandle_t xQueue;
static TaskHandle_t comTaskHanle;

#define START_FALG 0x5A
#define STOP_FLAG 0xA5

#define RX_BUFF_LEN 256
#define TX_BUFF_LEN 256

#define ERROR_CODE_NO_ERR 0
#define ERROR_CODE_CRC_ERR 1
#define ERROR_CODE_DATA_PARSE_ERR 2

#define MODIFY_SNATCH_LED_INTERVAL	1
#define MODIFY_ROAD_BLOCK_INTERVAL	2
#define MODIFY_WIPE_LED_INTERVAL	3
#define MODIFY_AGIL_TRAIN_INTRVAL	4
#define MODIFY_IDLE_DURATION		5
#define MODIFY_DISPLAYOFF_DURATION	6
#define MODIFY_FACTORY_RESET		7

static uint8_t com_rx_buff[RX_BUFF_LEN];
static uint8_t rx_idx;

static void com_positive_response(void) {
	uint8_t resp[6] = {0x5A, 3, 0, 0, 0, 0xA5};
	UART_Transmit(UART_DEBUG_CHANNEL, resp, 6);
}

static void com_nagitave_response(uint8_t errorcode) {
	uint8_t resp[7] = {0x5A, 4, 1, errorcode, 0, 0, 0xA5};
	UART_Transmit(UART_DEBUG_CHANNEL, resp, 7);
}

static uint8_t com_handle_rx_data(uint8_t *rxdata, int8_t len) {
	while(len > 0){
		switch(*rxdata) {
		case MODIFY_SNATCH_LED_INTERVAL:
			if(len >= 17) {
				len -= 17;
				rxdata++;
				uint32_t intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_SNATCH_LED, 0, intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_SNATCH_LED, 1,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_SNATCH_LED, 2,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				rxdata+=4;
				profile_set_blink_interval(PROFILE_SNATCH_LED, 3,intverval);
			} else {
				DEBUG_ERROR("<COM> parse data err\r\n");
				return ERROR_CODE_DATA_PARSE_ERR;
			}
			break;
		case MODIFY_ROAD_BLOCK_INTERVAL:
			if(len >= 17) {
				len -= 17;
				rxdata++;
				uint32_t intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 0, intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 1,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 2,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				rxdata+=4;
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 3,intverval);
			} else {
				DEBUG_ERROR("<COM> parse data err\r\n");
				return ERROR_CODE_DATA_PARSE_ERR;
			}
			break;
		case MODIFY_WIPE_LED_INTERVAL:
			if(len >= 17) {
				len -= 17;
				rxdata++;
				uint32_t intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 0, intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 1,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 2,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				rxdata+=4;
				profile_set_blink_interval(PROFILE_ROAD_BLOCK, 3,intverval);
			} else {
				DEBUG_ERROR("<COM> parse data err\r\n");
				return ERROR_CODE_DATA_PARSE_ERR;
			}
			break;
		case MODIFY_AGIL_TRAIN_INTRVAL:
			if(len >= 17) {
				len -= 17;
				rxdata++;
				uint32_t intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_AGIL_TRAIN, 0, intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_AGIL_TRAIN, 1,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_blink_interval(PROFILE_AGIL_TRAIN, 2,intverval);
				rxdata+=4;
				intverval = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				rxdata+=4;
				profile_set_blink_interval(PROFILE_AGIL_TRAIN, 3,intverval);
			} else {
				DEBUG_ERROR("<COM> parse data err\r\n");
				return ERROR_CODE_DATA_PARSE_ERR;
			}
			break;
		case MODIFY_IDLE_DURATION:
			if(len >= 5) {
				len -= 5;
				rxdata++;
				uint32_t duration = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_idle_duratoin(duration);
			}
			break;
		case MODIFY_DISPLAYOFF_DURATION:
			if(len >= 5) {
				len -= 5;
				rxdata++;
				uint32_t duration = rxdata[0] << 24 | rxdata[1] << 16 | rxdata[2] << 8 | rxdata[3];
				profile_set_displayoff_duratoin(duration);
			}
			break;
		case MODIFY_FACTORY_RESET:
			profile_factory_reset();
			com_positive_response();
			return ERROR_CODE_NO_ERR;
		default:
			return ERROR_CODE_DATA_PARSE_ERR;
		}
	}
	profile_save();
	com_positive_response();
	return ERROR_CODE_NO_ERR;
}

static void com_port_rx_check(void) {
	static bool find_escape;
	uint8_t byte = 0;
	while (Uart_Get_Char(UART_DEBUG_CHANNEL, &byte)) {
		if(byte == 0x5A && !find_escape) {
			rx_idx = 0;
		}
		else if(byte == 0xA5 && !find_escape) {
			uint8_t err_code;
			uint16_t crc = crc16_cal(com_rx_buff, rx_idx-2);
			if(crc == (com_rx_buff[rx_idx-1] << 8 | com_rx_buff[rx_idx])) {
				err_code = com_handle_rx_data(com_rx_buff, rx_idx-2);
				if(err_code != 0) {
					com_nagitave_response(err_code);
				}
			} else {
				com_nagitave_response(ERROR_CODE_CRC_ERR);
			}
		} else if(byte == '\\') {
			find_escape = true;
		} else {
			if(rx_idx >= RX_BUFF_LEN) {
				com_nagitave_response(ERROR_CODE_DATA_PARSE_ERR);
				rx_idx = 0;
			}
			com_rx_buff[rx_idx++] = byte;
			if(find_escape) {
				find_escape = false;
			}
		}
	}
}

void xTask(void *p) {
    uint8_t data = 0;
    for(;;)
    {
		xQueueReceive(xQueue, &data, pdMS_TO_TICKS(20));
		com_port_rx_check();
    }
}

void com_init(void) {
	xQueue = xQueueCreate(1, 1);
    if(xQueue == NULL)
    {
    	DEBUG_ERROR("<COM> create queue fail\r\n");
        return ;
    }
	if(pdPASS !=  xTaskCreate(xTask, "com", 128, NULL, 4, &comTaskHanle))
    {
		DEBUG_ERROR("<COM> create task fail\r\n");
    }
}
