#include "eeprom.h"
#include "delay.h"
#include "iic.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"
#include "debug.h"

#define PAGE_SIZE 32
#define EEPROM_SIZE 4*1024

void EE_Write_Byte(uint16_t WriteAddr, uint8_t ByteValue) {
	IIC_Start();

	IIC_Send_Byte(EE_WRITE_COM);
	IIC_Wait_Ack();
	IIC_Send_Byte((WriteAddr >> 8) & 0xFF);

	IIC_Wait_Ack();
	IIC_Send_Byte(WriteAddr & 0xFF);
	IIC_Wait_Ack();
	IIC_Send_Byte(ByteValue);
	IIC_Wait_Ack();
	IIC_Stop();
	Delay_ms(5);
}

static void EE_Write_Page(uint16_t WriteAddr, uint8_t *pData, uint16_t DataLen) {
	IIC_Start();
	IIC_Send_Byte(EE_WRITE_COM);
	IIC_Wait_Ack();

	IIC_Send_Byte((WriteAddr >> 8) & 0xFF);
	IIC_Wait_Ack();
	IIC_Send_Byte(WriteAddr & 0xFF);
	IIC_Wait_Ack();

	while (DataLen--) {
		IIC_Send_Byte(*pData++);
		IIC_Wait_Ack();
	}
	IIC_Stop();
	Delay_ms(5);
}

uint8_t EE_Write_Data(uint16_t WriteAddr, uint8_t *pData, uint16_t DataLen) {
#if 1
	uint8_t tmp[PAGE_SIZE] = "";
	if (WriteAddr + DataLen >= EEPROM_SIZE || pData == NULL || DataLen == 0) {
		return EE_PARAMETER_ERR ;
	}
	uint8_t page_cnt = DataLen / PAGE_SIZE;
	uint8_t page_remain = DataLen % PAGE_SIZE;
	uint8_t i;
	for (i = 0; i < page_cnt; i++) {
		EE_Write_Page(WriteAddr, pData, PAGE_SIZE);
		EE_Read_Data(WriteAddr, tmp, PAGE_SIZE);
		if (memcmp(pData, tmp, PAGE_SIZE)) {
			DEBUG_INFO("%s\r\n", tmp);
			DEBUG_ERROR("<EE> write error\r\n");
			return EE_ERROR;
		} else {
			WriteAddr += PAGE_SIZE;
			pData += PAGE_SIZE;
		}
	}
	if (page_remain) {
		EE_Write_Page(WriteAddr, pData, page_remain);
		EE_Read_Data(WriteAddr, tmp, page_remain);
		if (memcmp(pData, tmp, page_remain)) {
			DEBUG_ERROR("<EE> write error\r\n");
			return EE_ERROR;
		}
	}
#else
	uint16_t DataIndex = 0;
	if(WriteAddr + DataLen >= EE_SIZE || pData == NULL)
	{
		return EE_PARAMETER_ERR;
	}
	for(DataIndex = 0; DataIndex < DataLen; DataIndex++)
	{
		EE_Write_Byte(WriteAddr + DataIndex, pData[DataIndex]);
		if(EE_Read_Byte(WriteAddr + DataIndex) != pData[DataIndex])
		{
			return EE_ERROR;
		}
	}
#endif
	return EE_FINISH ;
}

uint8_t EE_Read_Byte(uint16_t ReadAddr) {
	uint8_t temp = 0;
	IIC_Start();

	IIC_Send_Byte(EE_WRITE_COM);
	IIC_Wait_Ack();

	IIC_Send_Byte((ReadAddr >> 8) & 0xFF);
	IIC_Wait_Ack();

	IIC_Send_Byte(ReadAddr & 0xFF);
	IIC_Wait_Ack();

	IIC_Start();

	IIC_Send_Byte(EE_READ_COM);
	IIC_Wait_Ack();

	temp = IIC_Read_Byte(0);
	IIC_Stop();
	return temp;
}

void EE_Read_Page(uint16_t WriteAddr, uint8_t *pData, uint16_t DataLen) {

}

uint8_t EE_Read_Data(uint16_t ReadAddr, uint8_t* pBuff, uint16_t BuffLen) {
#if 1
	if (ReadAddr + BuffLen >= EEPROM_SIZE || pBuff == NULL || BuffLen == 0) {
		return EE_PARAMETER_ERR ;
	}
	IIC_Start();
	IIC_Send_Byte(EE_WRITE_COM);
	IIC_Wait_Ack();

	IIC_Send_Byte((ReadAddr >> 8) & 0xFF);
	IIC_Wait_Ack();

	IIC_Send_Byte(ReadAddr & 0xFF);
	IIC_Wait_Ack();

	IIC_Start();
	IIC_Send_Byte(EE_READ_COM);
	IIC_Wait_Ack();

	while (BuffLen-- > 1) {
		*pBuff++ = IIC_Read_Byte(1);
	}
	*pBuff = IIC_Read_Byte(0);
	IIC_Stop();
#else
	int BuffIndex;
	if(ReadAddr + BuffLen >= EE_SIZE || pBuff == NULL)
	{
		return EE_PARAMETER_ERR;
	}
	for(BuffIndex = 0; BuffIndex < BuffLen; BuffIndex++, ReadAddr++)
	{
		pBuff[BuffIndex] = EE_Read_Byte(ReadAddr);
	}
#endif
	return EE_FINISH ;
}

bool EE_uint_test(void) {
	uint8_t error_code = 0;
	uint8_t buffer[100] = "12344567890123412433215124134123412352134123412341234112341341351423432512341234123";
	error_code = EE_Write_Data(0, buffer, 100);
	if (EE_FINISH != error_code) {
		DEBUG_ERROR("EE write err:%d\r\n", error_code);
	}
	return true;
}

