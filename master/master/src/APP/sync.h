/*
 * sync.h
 *
 *  Created on: 2019��3��4��
 *      Author: ecarx
 */

#ifndef APP_SYNC_H_
#define APP_SYNC_H_

//#include "fsm.h"
#include <stdint.h>
#include <stdio.h>

typedef enum
{
	PS_EVT_TEST,
	PS_EVT_BOOT,
	PS_EVT_FAULT,
	PS_EVT_MODE_CHGE,
	PS_EVT_GAME_OVER,
	PS_EVT_PAGE,
	PS_EVT_SNATCH_LED,
	PS_EVT_ROAD_BLOCK,
	PS_EVT_WIPE_LED,
	PS_EVT_AGIL_TRAIN,
	PS_START_STOP_GAME,
	PS_SWITCH_VOLUME,
	PS_COUNT_DOWN,
	PS_GAME_ACTION,
//	PS_STOP_GAME,
	PS_EVT_SLAVE_EVT,
	PS_EVT_PLATE_EXC,
	PS_HOLD_EVT,
	PS_EVT_AMOUNT,
}PS_EVT_t;

/* the precompiler produces an enum of the states */
#include "fsm_stat.h"
#include "ps_stt.h"

void ps_task_create(void);
uint8_t ps_send_event(uint16_t event, int16_t data);
#endif /* APP_SYNC_H_ */
