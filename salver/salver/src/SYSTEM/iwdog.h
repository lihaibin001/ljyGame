#ifndef __IWDOG_H__
#define __IWDOG_H__

#include "stm32f10x_conf.h"

/**
  * @brief  初始化看门狗
  * @param  无
  * @retval 无
  */
void IWdog_Init(void);	

/**
  * @brief  喂狗
  * @param  无
  * @retval 无
  */
void IWdog_Feed(void);
#endif //__IWDOG_H__
