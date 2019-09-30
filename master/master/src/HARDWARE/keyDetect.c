#include "keyDetect.h"
#include "sync.h"


#define TIME_PERIDIC 20	//scan the pin's status every TIME_PERIDIC ms
#define DEBOUNCE_CNT 2  //debounce count, debounce time = TIME_PERIDIC * DEBOUNCE_CNT


typedef void (*pfKeyPressHandler)(void);
typedef void (*pfKeyConsecutivePressHandler)(void);

typedef struct {
	GPIO_TypeDef *pGPO;
	uint16_t pin;
} Key_t;

typedef struct {
	Key_t const *pKey;
	const pfKeyPressHandler perssHandler;
	const pfKeyConsecutivePressHandler consetcutivePressHandler;
	const uint16_t consecutiveTimeout;
	uint16_t consecutiveTime;
	uint8_t status;
	uint8_t time;
	uint8_t decreaseTime;
} keyStatus_t;

static const Key_t key[keyIdxNum] = {
		{ GPIOB, GPIO_Pin_12 },
		{ GPIOB, GPIO_Pin_13 },
		{ GPIOB, GPIO_Pin_14 },
		{ GPIOB, GPIO_Pin_15 },
		{ GPIOD, GPIO_Pin_8 },
		{ GPIOD, GPIO_Pin_9 },
		{ GPIOD, GPIO_Pin_10 },
};

static void keyStatusDetect(void);
static void vTimerCallback(TimerHandle_t xTimer);
static void singleModeKeyHander(void);
static void mutleModeKeyHandler(void);
static void pageUpKeyHander(void);
static void pageDownKeyHandler(void);
static void startAndStopKeyHandler(void);
static void singleModeConsecutivePress(void);
static void mutleModeConsecutivePress(void);
static void pageUpConsecutivePress(void);
static void pageDownConsecutivePress(void);

static keyStatus_t keyStatus[keyIdxNum] = {
		{ &key[keyIdx_1], singleModeKeyHander, 		singleModeConsecutivePress, 3000,	1, 0, 0 },
		{ &key[keyIdx_2], mutleModeKeyHandler, 		mutleModeConsecutivePress, 	3000,	1, 0, 0 },
		{ &key[keyIdx_3], pageUpKeyHander, 			pageUpConsecutivePress, 	5000,	1, 0, 0 },
		{ &key[keyIdx_4], pageDownKeyHandler, 		pageDownConsecutivePress, 	5000,	1, 0, 0 },
		{ &key[keyIdx_5], startAndStopKeyHandler,	NULL, 						0, 		1, 0, 0 },
		{ &key[keyIdx_6], NULL, 					NULL, 						0, 		1, 0, 0 },
		{ &key[keyIdx_7], NULL, 					NULL, 						0, 		1, 0, 0 },
};

static TimerHandle_t xTimers;

static void vTimerCallback(TimerHandle_t xTimer) {
	keyStatusDetect();
}

void keyDetectInit(void) {
	xTimers = xTimerCreate("keyDetecter", pdMS_TO_TICKS(TIME_PERIDIC), pdTRUE,
			(void*) 0, vTimerCallback);
	if (xTimers == NULL) {
		return;
	}
	if (pdPASS != xTimerStart(xTimers, 0)) {
	}
}

uint8_t keyGetStatus(uint8_t keyIndex) {
	return GPIO_ReadInputDataBit(keyStatus[keyIndex].pKey->pGPO, keyStatus[keyIndex].pKey->pin);
}

static void keyStatusDetect(void) {
	uint8_t status;
	uint8_t keyNum;
	for (keyNum = keyIdx_1; keyNum < keyIdxNum; keyNum++) {
		status = GPIO_ReadInputDataBit(keyStatus[keyNum].pKey->pGPO, keyStatus[keyNum].pKey->pin);
		if (GPIO_ReadInputDataBit(keyStatus[keyNum].pKey->pGPO, keyStatus[keyNum].pKey->pin) != keyStatus[keyNum].status) {
			keyStatus[keyNum].time += TIME_PERIDIC;
			if (keyStatus[keyNum].time >= DEBOUNCE_CNT * TIME_PERIDIC) {
				keyStatus[keyNum].status = status;
				keyStatus[keyNum].time = 0;
				if (keyStatus[keyNum].status == 0) {
					if (keyStatus[keyNum].perssHandler) {
						keyStatus[keyNum].perssHandler();
						ps_reset_active_timer();
					}
				}
			}
			keyStatus[keyNum].decreaseTime = 0;
		} else {
			if(keyStatus[keyNum].decreaseTime >= DEBOUNCE_CNT * TIME_PERIDIC) {
				keyStatus[keyNum].time = 0;
			} else {
				if(keyStatus[keyNum].time != 0 ) {
					keyStatus[keyNum].decreaseTime += TIME_PERIDIC;
				}
			}
			if (keyStatus[keyNum].status == 0) {
				keyStatus[keyNum].time = 0;
				keyStatus[keyNum].consecutiveTime += TIME_PERIDIC;
				if (keyStatus[keyNum].consetcutivePressHandler) {
					if (keyStatus[keyNum].consecutiveTime >= keyStatus[keyNum].consecutiveTimeout) {
						keyStatus[keyNum].consecutiveTime = 0;
						ps_reset_active_timer();
						keyStatus[keyNum].consetcutivePressHandler();
					}
				}
			}
		}
	}
}

static void singleModeKeyHander(void) {
	(void) ps_send_event(PS_EVT_MODE_CHGE, 0);
}

static void mutleModeKeyHandler(void) {
	(void) ps_send_event(PS_EVT_MODE_CHGE, 1);
}

static void pageUpKeyHander(void) {
	ps_send_event(PS_EVT_PAGE, 0);
}

static void pageDownKeyHandler(void) {
	ps_send_event(PS_EVT_PAGE, 1);
}

static void startAndStopKeyHandler(void) {
	ps_send_event(PS_EVT_START_STOP_GAME, 1);
}

static void singleModeConsecutivePress(void) {
	if(keyStatus[keyIdx_2].status == 0) {
		ps_send_event(PS_EVT_POWERDOWN, 0);
	}
}

static void mutleModeConsecutivePress(void) {
	if(keyStatus[keyIdx_1].status == 0) {
		ps_send_event(PS_EVT_POWERDOWN, 0);
	}
}


static void pageUpConsecutivePress(void) {
	if(keyStatus[keyIdx_4].status == 0) {
		ps_send_event(PS_EVT_SWITCH_VOLUME, 0);
	}
}

static void pageDownConsecutivePress(void) {
	if(keyStatus[keyIdx_3].status == 0) {
		ps_send_event(PS_EVT_SWITCH_VOLUME, 0);
	}
}
