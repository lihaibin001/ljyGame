#ifndef __KEYDETECT_H__
#define __KEYDETECT_H__

#include <stdbool.h>
#include "sys.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "rand.h"
#include "string.h"
#include "maxtrixApp.h"

typedef enum
{
    keyIdx_1,
    keyIdx_2,
    keyIdx_3,
    keyIdx_4,
    keyIdx_5,
    keyIdx_6,
    keyIdx_7,
    keyIdxNum,
}keyIndex_t;



void keyDetectInit(void);
uint8_t keyGetStatus(uint8_t keyIndex);

#endif
