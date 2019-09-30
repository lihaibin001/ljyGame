
TREE (tree_psync)

STATE (PS_ROOT,					0,				cs_no_action)
TRANS (ENTRY,					INTERNAL,		ps_entry_root)
TRANS (PS_EVT_START,			PS_BOOT_TEST,	no_action)
TRANS (PS_EVT_POWERDOWN,		PS_STOP,		no_action)
TRANS (START,					PS_ROOT,		ps_start_action)
STATE_END

STATE (PS_BOOT_TEST,			PS_ROOT,		ps_cs_boot_test)
TRANS (ENTRY,					INTERNAL,		ps_entry_boot_test)
TRANS (PS_EVT_TEST,				INTERNAL,		ps_check_selftest)
TRANS (PS_EVT_BOOT, 			PS_IDLE,		no_action)
TRANS (PS_EVT_FAULT, 			PS_FAULT,		no_action)
STATE_END

STATE (PS_IDLE,					PS_ROOT,		cs_no_action)
TRANS (ENTRY,					INTERNAL,		ps_entry_idle)
TRANS (PS_EVT_PAGE,				INTERNAL,		ps_page)
TRANS (PS_EVT_MODE_CHGE,		INTERNAL,		ps_mode_change)
TRANS (PS_EVT_START_STOP_GAME,	PS_RUNNING,		ps_start_game)
TRANS (PS_EVT_SWITCH_VOLUME, 	PS_VOLUME,		no_action)
TRANS (PS_EVT_DISPLAYOFF,  		PS_DISPLAYOFF,	no_action)
STATE_END

STATE (PS_VOLUME,				PS_ROOT,		cs_no_action)
TRANS (ENTRY,					INTERNAL,		ps_entry_volume)
TRANS (PS_EVT_PAGE,				INTERNAL,		ps_adjust_volume)
TRANS (PS_EVT_START_STOP_GAME,	PS_IDLE,		no_action)
TRANS (EXIT,					INTERNAL,		ps_exit_volume)
STATE_END

STATE (PS_RUNNING,				PS_ROOT,		ps_cs_running)
TRANS (ENTRY,					INTERNAL,		ps_entery_running)
TRANS (PS_EVT_COUNT_DOWN,		INTERNAL,		ps_count_down)
TRANS (PS_EVT_GAME_ACTION,		CONDITION,		ps_game_action)
TRANS (PS_EVT_GAME_ACTION,		PS_SNATCH_LED,	no_action)
TRANS (PS_EVT_GAME_ACTION,		PS_ROAD_BLOCK,	no_action)
TRANS (PS_EVT_GAME_ACTION,		PS_WIPE_LED,	no_action)
TRANS (PS_EVT_GAME_ACTION,		PS_AGIL_TRAIN,	no_action)
TRANS (PS_EVT_GAME_ACTION,		INTERNAL,		no_action)
TRANS (PS_EVT_GAME_OVER,		INTERNAL,		ps_game_over_handle)
TRANS (PS_EVT_START_STOP_GAME,  PS_IDLE,		ps_stop_game)

STATE_END

STATE (PS_SNATCH_LED,			PS_RUNNING,		ps_cs_snatch_led)
TRANS (ENTRY,					INTERNAL,		ps_entry_snatch_led)
TRANS (PS_EVT_SLAVE_EVT,		INTERNAL,		ps_snatch_handle_slave_evt)
TRANS (PS_EVT_PLATE_EXC,		INTERNAL,		ps_snatch_handler)
STATE_END

STATE (PS_ROAD_BLOCK,			PS_RUNNING,		ps_cs_road_block)
TRANS (ENTRY,					INTERNAL,		ps_entry_road_block)
TRANS (PS_EVT_SLAVE_EVT,		INTERNAL,		ps_road_block_handle_slave_evt)
TRANS (PS_EVT_PLATE_EXC,		INTERNAL,		ps_road_block_handler)
STATE_END

STATE (PS_WIPE_LED,				PS_RUNNING,		ps_cs_wipe_led)
TRANS (ENTRY,					INTERNAL,		ps_entry_wipe_led)
TRANS (PS_EVT_SLAVE_EVT,		INTERNAL,		ps_wipe_led_handle_slave_evt)
TRANS (PS_EVT_PLATE_EXC,		INTERNAL,		ps_wipe_led_handler)
STATE_END

STATE (PS_AGIL_TRAIN,			PS_RUNNING,		ps_cs_agility_training)
TRANS (ENTRY,					INTERNAL,		ps_entry_agil_train)
TRANS (PS_EVT_SLAVE_EVT,		INTERNAL,		ps_agil_traning_handle_slave_evt)
TRANS (PS_EVT_PLATE_EXC,		INTERNAL,		ps_agil_traning_handler)
STATE_END

STATE (PS_FAULT,				PS_RUNNING,		ps_cs_fault)
TRANS (ENTRY,					INTERNAL,		ps_entery_fault)
TRANS (PS_HOLD_EVT,				PS_IDLE,		no_action)
STATE_END

STATE (PS_DISPLAYOFF,			PS_ROOT,		cs_no_action)
TRANS (ENTRY,					INTERNAL,		ps_entery_displayoff)
TRANS (PS_EVT_MODE_CHGE,		PS_IDLE,		no_action)
TRANS (PS_EVT_PAGE,				PS_IDLE,		no_action)
TRANS (PS_EVT_START_STOP_GAME, 	PS_IDLE,		no_action)
TRANS (PS_EVT_MODE_CHGE,		PS_IDLE,		no_action)
TRANS (PS_EVT_STOP,  			PS_STOP,		no_action)
STATE_END

STATE (PS_STOP,					PS_ROOT,		cs_goto_stop)
TRANS (ENTRY,					INTERNAL,		ps_entery_stop)
TRANS (PS_EVT_WAKEUP,			PS_ROOT,		no_action)
TRANS (EXIT,					INTERNAL,		ps_exit_stop)
STATE_END

TREE_END (tree_psync)
