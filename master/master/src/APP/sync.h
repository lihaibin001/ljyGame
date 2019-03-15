/*
 * sync.h
 *
 *  Created on: 2019��3��4��
 *      Author: ecarx
 */

#ifndef APP_SYNC_H_
#define APP_SYNC_H_

//#include "fsm.h"

#define PROTOCAL_LED_ON			(uint8_t)0
#define PROTOCAL_LED_OFF		(uint8_t)1
#define PROTOCAL_SELF_TESET		(uint8_t)2

typedef enum
{
	PS_EVT_TEST,
	PS_EVT_BOOT,
	PS_EVT_FAULT,
	PS_EVT_SNATCH_LED,
	PS_EVT_ROAD_BLOCK,
	PS_EVT_WIPE_LED,
	PS_EVT_AGIL_TRAIN,
	PS_START_GAME,
	PS_STOP_GAME,
	PS_EVT_AMOUNT,
}PS_EVT_t;

/* the precompiler produces an enum of the states */
#include "fsm_stat.h"
#include "ps_stt.h"

void ps_task_create(void);

#endif /* APP_SYNC_H_ */
