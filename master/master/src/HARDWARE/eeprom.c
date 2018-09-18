#include "eeprom.h"
#include "delay.h"
#include "iic.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"

void EE_Write_Byte(uint16_t WriteAddr, uint8_t ByteValue)
{
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
	Delay_ms(10);
}

uint8_t EE_Write_Data(uint16_t WriteAddr, uint8_t *pData, uint16_t DataLen)
{
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
	return EE_FINISH;
}

uint8_t EE_Read_Byte(uint16_t ReadAddr)
{
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


uint8_t EE_Read_Data(uint16_t addr, uint8_t* pBuff, uint16_t DataLen)
{
	int BuffIndex;
	if(addr + DataLen >= EE_SIZE || pBuff == NULL)
	{
		return EE_PARAMETER_ERR;
	}
	for(BuffIndex = 0; BuffIndex < DataLen; BuffIndex++, addr++)
	{
		pBuff[BuffIndex] = EE_Read_Byte(addr);
	}
	return EE_FINISH;
}

bool EE_uint_test(void)
{
	uint16_t i, j;
	uint8_t buffer[16] = "";
	for(i = 0; i < EE_SIZE; i += 16)
	{
		for(j = 0; j < 16; j++)
		{
			buffer[j] = j;
		}
		EE_Write_Data(i, buffer, 16);
	}
	for(i = 0; i < EE_SIZE; i += 16)
	{
		memset(buffer, 0, 16);
		EE_Read_Data(i, buffer, 16);
	}
	return true;
}


