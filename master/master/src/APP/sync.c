/*
 * sync.c
 *
 *  Created on: 2019Äê3ÔÂ4ÈÕ
 *      Author: ecarx
 */

#include "sync.h"
#include <stdint.h>
#include <stdio.h>
#include "fsm.h"

typedef uint8_t PS_Current_State_T;
typedef uint8_t PS_Flags_T;

static uint8_t ps_start_action(void);
static uint8_t no_action(void);

static uint8_t ps_entry_idle(void);
static uint8_t ps_entry_root (void);

static uint8_t ps_cs_root(void);
static uint8_t ps_cs_idle(void);
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
void Psync_Task(void *pvParameters)
{
   Data_Message_T msg;
   uint8_t status;
   PS_Current_State_T new_state;
   bool state_changed;

   for(;;)
   {

      state_changed = false;

      // wait until next event in mailbox OR time expired
      //status = OS_Wait_Message(OS_PSYNC_TASK, &msg.all, PS_TASK_REQUEUE_TIME);

      // process ALL events in queue first !!!
      while (0 == status)
      {
         // process event
         new_state = FSM_Process_Evt(msg, current_status, tree_psync);

         if (new_state != current_status)
         {
        	 current_status = new_state;
        	 state_changed = true;
         }

         // get next event from mailbox without waiting
         //status = OS_Receive_Message(OS_PSYNC_TASK, &msg.all);
      }

      // process all cs routines for the active state tree branch
      FSM_Process_CS(current_status, tree_psync);

      if (state_changed)
      {
         //OS_Release_Resource(RES_RELAYS);
      }
   }
}

/*********************************************************************/
/* entry actions                                                     */
/*********************************************************************/
static uint8_t ps_entry_root (void)
{
	return 0;
}

static uint8_t ps_entry_idle(void)
{
	return 0;
}

/*********************************************************************/
/* cs routines                                                       */
/*********************************************************************/
static uint8_t ps_cs_root(void)
{
    return 0;
}

static uint8_t ps_cs_idle(void)
{

    return 0;
}

static uint8_t ps_cs_snatch_led(void)
{
	return 0;
}

static uint8_t ps_cs_road_block(void)
{
	return 0;
}

static uint8_t ps_cs_wipe_led(void)
{

}

static uint8_t ps_cs_agility_training(void)
{

}

static uint8_t ps_cs_game_stop(void)
{

}

/*********************************************************************/
/* exit actions                                                      */
/*********************************************************************/

/*********************************************************************/
/* start actions                                                     */
/*********************************************************************/
static uint8_t ps_start_action(void)
{
	return 0;
}

/*********************************************************************/
/*no actions                                                     */
/*********************************************************************/
static uint8_t no_action(void)
{
   return (0);
}
