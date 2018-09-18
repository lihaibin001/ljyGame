#include "wwdog.h"


void wwdogInit(void)
{
	/* On Value line devices, WWDG clock counter = (PCLK1 (24MHz)/4096)/8 = 732 Hz (~1366 us)  */
	/* On other devices, WWDG clock counter = (PCLK1(36MHz)/4096)/8 = 1099 Hz (~910 us)  */
	WWDG_SetPrescaler(WWDG_Prescaler_8);

	/* Set Window value to 80; WWDG counter should be refreshed only when the counter
	  is below 80 (and greater than 64) otherwise a reset will be generated */
	WWDG_SetWindowValue(80);

	/* - On Value line devices,
	  Enable WWDG and set counter value to 127, WWDG timeout = ~1366 us * 64 = 87.42 ms
	  In this case the refresh window is: ~1366us * (127-80) = 64.20 ms < refresh window < ~1366us * 64 = 87.42ms
	   - On other devices
	  Enable WWDG and set counter value to 127, WWDG timeout = ~910 us * 64 = 58.25 ms
	  In this case the refresh window is: ~910 us * (127-80) = 42.77 ms < refresh window < ~910 us * 64 = 58.25ms
	*/

}

void wwdogFeed(void)
{
	/* Update WWDG counter */
	WWDG_SetCounter(127);
}

void wwdogEnable(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
    WWDG_Enable(127);
}

void wwdogDisable(void)
{
     RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, DISABLE);   
}
