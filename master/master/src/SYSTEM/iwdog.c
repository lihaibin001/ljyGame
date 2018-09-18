#include "iwdog.h"

__IO uint32_t LsiFreq = 40000;

/**
  * @brief  初始化看门狗
  * @param  无
  * @retval 无
  */
void IWdog_Init(void)
{
/* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_256);

  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(0x0FFF);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();

}	

/**
  * @brief  喂狗
  * @param  无
  * @retval 无
  */
void IWdog_Feed(void)
{
    IWDG_ReloadCounter();  
}
