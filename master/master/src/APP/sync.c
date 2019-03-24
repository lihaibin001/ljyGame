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

static TimerHandle_t xTimersPlayer1;
static TimerHandle_t xTimersPlayer2;
static uint32_t perdic1 = 2000;
static uint32_t perdic2 = 2000;

static uint8_t plate_on_cnt[2];

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

static void xTimer1Handler(void *p);
static void xTimer2Handler(void *p);

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
	xQueue = xQueueCreate(5, sizeof(Data_Message_T));
	xTimersPlayer1 = xTimerCreate("player1timer", 0, pdFALSE, (void*) NULL,
			xTimer1Handler);
	xTimersPlayer2 = xTimerCreate("player2timer", 0, pdFALSE, (void*) NULL,
			xTimer2Handler);
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
	xTimerStop(xTimersPlayer1, 100);
	xTimerStop(xTimersPlayer2, 100);
	maxtriAppStopTime();
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

	uint8_t i;
	can_frame_t msg;

	msg.dataByte0 = PROTOCAL_LED_ON;
	for (i = 3; i >= 1; i--) {
		RGBClearBuff();
		RGBdrawImage(28, 10, 0xFF, pNumber[i]);
		vTaskDelay(1000);
	}
	plate_on_cnt[0] = 1;
	maxtriAppStartTime();
	RGBClearBuff();
	maxtrixAppGameStart();
	srand(xTaskGetTickCount());
	if (maxtrixAppGetGameMode() == 0) {
		salverid[0] = rand() % PLATE_AMOUNT + 1;
		msg.dataByte1 = salverid[0];
		msg.dataByte2 = rand() % PLATE_AMOUNT;
		msg.dataByte3 = (uint8_t) (perdic1 / 100);
		CanAppSendMsg(&msg);
		xTimerStart(xTimersPlayer1, 100);

	} else {
		salverid[0] = rand() % (PLATE_AMOUNT / 2) + 1;
		msg.dataByte1 = salverid[0];
		msg.dataByte2 = rand() % (PLATE_AMOUNT / 2);
		msg.dataByte3 = (uint8_t) (perdic1 / 100);
		CanAppSendMsg(&msg);
		xTimerStart(xTimersPlayer1, 100);

		salverid[1] = rand() % (PLATE_AMOUNT / 2) + (PLATE_AMOUNT / 2) + 1;
		msg.dataByte1 = salverid[1];
		msg.dataByte2 = rand() % (PLATE_AMOUNT / 2);
		msg.dataByte3 = (uint8_t) (perdic2 / 100);
		CanAppSendMsg(&msg);
		xTimerStart(xTimersPlayer2, 100);
		plate_on_cnt[1] = 1;
	}
	return PS_SNATCH_LED;
}

static uint8_t ps_entry_road_block(uint16_t data) {

	return PS_ROAD_BLOCK;
}

static uint8_t ps_entry_wipe_led(uint16_t data) {

	return PS_WIPE_LED;
}

static uint8_t ps_entry_agil_train(uint16_t data) {
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
	RGBClearBuff();
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
		if (play_mode == PLAY_MODE_AMOUNT) {
			play_mode = SNATCH_LED;
		}
	} else { //page down
		if (play_mode == PLAY_MODE_UNKNOW) {
			play_mode = AGIL_TRAIN;
		} else {
			play_mode--;
		}
		if (play_mode == PLAY_MODE_UNKNOW) {
			play_mode = AGIL_TRAIN;
		}
	}
	RGBClearBuff();
	RGBShowImage(0xFF, play_mode_tbl[play_mode - 1]);
	return 0;
}

static uint8_t ps_start_game(uint16_t data) {
	if (play_mode == PLAY_MODE_UNKNOW) {
		play_mode = SNATCH_LED;
	}
	return play_mode;
}

static uint8_t ps_snatch_handler(uint16_t data) {
	uint8_t id = (uint8_t) (data >> 8);
	uint8_t color = (uint8_t) data;

	if (maxtrixAppGetGameMode() == 0) { //single mode

		if (id == salverid[0]) {
			if (color != 0xFF) { //event is not player touch the plate
				if (color == 0) { //red
					maxtriAppScoreIncrease(0);
				} else {
					maxtriAppScoreDecrease(0);
				}
			}
		}
		if (maxtriAppGetScore(0) > 45) {
			perdic1 = 500;
		} else if (maxtriAppGetScore(0) > 30) {
			perdic1 = 1000;
		} else if (maxtriAppGetScore(0) > 15) {
			perdic1 = 1500;
		} else {
			perdic1 = 2000;
		}
		xTimerStop(xTimersPlayer1, 100);
		xTimerChangePeriod(xTimersPlayer1, perdic1, 100);
		xTimerStart(xTimersPlayer1, 100);
	} else { //double mode
		if (id == salverid[0]) {
			if (color != 0xFF) {
				if (color == 0) {
					maxtriAppScoreIncrease(0);
				} else {
					maxtriAppScoreIncrease(1);
				}
			}

			if (maxtriAppGetScore(0) > 45) {
				perdic1 = 500;
			} else if (maxtriAppGetScore(0) > 30) {
				perdic1 = 1000;
			} else if (maxtriAppGetScore(0) > 15) {
				perdic1 = 1500;
			} else {
				perdic1 = 2000;
			}
			xTimerStop(xTimersPlayer1, 100);
			xTimerChangePeriod(xTimersPlayer1, perdic1, 100);
			xTimerStart(xTimersPlayer1, 100);
		} else if (id == salverid[1]) {
			if (color != 0xFF) {
				if (color == 0) { //red
					maxtriAppScoreIncrease(1);
				} else {
					maxtriAppScoreIncrease(0);
				}
			}

			if (maxtriAppGetScore(1) > 45) {
				perdic2 = 500;
			} else if (maxtriAppGetScore(1) > 30) {
				perdic2 = 1000;
			} else if (maxtriAppGetScore(1) > 15) {
				perdic2 = 1500;
			} else {
				perdic2 = 2000;
			}
			xTimerStop(xTimersPlayer2, 100);
			xTimerChangePeriod(xTimersPlayer2, perdic2, 100);
			xTimerStart(xTimersPlayer2, 100);
		}
	}
	return 0;
}

static void ps_snatch_led_on(void) {
	if (maxtrixAppGetGameMode() == 0) {

	} else {

	}
}

static void xTimer1Handler(void *p) {
//	ps_send_event(PS_EVT_PLATE_EXC, 0);
	can_frame_t msg;
	msg.dataByte0 = PROTOCAL_LED_ON;
	switch (current_status) {
	case PS_SNATCH_LED:
		if (++plate_on_cnt[0] == 100) {
			ps_send_event(PS_STOP_GAME, 0);
			return;
		}
		if (maxtrixAppGetGameMode() == 0) {
			salverid[0] = rand() % PLATE_AMOUNT + 1;
			msg.dataByte2 = rand() % PLATE_AMOUNT;
			msg.dataByte1 = salverid[0];
			msg.dataByte3 = perdic1 / 100;
//			if (++plate_on_cnt[0] == 100) {
//				ps_send_event(PS_STOP_GAME, 0);
//			}
//			CanAppSendMsg(&msg);
//			xTimerChangePeriod(xTimersPlayer1, 4000 /* perdic1 * 2 */+ 50, 100);
//			xTimerStart(xTimersPlayer1, 0);
		} else {

			salverid[0] = rand() % (PLATE_AMOUNT / 2) + 1;

			msg.dataByte2 = rand() % (PLATE_AMOUNT / 2);
			msg.dataByte1 = salverid[0];
			msg.dataByte3 = perdic1 / 100;
//			if (++plate_on_cnt[0] == 100) {
//				ps_send_event(PS_STOP_GAME, 0);
//				return;
//			}
//			xTimerChangePeriod(xTimersPlayer1, 4000 /* perdic1 * 2 */+ 50, 100);
		}
		CanAppSendMsg(&msg);
		xTimerChangePeriod(xTimersPlayer1, 4000 /* perdic1 * 2 */+ 50, 100);
		xTimerStart(xTimersPlayer1, 0);

		break;
	case PS_ROAD_BLOCK:
		break;
	case PS_WIPE_LED:
		break;
	case PS_AGIL_TRAIN:
		break;
	}
}

static void xTimer2Handler(void *p) {
	//	ps_send_event(PS_EVT_PLATE_EXC, 0);
	can_frame_t msg;
	msg.dataByte0 = PROTOCAL_LED_ON;
	switch (current_status) {
	case PS_SNATCH_LED:
		if (++plate_on_cnt[1] == 100) {
			ps_send_event(PS_STOP_GAME, 0);
			return;
		}
		if (maxtrixAppGetGameMode() == 0) {

		} else {
			salverid[1] = rand() % (PLATE_AMOUNT / 2) + 1 + (PLATE_AMOUNT / 2);
			msg.dataByte2 = rand() % (PLATE_AMOUNT / 2);
			msg.dataByte1 = salverid[1];
			msg.dataByte3 = perdic2 / 100;

//			if (++plate_on_cnt[1] == 100) {
//				ps_send_event(PS_STOP_GAME, 0);
//				return;
//			}
			CanAppSendMsg(&msg);
			xTimerChangePeriod(xTimersPlayer2, 4000 /* perdic1 * 2 */+ 50, 100);
			xTimerStart(xTimersPlayer2, 0);
		}

		break;
	case PS_ROAD_BLOCK:
		break;
	case PS_WIPE_LED:
		break;
	case PS_AGIL_TRAIN:
		break;
	}
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
