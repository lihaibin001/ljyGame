/*
 * sync.h
 *
 *  Created on: 2019Äê3ÔÂ4ÈÕ
 *      Author: ecarx
 */

#ifndef APP_SYNC_H_
#define APP_SYNC_H_

//#include "fsm.h"

typedef enum
{
	PS_EVT_BOOT,
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
