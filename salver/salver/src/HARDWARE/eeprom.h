#ifndef __M24256_H__
#define __M24256_H__

#include "stm32f10x_conf.h"
#include "stdbool.h"
#define EE_FINISH			(uint8_t)0
#define EE_PARAMETER_ERR	(uint8_t)1
#define EE_ERROR			(uint8_t)2

#define DEVICE_ADDRESS 0xA0

#define EE_ADDRES 0xA0
#define EE_WRITE_COM (EE_ADDRES & 0xFE)
#define EE_READ_COM (EE_ADDRES | 0x01)

#define EE_SIZE (uint16_t)4096

void EE_Write_Byte(uint16_t WriteAddr, uint8_t ByteValue);
uint8_t EE_Write_Data(uint16_t WriteAddr, uint8_t *pData, uint16_t DataLen);
uint8_t EE_Read_Byte(uint16_t ReadAddr);
uint8_t EE_Read_Data(uint16_t addr, uint8_t* pBuff, uint16_t DataLen);
bool EE_uint_test(void);

#endif //__M24256_H__
