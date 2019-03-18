#include "keyDetect.h"
#include "sync.h"

#define TIME_PERIDIC 20

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

static keyStatus_t keyStatus[keyIdxNum] = {
		{ &key[keyIdx_1], singleModeKeyHander, NULL, 1, 0, 0 },
		{ &key[keyIdx_2], mutleModeKeyHandler, NULL, 1, 0, 0 },
		{ &key[keyIdx_3], pageUpKeyHander, NULL, 1, 0, 0 },
		{ &key[keyIdx_4], pageDownKeyHandler, NULL, 1, 0, 0 },
		{ &key[keyIdx_5], startAndStopKeyHandler, NULL, 1, 0, 0 },
		{ &key[keyIdx_6], NULL, NULL, 1, 0, 0 },
		{ &key[keyIdx_7], NULL, NULL, 1, 0, 0 },
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

static void keyStatusDetect(void) {
	uint8_t status;
	uint8_t keyNum;
	for (keyNum = keyIdx_1; keyNum < keyIdxNum; keyNum++) {
		status = GPIO_ReadInputDataBit(keyStatus[keyNum].pKey->pGPO, keyStatus[keyNum].pKey->pin);
		if (GPIO_ReadInputDataBit(keyStatus[keyNum].pKey->pGPO, keyStatus[keyNum].pKey->pin) != keyStatus[keyNum].status) {
			keyStatus[keyNum].time += TIME_PERIDIC;
			if (keyStatus[keyNum].time >= 60) {
				keyStatus[keyNum].status = status;
				keyStatus[keyNum].time = 0;
				if (keyStatus[keyNum].status == 0) {
					if (keyStatus[keyNum].perssHandler) {
						keyStatus[keyNum].perssHandler();
					}
				}
			}
			keyStatus[keyNum].decreaseTime = 0;
		} else {
			if(keyStatus[keyNum].decreaseTime >= 60) {
				keyStatus[keyNum].time = 0;
			} else {
				if(keyStatus[keyNum].time != 0 ) {
					keyStatus[keyNum].decreaseTime += TIME_PERIDIC;
				}
			}
			if (keyStatus[keyNum].status == 0) {
				keyStatus[keyNum].time = 0;
				keyStatus[keyNum].consecutiveTime += TIME_PERIDIC;
				if (keyStatus[keyNum].consecutiveTime >= 1000) {
					keyStatus[keyNum].consecutiveTime = 0;
					if (keyStatus[keyNum].consetcutivePressHandler) {
						keyStatus[keyNum].consetcutivePressHandler();
					}
				}
			}
		}
	}
}

static void singleModeKeyHander(void) {
	ps_send_event(PS_EVT_MODE_CHGE, 1);
}

static void mutleModeKeyHandler(void) {
	ps_send_event(PS_EVT_MODE_CHGE, 2);
}

static void pageUpKeyHander(void) {

}

static void pageDownKeyHandler(void) {

}

static void startAndStopKeyHandler(void) {

}

