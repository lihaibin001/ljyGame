#ifndef __CAN_H__
#define __CAN_H__

#include "stm32f10x_conf.h"
#include "CanCfg.h"
#include "stdbool.h"
#define CAN_ID_STANDRD (uint8_t)0
#define CAN_ID_EXTEND (uint8_t)1

#define CAN_TYPE_DATA (uint8_t)0
#define CAN_TYPE_REMOTE (uint8_t)1

#define CAN_RX_DATA (uint8_t)0
#define CAN_TX_COMPLETE (uint8_t)1
#define CAN_WAKEUP (uint8_t)2
#define CAN_BUSSOFF_ERR (uint8_t)3
#define CAN_PASSIVE_ERR (uint8_t)4

typedef struct
{
    const uint32_t type : 1;
    const uint32_t id;
}canFirlter_t;

typedef struct
{
    const canFirlter_t *pFilter;
    const uint8_t num;
}canFIrlterList_t;

typedef enum
{
    CAN_Baud_1M = 0,
    CAN_Baud_500K,
    CAN_Baud_250K,
    CAN_Baud_125K,
    CAN_Baud_Num,
}CanBaud_t;
typedef enum
{
    canControllerIdx1 = 0,
//    canControllerIdx2,
    canControllerIdxNum,
}CanControllerIdx_t;

typedef struct
{
    struct
    {
        uint32_t timestamp : 16;
        uint32_t length : 4;
        uint32_t type : 1;
        uint32_t format : 1;
        uint32_t reserve : 10;
    };
    uint32_t id;
    union
    {
        struct
        {
            uint32_t dataWord0;
            uint32_t dataWord1;
        };
        struct
        {
            uint8_t dataByte3;
            uint8_t dataByte2;
            uint8_t dataByte1;
            uint8_t dataByte0;
            uint8_t dataByte7;
            uint8_t dataByte6;
            uint8_t dataByte5;
            uint8_t dataByte4;
        };
    };
}can_frame_t;

typedef void (*pHanlderCb)(CanControllerIdx_t, uint8_t);

typedef struct
{
    uint8_t isHaveMsg;
    can_frame_t Frame;
    pHanlderCb cb;
}CanHandler_t;

typedef struct
{
	uint8_t data[8];
	uint16_t id;
	uint8_t startBit;
	uint8_t dataLength;
	float resolution;
}CanDataType;	



void CanInit(CanControllerIdx_t controller, CanBaud_t baud, pHanlderCb cb, const canFIrlterList_t *pFirlter);
void CanDeinit(CanControllerIdx_t controller);
bool CanSend_MSG(CanControllerIdx_t controller, const can_frame_t *pFrame);
bool CanGet_MSG(CanControllerIdx_t controller, can_frame_t *pFrame);
#endif //__CAN_H__
