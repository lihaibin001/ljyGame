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
#include "mp3.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include <string.h>
#include <rand.h>

typedef uint8_t PS_Current_State_T;
typedef uint8_t PS_Flags_T;

/* macro */
#define STOP_CNT 100

/* variable */
static TimerHandle_t xTimersPlayer[2];
static uint8_t plate_status;
static TimerHandle_t xTimers;
static uint8_t plate_on_cnt[2];
static uint8_t salverid[2];
static uint8_t current_status;
static QueueHandle_t xQueue;
static uint8_t play_mode;
const static uint8_t *play_mode_tbl[] = { snatch_led, road_block, wipe_led,
		agil_train, };

/* function declaration */
static void xTask(void *p);
static uint8_t ps_start_action(uint16_t data);
static uint8_t no_action(uint16_t data);
static uint8_t cs_no_action(void);
static uint8_t ps_check_selftest(uint16_t data);
static uint8_t ps_mode_change(uint16_t data);
static uint8_t ps_start_game(uint16_t data);
static uint8_t ps_page(uint16_t data);
static uint8_t ps_count_down(uint16_t data);
static uint8_t ps_game_action(uint16_t data);
static uint8_t ps_game_over_handle(uint16_t data);
static uint8_t ps_stop_game(uint16_t data);
static uint8_t ps_snatch_handle_slave_evt(uint16_t data);
static uint8_t ps_snatch_handler(uint16_t data);
static uint8_t ps_road_block_handle_slave_evt(uint16_t data);
static uint8_t ps_road_block_handler(uint16_t data);
static uint8_t ps_wipe_led_handle_slave_evt(uint16_t data);
static uint8_t ps_wipe_led_handler(uint16_t data);
static uint8_t ps_agil_traning_handle_slave_evt(uint16_t data);
static uint8_t ps_agil_traning_handler(uint16_t data);
static uint8_t ps_entry_idle(uint16_t data);
static uint8_t ps_entery_running(uint16_t data);
static uint8_t ps_entery_fault(uint16_t data);
static uint8_t ps_entry_snatch_led(uint16_t data);
static uint8_t ps_entry_road_block(uint16_t data);
static uint8_t ps_entry_wipe_led(uint16_t data);
static uint8_t ps_entry_agil_train(uint16_t data);
static uint8_t ps_entry_root(uint16_t data);
static uint8_t ps_entry_boot_test(uint16_t data);
static uint8_t ps_cs_boot_test(void);
static uint8_t ps_cs_snatch_led(void);
static uint8_t ps_cs_road_block(void);
static uint8_t ps_cs_wipe_led(void);
static uint8_t ps_cs_agility_training(void);
static uint8_t ps_cs_fault(void);
static void xTimer1Handler(void *p);
static void xTimer2Handler(void *p);
static void xTimerFaultHoldTimeOUt(void *p);
static void xTimerCountdown(void *p);
static void xTimerGameoverHold(void *p);
static void xTImerGameover(void *p);
static uint32_t ps_get_game_perdic(uint8_t player);

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

#define FATAL_HANDLER() while(1)

enum {
	PLAY_MODE_UNKNOW,
	SNATCH_LED,
	ROAD_BLOCK,
	WIPE_LED,
	AGIL_TRAIN,
	PLAY_MODE_AMOUNT,
};

void ps_task_create(void) {
	xQueue = xQueueCreate(10, sizeof(Data_Message_T));
	if (xQueue == NULL) {
		FATAL_HANDLER()
			;
	}
	xTimersPlayer[0] = xTimerCreate("player1timer", 0, pdTRUE, (void*) NULL,
			xTimer1Handler);
	if (xTimersPlayer[0] == NULL) {
		FATAL_HANDLER()
			;
	}
	xTimersPlayer[1] = xTimerCreate("player2timer", 0, pdTRUE, (void*) NULL,
			xTimer2Handler);
	if (xTimersPlayer[1] == NULL) {
		FATAL_HANDLER()
			;
	}
	xTaskCreate(xTask, "sync", 256, NULL, 3, NULL);
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

	if (RGBTakeLock()) {
		RGBClearBuff();
		if (play_mode == PLAY_MODE_UNKNOW || play_mode == PLAY_MODE_AMOUNT) {
			RGBrawString(6, 12, 0x0000FF, "WAN DE V1.1");
		} else {
			RGBShowImage(0xFF, play_mode_tbl[play_mode - 1]);
		}
		RGBReleaseLock();
	}
	if (xTimers != NULL) {
		xTimerStop(xTimers, 100);
		xTimerDelete(xTimers, 100);
		xTimers = NULL;
	}
	xTimerStop(xTimersPlayer[0], 100);
	xTimerStop(xTimersPlayer[1], 100);
	maxtriAppStopTime();
	return PS_IDLE;
}

static uint8_t ps_entery_running(uint16_t data) {
	mp3_send_command(0x08, 0x0002);
	ps_send_event(PS_COUNT_DOWN, 0);
	return PS_RUNNING;
}

static uint8_t ps_entery_fault(uint16_t data) {
	char drawBuff[16] = "";
	uint8_t status;
	memset(drawBuff, 0, 4);
	if (maxtrixAppGetPlateStatue(&status)) {
		sprintf(drawBuff, "ERROR: %X !!!", status);
	}
	if (RGBTakeLock()) {
		RGBClearBuff();
		RGBrawString(4, 12, 0x0000FF, drawBuff);
		RGBReleaseLock();
	}
	if (xTimers != NULL) {
		xTimerStop(xTimers, 100);
		xTimerDelete(xTimers, 100);
		xTimers = NULL;
	}
	xTimers = xTimerCreate("SYNC timer", 3000, pdFALSE, (void*) NULL,
			xTimerFaultHoldTimeOUt);
	xTimerStart(xTimers, 100);
	return PS_FAULT;
}

/*********************************************************************/
/* event handler                                                     */
/*********************************************************************/
static uint8_t ps_entry_snatch_led(uint16_t data) {
//	ps_send_event(PS_HOLD_EVT, 0);
	can_frame_t msg;
	msg.dataByte0 = 0;
	if (maxtrixAppGetGameMode() == 0) {
		salverid[0] = rand() % PLATE_AMOUNT + 1;
		msg.dataByte1 = salverid[0];
		msg.dataByte2 = rand() % 5;
		if (msg.dataByte2 != 0) {
			msg.dataByte2 = 1;
		}
		msg.dataByte3 = (uint8_t) (ps_get_game_perdic(0) / 100);
		CanAppSendMsg(&msg);
		xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0) * 2, 100);
		xTimerStart(xTimersPlayer[0], 100);

	} else {

		salverid[0] = rand() % (PLATE_AMOUNT / 2) + 1;
		msg.dataByte1 = salverid[0];
		msg.dataByte2 = rand() % 5;
		if (msg.dataByte2 != 0) {
			msg.dataByte2 = 1;
		}
		msg.dataByte3 = (uint8_t) (ps_get_game_perdic(data) / 100);
		CanAppSendMsg(&msg);
		xTimerChangePeriod(xTimersPlayer[data], ps_get_game_perdic(0) * 2, 100);
		xTimerStart(xTimersPlayer[0], 100);

		salverid[1] = rand() % (PLATE_AMOUNT / 2) + (PLATE_AMOUNT / 2) + 1;
		msg.dataByte1 = salverid[1];
		msg.dataByte2 = rand() % 5;
		if (msg.dataByte2 == 0) {
			msg.dataByte2 = 1;
		} else {
			msg.dataByte2 = 0;
		}
		msg.dataByte3 = (uint8_t) (ps_get_game_perdic(1) / 100);
		CanAppSendMsg(&msg);
		xTimerChangePeriod(xTimersPlayer[1], ps_get_game_perdic(1) * 2, 100);
		xTimerStart(xTimersPlayer[1], 100);
	}
	return PS_SNATCH_LED;
}

static uint8_t ps_entry_road_block(uint16_t data) {
	can_frame_t msg;
	uint8_t i;
	msg.dataByte0 = 0;
	if (maxtrixAppGetGameMode() == 0) {
		for (i = 0; i < PLATE_AMOUNT; i += 2) {
			msg.dataByte1 = rand() % 2 + i + 1;
			plate_status |= 1 << (msg.dataByte1 - 1);
			msg.dataByte2 = 1;
			msg.dataByte3 = (uint8_t) 0xFF;
//			msg.dataByte3 = (uint8_t) (ps_get_game_perdic(data) / 100);
			CanAppSendMsg(&msg);
		}
//		xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
//		xTimerStart(xTimersPlayer[0], 100);
	} else {
		for (i = 0; i < PLATE_AMOUNT / 2; i += 2) {
			msg.dataByte1 = rand() % 2 + i + 1;
			plate_status |= 1 << (msg.dataByte1 - 1);
			msg.dataByte2 = 1;
			msg.dataByte3 = (uint8_t) 0xFF;
//			msg.dataByte3 = (uint8_t) (ps_get_game_perdic(data) / 100);
			CanAppSendMsg(&msg);
		}
		for(i = PLATE_AMOUNT / 2; i < PLATE_AMOUNT; i += 2) {
			msg.dataByte1 = rand() % 2 + i + 1;
			plate_status |= 1 << (msg.dataByte1 - 1);
			msg.dataByte2 = 1;
			msg.dataByte3 = (uint8_t) 0xFF;
//			msg.dataByte3 = (uint8_t) (ps_get_game_perdic(data) / 100);
			CanAppSendMsg(&msg);
		}
//		xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
//		xTimerStart(xTimersPlayer[0], 100);
	}
	return PS_ROAD_BLOCK;
}

static uint8_t ps_entry_wipe_led(uint16_t data) {
	can_frame_t msg;
	msg.dataByte0 = 0;
	plate_status = 0;
	if (maxtrixAppGetGameMode() == 0) {
		msg.dataByte1 = rand() % PLATE_AMOUNT + 1;
		plate_status |= 1 << (msg.dataByte1 - 1);
		msg.dataByte2 = 1;
		msg.dataByte3 = 0xFF;
		CanAppSendMsg(&msg);
		xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
		xTimerStart(xTimersPlayer[0], 100);

	} else {
		msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + 1;
		plate_status |= 1 << (msg.dataByte1 - 1);
		msg.dataByte2 = 1;
		msg.dataByte3 = 0xFF;
		CanAppSendMsg(&msg);

		msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + (PLATE_AMOUNT / 2) + 1;
		plate_status |= 1 << (msg.dataByte1 - 1);
		msg.dataByte2 = 0;
		msg.dataByte3 = 0xFF;
		CanAppSendMsg(&msg);
		xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
		xTimerStart(xTimersPlayer[0], 100);
		xTimerChangePeriod(xTimersPlayer[1], ps_get_game_perdic(1), 100);
		xTimerStart(xTimersPlayer[1], 100);

	}
	xTimers = xTimerCreate("game over", 90000, pdFALSE, (void*) NULL,
			xTImerGameover);
	xTimerStart(xTimers, 100);
	return PS_WIPE_LED;
}

static uint8_t ps_entry_agil_train(uint16_t data) {
	can_frame_t msg;
	msg.dataByte0 = 0;
	if (maxtrixAppGetGameMode() == 0) {
		msg.dataByte1 = rand() % PLATE_AMOUNT + 1;
		salverid[0] = msg.dataByte1;
		msg.dataByte2 = 1;
		msg.dataByte3 = ps_get_game_perdic(0) / 100;
		CanAppSendMsg(&msg);
	} else {
		msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + 1;
		msg.dataByte2 = 1;
		msg.dataByte3 = ps_get_game_perdic(0) / 100;
		CanAppSendMsg(&msg);
		salverid[0] = msg.dataByte1;
		msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + (PLATE_AMOUNT / 2) + 1;
		msg.dataByte2 = 0;
		msg.dataByte3 = ps_get_game_perdic(1) / 100;
		CanAppSendMsg(&msg);
	}
	xTimers = xTimerCreate("game over", 90000, pdFALSE, (void*) NULL,
			xTImerGameover);
	xTimerStart(xTimers, 100);
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
				ps_send_event(PS_EVT_FAULT, 0);
				//ps_send_event(PS_EVT_BOOT, 0);
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
	maxtrixAppSetGameMode(data);
	return 0;
}

static uint8_t ps_start_game(uint16_t data) {
	can_frame_t msg;
	msg.dataByte0 = PROTOCAL_START_GAME;
	if (maxtrixAppGetGameMode() == 0) {
		msg.dataByte1 = PLATE_AMOUNT_BIT;
		msg.dataByte2 = PLATE_AMOUNT_BIT;
	} else {
		msg.dataByte1 = PLATE_AMOUNT_BIT;
		msg.dataByte2 = PLATE_PLAYER1_BIT;
	}
	plate_on_cnt[0] = 0;
	plate_on_cnt[1] = 0;
	CanAppSendMsg(&msg);
	maxtrixAppGameStart();
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
	if (RGBTakeLock()) {
		RGBClearBuff();
		RGBShowImage(0xFF, play_mode_tbl[play_mode - 1]);
		RGBReleaseLock();
	}
	return 0;
}

static uint8_t ps_count_down(uint16_t data) {
	static uint8_t count_down = 3;
	if (count_down == 0) {
		count_down = 3;
		ps_send_event(PS_GAME_ACTION, 0);

	} else {
		if (RGBTakeLock()) {
			RGBClearBuff();
			RGBdrawImage(28, 10, 0xFF, pNumber[count_down]);
			RGBReleaseLock();
		}
		count_down--;
		xTimers = xTimerCreate("count down", 1000, pdTRUE, (void*) NULL,
				xTimerCountdown);
		xTimerStart(xTimers, 100);
	}
	return 0;
}

static uint8_t ps_game_action(uint16_t data) {
	if (play_mode == PLAY_MODE_UNKNOW) {
		play_mode = SNATCH_LED;
	}
	maxtriAppStartTime();
	maxtrixAppGameStart();
	return play_mode;
}

static uint8_t ps_game_over_handle(uint16_t data) {
	can_frame_t msg;
	msg.dataByte0 = PROTOCAL_GAME_OVER;
	if (maxtrixAppGetGameMode() == 0) {
		msg.dataByte1 = PLATE_AMOUNT_BIT;
		msg.dataByte2 = 1;
	} else {
		if (maxtriAppGetScore(0) == maxtriAppGetScore(1)) {
			msg.dataByte1 = PLATE_AMOUNT_BIT;
			msg.dataByte2 = 3;
		} else if (maxtriAppGetScore(0) > maxtriAppGetScore(1)) {
			msg.dataByte1 = PLATE_PLAYER1_BIT;
			msg.dataByte2 = 1;
		} else {
			msg.dataByte1 = PLATE_PLAYER2_BIT;
			msg.dataByte2 = 2;
		}
	}
	CanAppSendMsg(&msg);
	if (xTimers != NULL) {
		xTimerStop(xTimers, 100);
		xTimerDelete(xTimers, 100);
		xTimers = NULL;
	}
	xTimers = xTimerCreate("game over", 5000, pdFALSE, (void*) NULL,
			xTimerGameoverHold);
	if (xTimers == NULL) {
		while (1)
			;
	}
	xTimerStart(xTimers, 100);
	maxtriAppStopTime();
	xTimerStop(xTimersPlayer[0], 100);
	xTimerStop(xTimersPlayer[1], 100);
	return 0;
}

static uint8_t ps_stop_game(uint16_t data) {
	mp3_send_command(0x08, 0x0001);
	return 0;
}

static uint32_t ps_get_game_perdic(uint8_t player) {
	switch (play_mode) {
	case SNATCH_LED:
	case WIPE_LED:
		if (maxtriAppGetScore(player) > 45) {
			return 500;
		} else if (maxtriAppGetScore(player) > 30) {
			return 1000;
		} else if (maxtriAppGetScore(player) > 15) {
			return 1500;
		} else {
			return 2000;
		}
	case AGIL_TRAIN:
	case ROAD_BLOCK:
		if (maxtriAppGetTime() > 60) {
			return 1500;
		} else if (maxtriAppGetTime() > 30) {
			return 2300;
		} else {
			return 3000;
		}
	default:
		return 0;
	}
}

static uint8_t ps_snatch_handle_slave_evt(uint16_t data) {
	uint8_t id = (uint8_t) (data >> 8);
	uint8_t event = (uint8_t) data;
	if (event == 0xFF) {
		return 0;
	}
	if (maxtrixAppGetGameMode() == 0) { //single mode
		if (id == salverid[0]) {
			if (event == 0) { //red
				maxtriAppScoreIncrease(0, 0x0101);
//				mp3_send_command(0x25, 0x0101);
			} else {
				mp3_send_command(0x25, 0x0102);
				maxtriAppScoreDecrease(0, 0x0102);
			}
		}
		xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
		xTimerReset(xTimersPlayer[0], 100);
	} else { //double mode
		if (id == salverid[0]) {
			if (event == 0) {
//				mp3_send_command(0x25, 0x0101);
				maxtriAppScoreIncrease(0, 0x0101);
			} else {
				mp3_send_command(0x25, 0x0102);
				maxtriAppScoreIncrease(1, 0x0102);
			}
			xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
			xTimerReset(xTimersPlayer[0], 100);
		} else if (id == salverid[1]) {
			if (event == 0) { //red
				maxtriAppScoreIncrease(0, 0x0102);
			} else {
//				mp3_send_command(0x25, 0x0101);
				maxtriAppScoreIncrease(1, 0x0101);
			}
			xTimerChangePeriod(xTimersPlayer[1], ps_get_game_perdic(1), 100);
			xTimerReset(xTimersPlayer[1], 100);
		}
	}
	if (plate_on_cnt[0] == STOP_CNT || plate_on_cnt[1] == STOP_CNT) {
		ps_send_event(PS_EVT_GAME_OVER, 2);
	}
	return 0;
}

static uint8_t ps_snatch_handler(uint16_t data) {
	can_frame_t msg;
	plate_on_cnt[data]++;
	msg.dataByte0 = PROTOCAL_LED_ON;
	if (plate_on_cnt[0] == STOP_CNT || plate_on_cnt[1] == STOP_CNT) {
		ps_send_event(PS_EVT_GAME_OVER, 2);
		return 0;
	}
	srand(xTaskGetTickCount());
	if (maxtrixAppGetGameMode() == 0) {
		salverid[0] = rand() % PLATE_AMOUNT + 1;
		msg.dataByte1 = salverid[0];
		msg.dataByte2 = rand() % 5;
		if (msg.dataByte2 != 0) {
			msg.dataByte2 = 1;
		}
		msg.dataByte3 = (uint8_t) (ps_get_game_perdic(0) / 100);
		CanAppSendMsg(&msg);

	} else {
		if (data == 0) {
			salverid[0] = rand() % (PLATE_AMOUNT / 2) + 1;
			msg.dataByte1 = salverid[0];
			msg.dataByte2 = rand() % 5;
		} else {
			salverid[1] = rand() % (PLATE_AMOUNT / 2) + (PLATE_AMOUNT / 2) + 1;
			msg.dataByte1 = salverid[1];
			msg.dataByte2 = rand() % 5;
			if (msg.dataByte2 == 0) {
				msg.dataByte2 = 1;
			} else {
				msg.dataByte2 = 0;
			}
		}

		msg.dataByte3 = (uint8_t) (ps_get_game_perdic(data) / 100);
		CanAppSendMsg(&msg);
	}
	xTimerChangePeriod(xTimersPlayer[data], ps_get_game_perdic(data) * 2, 100);
	xTimerReset(xTimersPlayer[data], 100);
	return 0;
}

static uint8_t ps_road_block_handle_slave_evt(uint16_t data) {
	can_frame_t msg;
	uint8_t id = (uint8_t) (data >> 8);
//	uint8_t event = (uint8_t) data;
	uint8_t i;
//	if (event == 0xFF) {
//		if(id == 0) {
//			vTaskDelay(100);
//			plate_status = 0;
//			for (i = 0; i < PLATE_AMOUNT; i += 2) {
//				msg.dataByte1 = rand() % 2 + i + 1;
//				plate_status |= 1 << (msg.dataByte1 - 1);
//				msg.dataByte2 = 1;
////				msg.dataByte3 = (uint8_t) 0xFF;
//				msg.dataByte3 = (uint8_t) (ps_get_game_perdic(0) / 100);
////				vTaskDelay(10);
//				CanAppSendMsg(&msg);
//			}
//			xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
//			xTimerReset(xTimersPlayer[0], 100);
//		}
//	}
//	if(plate_status == 0) {
//		return 0;
//	}
	msg.dataByte0 = 0;
	if (maxtrixAppGetGameMode() == 0) {
		if ((1 << (id - 1)) & plate_status) {
			plate_status &= ~(1 << (id - 1));
		}
		if(plate_status == 0) {
			maxtriAppScoreIncrease(0, 0x0101);
//			vTaskDelay(10);
			for (i = 0; i < PLATE_AMOUNT; i += 2) {
				msg.dataByte1 = rand() % 2 + i + 1;
				plate_status |= 1 << (msg.dataByte1 - 1);
				msg.dataByte2 = 1;
				msg.dataByte3 = (uint8_t) 0xFF;
//				msg.dataByte3 = (uint8_t) (ps_get_game_perdic(0) / 100);
				CanAppSendMsg(&msg);
			}
			xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
			xTimerReset(xTimersPlayer[0], 100);
		}
	} else {
		if ((1 << (id - 1)) & plate_status) {
			plate_status &= ~(1 << (id - 1));
		}
		if (id <= PLATE_AMOUNT / 2) {
			if((plate_status & PLATE_PLAYER1_BIT) == 0) {
				maxtriAppScoreIncrease(0, 0x0101);
//				vTaskDelay(10);
				for (i = 0; i < PLATE_AMOUNT / 2; i += 2) {
					msg.dataByte1 = rand() % 2 + i + 1;
					plate_status |= 1 << (msg.dataByte1 - 1);
					msg.dataByte2 = 1;
					msg.dataByte3 = (uint8_t) 0xFF;
//					msg.dataByte3 = (uint8_t) (ps_get_game_perdic(0) / 100);
					CanAppSendMsg(&msg);
				}
				xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
				xTimerReset(xTimersPlayer[0], 100);
			}
		} else {
			if((plate_status & PLATE_PLAYER2_BIT) == 0) {
				maxtriAppScoreIncrease(0, 0x0101);
				vTaskDelay(10);
				for (i = PLATE_AMOUNT / 2; i < PLATE_AMOUNT; i += 2) {
					msg.dataByte1 = rand() % 2 + i + 1;
					plate_status |= 1 << (msg.dataByte1 - 1);
					msg.dataByte2 = 1;
					msg.dataByte3 = (uint8_t) 0xFF;
//					msg.dataByte3 = (uint8_t) (ps_get_game_perdic(0) / 100);
					CanAppSendMsg(&msg);
				}
				xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
				xTimerReset(xTimersPlayer[0], 100);
			}
		}
	}
	return 0;
}

static uint8_t ps_road_block_handler(uint16_t data) {
	ps_send_event(PS_EVT_SLAVE_EVT, data | 0xFF);
	return 0;
}

static uint8_t ps_wipe_led_handle_slave_evt(uint16_t data) {
	uint8_t id = (uint8_t) (data >> 8);
	uint8_t event = (uint8_t) data;
	if (event == 0xFF) {
		return 0;
	}
	if (maxtrixAppGetGameMode() == 0) { //single mode
		if ((1 << (id - 1)) & plate_status) {
			plate_status &= ~(1 << (id - 1));
			maxtriAppScoreIncrease(0, 0x0101);
		}
	} else { //double mode
		if ((1 << (id - 1)) & plate_status) {
			plate_status &= ~(1 << (id - 1));
		}
		if (id <= PLATE_AMOUNT / 2) {
			maxtriAppScoreIncrease(0, 0x0101);
		} else {
			maxtriAppScoreIncrease(1, 0x0101);
		}
	}
	return 0;
}

static uint8_t ps_wipe_led_handler(uint16_t data) {
	can_frame_t msg;
	msg.dataByte0 = 0;
	if (maxtrixAppGetGameMode() == 0) {
		do {
			msg.dataByte1 = rand() % PLATE_AMOUNT + 1;
		} while ((1 << (msg.dataByte1 - 1)) & plate_status);
		plate_status |= 1 << (msg.dataByte1 - 1);
		if (plate_status == PLATE_AMOUNT_BIT) {
			ps_send_event(PS_EVT_GAME_OVER, 0);
			return 0;
		}
		msg.dataByte2 = msg.dataByte1;
		msg.dataByte3 = 0xFF;
		CanAppSendMsg(&msg);
		xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
		xTimerReset(xTimersPlayer[0], 100);

	} else {
		if (data == 0) {
			do {
				msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + 1;
			} while ((1 << (msg.dataByte1 - 1)) & plate_status);
			plate_status |= 1 << (msg.dataByte1 - 1);
			if ((plate_status & PLATE_PLAYER1_BIT) == PLATE_PLAYER1_BIT) {
				ps_send_event(PS_EVT_GAME_OVER, 0);
				return 0;
			}
			msg.dataByte2 = 1;
			msg.dataByte3 = 0xFF;
			CanAppSendMsg(&msg);
			xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0), 100);
			xTimerReset(xTimersPlayer[0], 100);
		} else {
			do {
				msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + (PLATE_AMOUNT / 2)
						+ 1;
			} while ((1 << (msg.dataByte1 - 1)) & plate_status);
			plate_status |= 1 << (msg.dataByte1 - 1);
			if ((plate_status & PLATE_PLAYER2_BIT) == PLATE_PLAYER2_BIT) {
				ps_send_event(PS_EVT_GAME_OVER, 0);
				return 0;
			}
			msg.dataByte2 = 0;
			msg.dataByte3 = 0xFF;
			CanAppSendMsg(&msg);
			xTimerChangePeriod(xTimersPlayer[1], ps_get_game_perdic(1), 100);
			xTimerReset(xTimersPlayer[1], 100);
		}
	}
	return 0;
}

static uint8_t ps_agil_traning_handle_slave_evt(uint16_t data) {
	uint8_t id = (uint8_t) (data >> 8);
	can_frame_t msg;
	msg.dataByte0 = 0;
	if (maxtrixAppGetGameMode() == 0) { //single mode
		if (id == salverid[0]) {

			maxtriAppScoreIncrease(0, 0x0101);
			msg.dataByte1 = rand() % PLATE_AMOUNT + 1;
			salverid[0] = msg.dataByte1;
			msg.dataByte2 = 1;
			msg.dataByte3 = ps_get_game_perdic(0) / 100;
			CanAppSendMsg(&msg);
		}
	} else { //double mode
		if (id == salverid[0]) {
			maxtriAppScoreIncrease(0, 0x0101);
			msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + 1;
			msg.dataByte2 = 1;
			msg.dataByte3 = ps_get_game_perdic(0) / 100;
			CanAppSendMsg(&msg);
			salverid[0] = msg.dataByte1;
		}
		if (id == salverid[1]) {
			maxtriAppScoreIncrease(1, 0x0101);
			msg.dataByte1 = rand() % (PLATE_AMOUNT / 2) + (PLATE_AMOUNT / 2)
					+ 1;
			msg.dataByte2 = 0;
			msg.dataByte3 = ps_get_game_perdic(0) / 100;
			CanAppSendMsg(&msg);
			salverid[1] = msg.dataByte1;
		}
	}
	return 0;
}

static uint8_t ps_agil_traning_handler(uint16_t data) {
	return 0;
}

static void xTimer1Handler(void *p) {
	ps_send_event(PS_EVT_PLATE_EXC, 0);
	xTimerChangePeriod(xTimersPlayer[0], ps_get_game_perdic(0) * 2, 100);
//	if (plate_on_cnt[0] == STOP_CNT) {
//		ps_send_event(PS_EVT_GAME_OVER, 2);
//	}
}

static void xTimer2Handler(void *p) {
	ps_send_event(PS_EVT_PLATE_EXC, 1);
	xTimerChangePeriod(xTimersPlayer[1], ps_get_game_perdic(1) * 2, 100);
//	if (plate_on_cnt[1] == STOP_CNT) {
//		ps_send_event(PS_EVT_GAME_OVER, 2);
//	}
}

static void xTimerFaultHoldTimeOUt(void *p) {
	ps_send_event(PS_HOLD_EVT, 0);
	if (xTimers != NULL) {
		xTimerDelete(xTimers, 100);
		xTimers = NULL;
	}
}

static void xTimerCountdown(void *p) {
	ps_send_event(PS_COUNT_DOWN, 0);
	xTimerDelete(xTimers, 100);
	xTimers = NULL;
}

static void xTimerGameoverHold(void *p) {
	ps_send_event(PS_START_STOP_GAME, 0);
	xTimerDelete(xTimers, 100);
	xTimers = NULL;

}

static void xTImerGameover(void *p) {
	ps_send_event(PS_EVT_GAME_OVER, 0);
	xTimerDelete(xTimers, 100);
	xTimers = NULL;
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
