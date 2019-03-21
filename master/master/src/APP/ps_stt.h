/*
 * gm_stt.h
 *
 *  Created on: 2019��3��4��
 *      Author: ecarx
 */

TREE (tree_psync)

STATE (PS_ROOT,				0,				cs_no_action)
TRANS (ENTRY,				INTERNAL,		ps_entry_root)
TRANS (START,				PS_ROOT,		ps_start_action)
STATE_END

STATE (PS_BOOT_TEST,		PS_ROOT,		ps_cs_boot_test)
TRANS (ENTRY,				INTERNAL,		ps_entry_boot_test)
TRANS (PS_EVT_TEST,			INTERNAL,		ps_check_selftest)
TRANS (PS_EVT_BOOT, 		PS_IDLE,		no_action)
TRANS (PS_EVT_FAULT, 		PS_FAULT,		no_action)
STATE_END


STATE (PS_IDLE,				PS_ROOT,		cs_no_action)
TRANS (ENTRY,				INTERNAL,		ps_entry_idle)
TRANS (PS_EVT_MODE_CHGE,	INTERNAL,		ps_mode_change)
TRANS (PS_EVT_PAGE,			INTERNAL,		ps_page)
TRANS (PS_START_GAME,		CONDITION,		ps_start_game)
TRANS (PS_START_GAME,		PS_SNATCH_LED,	no_action)
TRANS (PS_START_GAME,		PS_ROAD_BLOCK,	no_action)
TRANS (PS_START_GAME,	    PS_WIPE_LED,	no_action)
TRANS (PS_START_GAME,		PS_AGIL_TRAIN,	no_action)
STATE_END

STATE (PS_SNATCH_LED,		PS_ROOT,		ps_cs_snatch_led)
TRANS (ENTRY,				INTERNAL,		ps_entry_snatch_led)
TRANS (PS_EVT_PLATE_EXC,	INTERNAL,		ps_snatch_handler)
//TRANS (PS_EVT_PRE_START,	INTERNAL,		ps_pre_start)
TRANS (PS_STOP_GAME,		PS_IDLE,		no_action)
STATE_END

STATE (PS_ROAD_BLOCK,		PS_ROOT,		ps_cs_road_block)
TRANS (ENTRY,				INTERNAL,		ps_entry_road_block)
//TRANS (PS_EVT_PRE_START,	INTERNAL,		ps_pre_start)
TRANS (PS_STOP_GAME,		PS_IDLE,		no_action)
STATE_END

STATE (PS_WIPE_LED,			PS_ROOT,		ps_cs_wipe_led)
TRANS (ENTRY,				INTERNAL,		ps_entry_wipe_led)
//TRANS (PS_EVT_PRE_START,	INTERNAL,		ps_pre_start)
TRANS (PS_STOP_GAME,		PS_IDLE,		no_action)
STATE_END

STATE (PS_AGIL_TRAIN,		PS_ROOT,		ps_cs_agility_training)
TRANS (ENTRY,				INTERNAL,		ps_entry_agil_train)
//TRANS (PS_EVT_PRE_START,	INTERNAL,		ps_pre_start)
TRANS (PS_STOP_GAME,		PS_IDLE,		no_action)
STATE_END

STATE (PS_FAULT,			PS_ROOT,		ps_cs_fault)
TRANS (ENTRY,				INTERNAL,		ps_entery_fault)
STATE_END

TREE_END (tree_psync)

