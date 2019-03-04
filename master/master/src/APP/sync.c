/*
 * sync.c
 *
 *  Created on: 2019��3��4��
 *      Author: ecarx
 */

#include "sync.h"
#include <stdint.h>
#include <stdio.h>
#include "fsm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "maxtrixApp.h"

typedef uint8_t PS_Current_State_T;
typedef uint8_t PS_Flags_T;

static void xTask(void *p);
static uint8_t ps_start_action(void);
static uint8_t no_action(void);
static uint8_t ps_entry_idle(void);
static uint8_t ps_entry_root(void);
static uint8_t ps_entry_boot_test(void);
static uint8_t ps_cs_root(void);
static uint8_t ps_cs_idle(void);
static uint8_t ps_cs_boot_test(void);
static uint8_t ps_cs_snatch_led(void);
static uint8_t ps_cs_road_block(void);
static uint8_t ps_cs_wipe_led(void);
static uint8_t ps_cs_agility_training(void);
static uint8_t ps_cs_game_stop(void);
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
	xQueue = xQueueCreate(3, 1);
	xTaskCreate(xTask, "sync", 128, NULL, 3, NULL);
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
	PS_Current_State_T new_state;

	for (;;) {

		// wait until next event in mailbox OR time expired
		status = xQueueReceive(xQueue, &msg, pdMS_TO_TICKS(100));
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
static uint8_t ps_entry_root(void) {
	current_status = PS_BOOT_TEST;
	return 0;
}

static uint8_t ps_entry_boot_test(void) {
	return 0;
}

static uint8_t ps_entry_idle(void) {
	return 0;
}

/*********************************************************************/
/* cs routines                                                       */
/*********************************************************************/
static uint8_t ps_cs_root(void) {
	return 0;
}

static uint8_t ps_cs_idle(void) {

	return 0;
}

static uint8_t ps_cs_boot_test(void) {
	maxtrixAppBooting();
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

/*********************************************************************/
/* exit actions                                                      */
/*********************************************************************/

/*********************************************************************/
/* start actions                                                     */
/*********************************************************************/
static uint8_t ps_start_action(void) {
	return 0;
}

/*********************************************************************/
/*no actions                                                     */
/*********************************************************************/
static uint8_t no_action(void) {
	return (0);
}
