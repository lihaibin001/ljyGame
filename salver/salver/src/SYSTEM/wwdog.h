#ifndef __WWDOG_H__
#define __WWDOG_H__

#include "stm32f10x_conf.h"

void wwdogInit(void);
void wwdogFeed(void);
void wwdogEnable(void);
void wwdogDisable(void);
#endif