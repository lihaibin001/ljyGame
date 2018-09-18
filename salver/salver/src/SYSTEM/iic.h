#ifndef __IIC_H__
#define __IIC_H__

#include "stm32f10x_conf.h"

#define SDA_IN()  {GPIOB->CRH &= 0XFFFFFF0F;GPIOB->CRH |= (uint32_t)0x08 << 4;}
#define SDA_OUT() {GPIOB->CRH &= 0XFFFFFF0F;GPIOB->CRH |= (uint32_t)0x03 << 4;}

 
#define IIC_SCL    PBout(8) //SCL
#define IIC_SDA    PBout(9) //SDA	 
#define READ_SDA   PBin(9)  //SDA 


void IIC_Init(void);

void IIC_Start(void);

void IIC_Stop(void);

void IIC_Send_Byte(uint8_t ByteValue);

uint8_t IIC_Read_Byte(unsigned char ack);

uint8_t IIC_Wait_Ack(void);

void IIC_Ack(void);

void IIC_NAck(void);

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);

uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	  

#endif //__IIC_H__
