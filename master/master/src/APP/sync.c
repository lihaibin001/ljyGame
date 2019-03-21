/*
 * sync.c
 *
 *  Created on: 2019��3��4��
 *      Author: ecarx
 */
#include "CanApp.h"
#include "sync.h"
#include "fsm.h"
#include "maxtrixApp.h"
#include "RGBMatrix.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include <string.h>
#include <rand.h>

typedef uint8_t PS_Current_State_T;
typedef uint8_t PS_Flags_T;

static TimerHandle_t xTimers;
static TimerHandle_t exc_timers;
static TickType_t send_peridic = 2000;
static uint16_t exc_time;

/* function declaration */
static void xTask(void *p);
static uint8_t ps_start_action(uint16_t data);
static uint8_t no_action(uint16_t data);
static uint8_t cs_no_action(void);
static uint8_t ps_check_selftest(uint16_t data);
static uint8_t ps_mode_change(uint16_t data);
static uint8_t ps_page(uint16_t data);
static uint8_t ps_start_game(uint16_t data);
static uint8_t ps_snatch_handler(uint16_t data);
static uint8_t ps_entry_idle(uint16_t data);
static uint8_t ps_entery_fault(uint16_t data);
static uint8_t ps_entry_snatch_led(uint16_t data);
static uint8_t ps_entry_road_block(uint16_t data);
static uint8_t ps_entry_wipe_led(uint16_t data);
static uint8_t ps_entry_agil_train(uint16_t data);
static uint8_t ps_entry_root(uint16_t data);
static uint8_t ps_entry_boot_test(uint16_t data);
//static uint8_t ps_cs_root(void);
//static uint8_t ps_cs_idle(void);
static uint8_t ps_cs_boot_test(void);
static uint8_t ps_cs_snatch_led(void);
static uint8_t ps_cs_road_block(void);
static uint8_t ps_cs_wipe_led(void);
static uint8_t ps_cs_agility_training(void);
//static uint8_t ps_cs_game_stop(void);
static uint8_t ps_cs_fault(void);
static void ps_send_command(uint8_t id);
static void xTimerHandler(void *p);
static void exc_timerHandler(void *p);

/********************************/
/* STATE TRANSITION DESCRIPTION */
/********************************/
#include "fsm_tran.h"
#include "ps_stt.h"

/********************************/
/* STATE TREE DESCRIPTION       */
/********************************/
#include "fsm_tree.h"
#include "ps_stt.h"

enum {
	PLAY_MODE_UNKNOW,
	SNATCH_LED,
	ROAD_BLOCK,
	WIPE_LED,
	AGIL_TRAIN,
	PLAY_MODE_AMOUNT,
};
static uint8_t salverid[2];
static uint8_t current_status;
static QueueHandle_t xQueue;
static uint8_t play_mode;
const static uint8_t *play_mode_tbl[] = { snatch_led, road_block, wipe_led,
		agil_train, };

void ps_task_create(void) {
	xQueue = xQueueCreate(3, sizeof(Data_Message_T));
	xTimers = xTimerCreate("sync timer", pdMS_TO_TICKS(send_peridic), pdTRUE,
			(void*) 0, xTimerHandler);
	exc_timers = xTimerCreate("exc timer", pdMS_TO_TICKS(send_peridic), pdTRUE,
			(void*) 0, exc_timerHandler);
	xTaskCreate(xTask, "sync", 128, NULL, 3, NULL);
}

void ps_plate_data_handler(uint32_t id, uint8_t data, uint8_t type) {
//	switch(type) {
//	case :
//	}
}
/**********************************************************************
 *
 *    Function: PS_Task
 *
 *  Parameters: none
 *
 *     Returns: none
 *
 * Description: Entry point for the psync task.
 *
 **********************************************************************/
static void xTask(void *p) {
	(void) p;
	Data_Message_T msg;
	BaseType_t status;
	msg.parts.msg = START;
	msg.parts.data = 0;
	uint8_t new_state = FSM_Process_Evt(msg, PS_ROOT, tree_psync);
	for (;;) {

		// wait until next event in mailbox OR time expired
		status = xQueueReceive(xQueue, &msg, pdMS_TO_TICKS(250));
		// process ALL events in queue first !!!
		while (pdPASS == status) {
			// process event
			new_state = FSM_Process_Evt(msg, current_status, tree_psync);

			if (new_state != current_status) {
				current_status = new_state;
			}

			// get next event from mailbox without waiting
			status = xQueueReceive(xQueue, &msg, 0);
		}

		// process all cs routines for the active state tree branch
		FSM_Process_CS(current_status, tree_psync);
	}
}

/*********************************************************************/
/* entry actions                                                     */
/*********************************************************************/
static uint8_t ps_entry_root(uint16_t data) {
	if (current_status == PS_ROOT) {
		current_status = PS_BOOT_TEST;
	}
	return PS_ROOT;
}

static uint8_t ps_entry_boot_test(uint16_t data) {
	uint8_t i;
	for (i = 0; i < PLATE_AMOUNT; i++) {
		maxtrixAppSetPlateStatus(i, PLATE_STA_UNKNOW);
	}
	return PS_BOOT_TEST;
}

static uint8_t ps_entry_idle(uint16_t data) {
	RGBClearBuff();
	RGBrawString(15, 12, 0x0000FF, "WAN DE V1.1");
	current_status = PS_IDLE;
	return PS_IDLE;
}

static uint8_t ps_entery_fault(uint16_t data) {
	char drawBuff[16] = "";
	uint8_t status;
	memset(drawBuff, 0, 4);
	if (maxtrixAppGetPlateStatue(&status)) {
		sprintf(drawBuff, "ERROR: %X !!!", status);
	}
	RGBClearBuff();
	RGBrawString(8, 12, 0x0000FF, drawBuff);
	return PS_FAULT;
}

static uint8_t ps_entry_snatch_led(uint16_t data) {
	RGBClearBuff();
	uint8_t i;
	for (i = 3; i >= 1; i--) {
		RGBClearBuff();
		RGBdrawImage(28, 10, 0xFF, pNumber[i]);
		vTaskDelay(1000);
	}
	maxtriAppStartTime();
	maxtrixAppGameStart();
	srand(xTaskGetTickCount());
	if(maxtrixAppGetGameMode() == 0) {
		salverid[0] = rand() % 8;
		ps_send_command(salverid[0]);
	} else {
		salverid[0] = rand() % 4;
		ps_send_command(salverid[0]);
		salverid[1] = rand() % 4 + 4;
		ps_send_command(salverid[1]);
	}
	return PS_SNATCH_LED;
}

static uint8_t ps_entry_road_block(uint16_t data) {
	RGBClearBuff();
	uint8_t i;
	for (i = 3; i >= 1; i--) {
		RGBClearBuff();
		RGBdrawImage(28, 10, 0xFF, pNumber[i]);
		vTaskDelay(1000);
	}
	maxtrixAppGameStart();
	return PS_ROAD_BLOCK;
}

static uint8_t ps_entry_wipe_led(uint16_t data) {
	RGBClearBuff();
	uint8_t i;
	for (i = 3; i >= 1; i--) {
		RGBClearBuff();
		RGBdrawImage(28, 10, 0xFF, pNumber[i]);
		vTaskDelay(1000);
	}
	maxtrixAppGameStart();
	return PS_WIPE_LED;
}

static uint8_t ps_entry_agil_train(uint16_t data) {
	RGBClearBuff();
	uint8_t i;
	for (i = 3; i >= 1; i--) {
		RGBClearBuff();
		RGBdrawImage(28, 10, 0xFF, pNumber[i]);
		vTaskDelay(1000);
	}
	maxtrixAppGameStart();
	return PS_AGIL_TRAIN;
}

/*********************************************************************/
/* cs routines                                                       */
/*********************************************************************/
//static uint8_t ps_cs_root(void) {
//	return 0;
//}
//static uint8_t ps_cs_idle(void) {
//	return 0;
//}
static uint8_t ps_cs_boot_test(void) {
	uint8_t status = 0;
	if (maxtrixAppSelfTest()) {
		if (maxtrixAppGetPlateStatue(&status)) {
			if (status != 0) {
				//ps_send_event(PS_EVT_FAULT, 0);
				ps_send_event(PS_EVT_BOOT, 0);
			} else {
				ps_send_event(PS_EVT_BOOT, 0);
			}
		}
	}
	return 0;
}

static void ps_send_command(uint8_t id) {
	uint8_t color;
	uint32_t rand_num;
	can_frame_t msg;

	do {

		rand_num = rand();
	} while (rand_num == 0);
	do {
		salverid[0] = rand();
	}while(salverid[0] == 0);
	if (maxtrixAppGetGameMode() == 1) { //single mode

		color = rand_num % 8;
	} else { //double mode
		color = rand_num % 4;
	};

	msg.dataByte1 = id;
	if (color == 0) {
		msg.dataByte2 = 1;
	} else {
		msg.dataByte2 = 0;
	}
	msg.dataByte0 = PROTOCAL_LED_ON;
	CanAppSendMsg(&msg);
}

static uint8_t ps_cs_snatch_led(void) {
	return 0;
}

//static uint8_t ps_send_can_msg(can_frame_t *pMsg) {
//
//}

static uint8_t ps_cs_road_block(void) {
	return 0;
}

static uint8_t ps_cs_wipe_led(void) {
	return 0;
}

static uint8_t ps_cs_agility_training(void) {
	return 0;
}

//static uint8_t ps_cs_game_stop(void) {
//	return 0;
//}

static uint8_t ps_cs_fault(void) {

	return 0;
}
/*********************************************************************/
/* exit actions                                                      */
/*********************************************************************/

/*********************************************************************/
/* start actions                                                     */
/*********************************************************************/
static uint8_t ps_start_action(uint16_t data) {
	return 0;
}

/*********************************************************************/
/*no actions                                                     */
/*********************************************************************/
static uint8_t no_action(uint16_t data) {
	return (0);
}

static uint8_t cs_no_action(void) {
	return 0;
}

static uint8_t ps_check_selftest(uint16_t data) {
	maxtrixAppSetPlateStatus((uint8_t) (data - 1), PLATE_STA_OK);
	return 0;
}

static uint8_t ps_mode_change(uint16_t data) {
	maxtrixAppSetGameMode(data);
	return 0;
}

static uint8_t ps_page(uint16_t data) {
	if (data == 0) { //page up
		if (play_mode == PLAY_MODE_AMOUNT) {
			play_mode = SNATCH_LED;
		} else {
			play_mode++;
		}
	} else { //page down
		if (play_mode == PLAY_MODE_UNKNOW) {
			play_mode = AGIL_TRAIN;
		} else {
			play_mode--;
		}
	}
	RGBClearBuff();
	RGBShowImage(0xFF, play_mode_tbl[play_mode - 1]);
	return 0;
}

static uint8_t ps_start_game(uint16_t data) {

	return play_mode;
}

static uint8_t ps_snatch_handler(uint16_t data) {
	uint8_t id = (uint8_t) (data >> 8);
	uint8_t color = (uint8_t) data;
	if(maxtrixAppGetGameMode() == 0) { //single mode
		if(id == salverid[0]) {
			if(color == 0) { //red
				maxtriAppScoreIncrease(0);
			} else {
				maxtriAppScoreDecrease(0);
			}
		}
	} else {

	}
	return 0;
}

static void xTimerHandler(void *p) {

}
static void exc_timerHandler(void *p) {

}

uint8_t ps_send_event(uint16_t event, int16_t data) {
	Data_Message_T msg;
	uint8_t ret = 1;
	msg.parts.msg = event;
	msg.parts.data = data;
	if (pdPASS == xQueueSend(xQueue, &msg, 100)) {
		ret = 0;
	}
	return ret;
}

uint8_t ps_send_event_from_irq(uint16_t event, int16_t data) {
	Data_Message_T msg;
	uint8_t ret = 1;
	msg.parts.msg = event;
	msg.parts.data = data;
	if (pdPASS == xQueueSendFromISR(xQueue, &msg, NULL)) {
		ret = 0;
	}
	return ret;
}
