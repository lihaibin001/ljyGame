#include "lowpw.h"

/**
 * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
 *         and select PLL as system clock source.
 * @param  None
 * @retval None
 */
void SYSCLKConfig_STOP(void) {
	ErrorStatus HSEStartUpStatus;
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS) {

#ifdef STM32F10X_CL
		/* Enable PLL2 */
		RCC_PLL2Cmd(ENABLE);

		/* Wait till PLL2 is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET) {
		}

#endif

		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
		}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while (RCC_GetSYSCLKSource() != 0x08) {
		}
	}
}

static void unused_io_config(void) {

}

void lwpw_entery_stop(void) {
	unused_io_config();
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
	SYSCLKConfig_STOP();
}
