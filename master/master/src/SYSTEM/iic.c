#include "iic.h"
#include "sys.h"
#include "delay.h"

void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);
}

void IIC_Start(void)
{
	SDA_OUT();
	IIC_SDA = 1;
	IIC_SCL = 1;
	Delay_us(4);
	IIC_SDA = 0;
	Delay_us(4);
	IIC_SCL = 0;
}

void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SCL = 0;
	IIC_SDA = 0;
	Delay_us(4);
	IIC_SCL = 1;
	Delay_us(4);
	IIC_SDA = 1;
}

uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime = 0;
	SDA_IN();
	IIC_SDA = 1;
	Delay_us(1);

	IIC_SCL = 1;
	Delay_us(1);
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0;
	return 0;
}

void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	Delay_us(2);
	IIC_SCL = 1;
	Delay_us(2);
	IIC_SCL = 0;
}

void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	Delay_us(2);
	IIC_SCL = 1;
	Delay_us(2);
	IIC_SCL = 0;
}

void IIC_Send_Byte(uint8_t ByteValue)
{
	uint8_t t;
	SDA_OUT();
	IIC_SCL = 0;
	for(t = 0; t < 8; t++)
	{
		if((ByteValue & 0x80) >> 7)
		{
			IIC_SDA = 1;
		}
		else
		{
			IIC_SDA = 0;
		}
		ByteValue <<= 1;
		Delay_us(2);
		IIC_SCL = 1;
		Delay_us(2);
		IIC_SCL = 0;
		Delay_us(2);
	}
}

uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SDA_IN();
	for(i = 0; i < 8; i++)
	{
		IIC_SCL = 0;
		Delay_us(2);
		IIC_SCL = 1;
		receive <<= 1;
		if(READ_SDA)
		{
			receive++;
		}
		Delay_us(1);
	}
	if(!ack)
	{
		IIC_NAck();
	}
	else
	{
		IIC_Ack();
	}
	return receive;
}
