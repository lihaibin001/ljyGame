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

#define PLATE_AMOUNT 4
#define PLATE_AMOUNT_BIT 0x0F
#define PLATE_PLAYER1_BIT 0x03
#define PLATE_PLAYER2_BIT 0x0C
#define PLATE_STA_UNKNOW		0
#define PLATE_STA_OK			1
#define PLATE_STA_FAULT		2

#define PROTOCAL_LED_ON			(uint8_t)0
#define PROTOCAL_GAME_OVER		(uint8_t)1
#define PROTOCAL_SELF_TESET		(uint8_t)2

extern const uint8_t snatch_led[];
extern const uint8_t road_block[];
extern const uint8_t wipe_led[];
extern const uint8_t agil_train[];
extern const uint8_t *pNumber[];
extern const uint8_t gImage_a[];
showMode_t maxtrixAppGetShowMode(void);
uint8_t maxtrixAppSetGameLevel(uint8_t level);
void maxtrixAppGameLevelIncrease(void);
uint8_t maxtrixAppSetImage(uint8_t *pImage);
bool maxtrixAppSelfTest(void);
void maxtrixAppFault(void);
bool maxtrixApplockPlate(uint8_t i, uint32_t timeout);
void maxtrixAppUnlockPlate(uint8_t i);
bool maxtrixAppSetPlateStatus(uint8_t i, uint8_t status);
bool maxtrixAppGetPlateStatue(uint8_t *status);
void maxtrixAppSetGameMode(uint8_t mode);
uint8_t maxtrixAppGetGameMode(void);
void maxtrixAppGameStart(void);
void maxtrixAppGameStop(void);
bool maxtrixAppGetGameStatus(void);
void maxtriAppInit(void);
void maxtriAppStartTime(void);
void maxtriAppStopTime(void);
uint8_t maxtriAppGetPlayerSalverId(uint8_t player);
void maxtriAppScoreIncrease(uint8_t player);
uint8_t maxtriAppGetScore(uint8_t player);
void maxtriAppScoreDecrease(uint8_t player);
void maxtriAppResetPlayerSalverId(void);
#endif
