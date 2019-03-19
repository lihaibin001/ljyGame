#ifndef __MAXTRIXAPP_H__
#define __MAXTRIXAPP_H__
#include <stdint.h>
#include <stdbool.h>
typedef enum
{
    showImage,
    showGameMode,
    showGameLevel,
    showScores,
}showMode_t;

#define PLATE_AMOUNT 8
#define PLATE_STA_UNKNOW		0
#define PLATE_STA_OK			1
#define PLATE_STA_FAULT		2

#define PROTOCAL_LED_ON			(uint8_t)0
#define PROTOCAL_LED_OFF		(uint8_t)1
#define PROTOCAL_SELF_TESET		(uint8_t)2

extern const uint8_t snatch_led[];
extern const uint8_t road_block[];
extern const uint8_t wipe_led[];
extern const uint8_t agil_train[];
extern const uint8_t *pNumber[];
extern const uint8_t gImage_a[];
uint8_t maxtrixAppGetGameLevel(void);
showMode_t maxtrixAppGetShowMode(void);
uint8_t maxtrixAppSetGameLevel(uint8_t level);
void maxtrixAppGameLevelIncrease(void);
uint8_t maxtrixAppSetGameMode(uint8_t cnt);
uint8_t maxtrixAppSetImage(uint8_t *pImage);
void maxtrixAppDisplayBootImage(void);
bool maxtrixAppSelfTest(void);
void maxtrixAppFault(void);
bool maxtrixApplockPlate(uint8_t i, uint32_t timeout);
void maxtrixAppUnlockPlate(uint8_t i);
bool maxtrixAppSetPlateStatus(uint8_t i, uint8_t status);
bool maxtrixAppGetPlateStatue(uint8_t *status);
void maxtrixAppBooting(void);
uint8_t maxtrixAppGetGameMode(void);
void maxtrixAppGameStart(void);
void maxtrixAppGameStop(void);
bool maxtrixAppGetGameStatus(void);
void maxtriAppInit(void);
uint8_t maxtriAppGetPlayerSalverId(uint8_t player);
void maxtriAppScoreIncrease(uint8_t player);
void maxtriAppResetPlayerSalverId(void);
#endif
