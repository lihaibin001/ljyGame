/*
 * gm_stt.h
 *
 *  Created on: 2019��3��4��
 *      Author: ecarx
 */

TREE (tree_psync)

STATE (PS_ROOT,				0,				ps_cs_root)
TRANS (ENTRY,				INTERNAL,		ps_entry_root)
TRANS (START,				PS_ROOT,		ps_start_action)
STATE_END

STATE (PS_BOOT_TEST,		PS_ROOT,		ps_cs_boot_test)
TRANS (ENTRY,				INTERNAL,		ps_entry_boot_test)
TRANS (PS_EVT_BOOT, 		PS_IDLE,		no_action)
TRANS (PS_EVT_FAULT, 		PS_FAULT,		no_action)
STATE_END


STATE (PS_IDLE,				PS_ROOT,		ps_cs_idle)
TRANS (ENTRY,				INTERNAL,		ps_entry_idle)
TRANS (PS_EVT_SNATCH_LED,	PS_SNATCH_LED,	no_action)
TRANS (PS_EVT_ROAD_BLOCK,	PS_ROAD_BLOCK,	no_action)
TRANS (PS_EVT_WIPE_LED,	    PS_WIPE_LED,	no_action)
TRANS (PS_EVT_AGIL_TRAIN,	PS_AGIL_TRAIN,	no_action)
STATE_END

STATE (PS_SNATCH_LED,		PS_ROOT,		ps_cs_snatch_led)
TRANS (PS_STOP_GAME,		PS_GAME_STOP,	no_action)
STATE_END

STATE (PS_ROAD_BLOCK,		PS_ROOT,		ps_cs_road_block)
TRANS (PS_STOP_GAME,		PS_GAME_STOP,	no_action)
STATE_END

STATE (PS_WIPE_LED,			PS_ROOT,		ps_cs_wipe_led)
TRANS (PS_STOP_GAME,		PS_GAME_STOP,	no_action)
STATE_END

STATE (PS_AGIL_TRAIN,       PS_ROOT,		ps_cs_agility_training)
TRANS (PS_STOP_GAME,		PS_GAME_STOP,	no_action)
STATE_END

STATE (PS_GAME_STOP,		PS_ROOT,		ps_cs_game_stop)
TRANS (PS_START_GAME,		PS_IDLE,		no_action)
STATE_END

STATE (PS_FAULT,			PS_ROOT,		ps_cs_boot_test)
STATE_END

TREE_END (tree_psync)

