/*
 * sync.c
 *
 *  Created on: 2019��3��4��
 *      Author: ecarx
 */

#include "sync.h"

#include "fsm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "maxtrixApp.h"
#include "RGBMatrix.h"
#include "string.h"

typedef uint8_t PS_Current_State_T;
typedef uint8_t PS_Flags_T;

/* function declaration */
static void xTask(void *p);
static uint8_t ps_start_action(uint16_t data);
static uint8_t no_action(uint16_t data);
static uint8_t cs_no_action(void);
static uint8_t ps_check_selftest(uint16_t data);
static uint8_t ps_entry_idle(uint16_t data);
static uint8_t ps_entery_fault(uint16_t data);
static uint8_t ps_entry_root(uint16_t data);
static uint8_t ps_entry_boot_test(uint16_t data);
//static uint8_t ps_cs_root(void);
//static uint8_t ps_cs_idle(void);
static uint8_t ps_cs_boot_test(void);
static uint8_t ps_cs_snatch_led(void);
static uint8_t ps_cs_road_block(void);
static uint8_t ps_cs_wipe_led(void);
static uint8_t ps_cs_agility_training(void);
static uint8_t ps_cs_game_stop(void);
static uint8_t ps_cs_fault(void);

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

static uint8_t current_status;
static QueueHandle_t xQueue;



void ps_task_create(void) {
	xQueue = xQueueCreate(3, sizeof(Data_Message_T));
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
    msg.parts.msg  = START;
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
	if(current_status == PS_ROOT) {
		current_status = PS_BOOT_TEST;
	} else {
		current_status = PS_IDLE;
	}
	return 0;
}

static uint8_t ps_entry_boot_test(uint16_t data) {
	uint8_t i;
	for(i = 0; i < PLATE_AMOUNT; i++) {
		maxtrixAppSetPlateStatus(i, PLATE_STA_UNKNOW);
	}
	return 0;
}

static uint8_t ps_entry_idle(uint16_t data) {
	RGBClearBuff();
	RGBrawString(18, 12, 0x0000FF, "WAN DE");
	return 0;
}

static uint8_t ps_entery_fault(uint16_t data) {
	char drawBuff[16] = "";
	uint8_t status;
	memset(drawBuff, 0, 4);
	if(maxtrixAppGetPlateStatue(&status)) {
		sprintf(drawBuff, "ERROR: %X !!!", status);
	}
	RGBClearBuff();
	RGBrawString(8, 12, 0x0000FF, drawBuff);
	return 0;
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
	if(maxtrixAppSelfTest()) {
		if(maxtrixAppGetPlateStatue(&status)) {
			if(status != 0) {
				ps_send_event(PS_EVT_FAULT, 0);
			} else {
				ps_send_event(PS_EVT_BOOT, 0);
			}
		}
	}
	return 0;
}

static uint8_t ps_cs_snatch_led(void) {
	return 0;
}

static uint8_t ps_cs_road_block(void) {
	return 0;
}

static uint8_t ps_cs_wipe_led(void) {
	return 0;
}

static uint8_t ps_cs_agility_training(void) {
	return 0;
}

static uint8_t ps_cs_game_stop(void) {
	return 0;
}

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
	maxtrixAppSetPlateStatus((uint8_t)(data - 1), PLATE_STA_OK);
	return 0;
}

uint8_t ps_send_event(uint16_t event, int16_t data) {
	Data_Message_T msg;
	uint8_t ret = 1;
	msg.parts.msg = event;
	msg.parts.data = data;
	if(pdPASS == xQueueSend(xQueue, &msg, 0)) {
		ret = 0;
	}
	return ret;
}
