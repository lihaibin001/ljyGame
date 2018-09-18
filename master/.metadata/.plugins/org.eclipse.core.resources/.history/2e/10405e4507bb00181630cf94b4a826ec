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

extern const uint8_t gImage_a[];
uint8_t maxtrixAppGetGameLevel(void);
showMode_t maxtrixAppGetShowMode(void);
uint8_t maxtrixAppSetGameLevel(uint8_t level);
void maxtrixAppGameLevelIncrease(void);
uint8_t maxtrixAppSetGameMode(uint8_t cnt);
uint8_t maxtrixAppSetImage(uint8_t *pImage);
void maxtrixDisplayBootImage(void);
uint8_t maxtrixGetGameMode(void);
void maxtrixGameStart(void);
void maxtrixGameStop(void);
bool maxtrixGetGameStatus(void);
#endif
