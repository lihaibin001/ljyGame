#include "keyDetect.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "ws2812b.h"
#include "rand.h"
#include "string.h"
#include "maxtrixApp.h"

#define TIME_PERIDIC 20

typedef void (*pfKeyPressHandler)(void);
typedef void (*pfKeyConsecutivePressHandler)(void);
typedef struct
{
	GPIO_TypeDef *pGPO;
	uint16_t pin;
} key_t;

static const key_t key[keyIdxNum] =
{

	{GPIOB, GPIO_Pin_12},
	{GPIOB, GPIO_Pin_13},
	{GPIOB, GPIO_Pin_14},
	{GPIOB, GPIO_Pin_15},
	{GPIOD, GPIO_Pin_8},
	{GPIOD, GPIO_Pin_9},
	{GPIOD, GPIO_Pin_10},
};

typedef struct
{
	key_t const *pKey;
	const pfKeyPressHandler perssHandler;
	const pfKeyConsecutivePressHandler consetcutivePressHandler;
	uint16_t consecutiveTime;
	uint8_t status;
	uint8_t time;

} keyStatus_t;

static void keyStatusDetect(void);
static void vTimerCallback(TimerHandle_t xTimer);
static void key1PressHandler(void);
static void key2PressHandler(void);
static void key3PressHandler(void);
static void key4PressHandler(void);

static void key1ConsecutivePressHandler(void);

static keyStatus_t keyStatus[keyIdxNum] =
{
	{&key[keyIdx_1], key1PressHandler, key1ConsecutivePressHandler, 1, 0},
	{&key[keyIdx_2], key2PressHandler, NULL, 1, 0},
	{&key[keyIdx_3], key3PressHandler, NULL, 1, 0},
	{&key[keyIdx_4], key4PressHandler, NULL, 1, 0},
	{&key[keyIdx_5], NULL, NULL, 1, 0},
	{&key[keyIdx_6], NULL, NULL, 1, 0},
	{&key[keyIdx_7], NULL, NULL, 1, 0},


};

static TimerHandle_t xTimers;

static void vTimerCallback(TimerHandle_t xTimer)
{
	keyStatusDetect();
}

void keyDetectInit(void)
{
	xTimers = xTimerCreate("keyDetecter", pdMS_TO_TICKS(TIME_PERIDIC), pdTRUE, (void*)0, vTimerCallback);
	if(xTimers == NULL)
	{
		return;
	}
	if(pdPASS !=  xTimerStart(xTimers, 0))
	{
	}
}

static void keyStatusDetect(void)
{
	uint8_t status;
	uint8_t keyNum;
	for(keyNum = keyIdx_1; keyNum < keyIdxNum; keyNum++)
	{
		status = GPIO_ReadInputDataBit(keyStatus[keyNum].pKey->pGPO, keyStatus[keyNum].pKey->pin);
		if(GPIO_ReadInputDataBit(keyStatus[keyNum].pKey->pGPO, keyStatus[keyNum].pKey->pin) != keyStatus[keyNum].status)
		{
			keyStatus[keyNum].time += TIME_PERIDIC;
			if(keyStatus[keyNum].time >= 60)
			{
				keyStatus[keyNum].status = status;
				keyStatus[keyNum].time = 0;
				if(keyStatus[keyNum].status == 0)
				{
					if(keyStatus[keyNum].perssHandler)
					{
						keyStatus[keyNum].perssHandler();
					}
				}
			}
		}
		else
		{
			if(keyStatus[keyNum].status == 0)
			{
				keyStatus[keyNum].time = 0;
				keyStatus[keyNum].consecutiveTime += TIME_PERIDIC;
				if(keyStatus[keyNum].consecutiveTime >= 1000)
				{
                    keyStatus[keyNum].consecutiveTime = 0;
					if(keyStatus[keyNum].consetcutivePressHandler)
					{
						keyStatus[keyNum].consetcutivePressHandler();
					}
				}
			}
		}
	}
}



static void key1PressHandler(void)
{
	if(maxtrixAppGetShowMode() == showGameLevel)
	{
		maxtrixAppGameLevelIncrease();
	}
	else
	{
		maxtrixAppSetGameLevel(maxtrixAppGetGameLevel());
	}
}
static void key2PressHandler(void)
{
	if(maxtrixAppGetShowMode() != showGameMode)
	{
		maxtrixAppSetGameMode(maxtrixGetGameMode());
	}
	else
	{
        if(maxtrixGetGameMode() != 1)
        {
            maxtrixAppSetGameMode(1);
        }
        else
        {
            maxtrixAppSetGameMode(2);
        }
	}    
}
static void key3PressHandler(void)
{
    maxtrixGameStart();
}
static void key4PressHandler(void)
{
    
}

static void key1ConsecutivePressHandler(void)
{
	maxtrixAppGameLevelIncrease();
}
