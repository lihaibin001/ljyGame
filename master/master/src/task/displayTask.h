#ifndef __DISPLAYTASK_H__
#define __DISPLAYTASK_H__
#include "stdbool.h"
#include "taskConfig.h"
#include "stdint.h"

bool CreateDisplayTask(void);
void DisplayRefalsh(void);
uint8_t DisplaySuspend(uint32_t waittime);
void DisplayResume(void);
#endif
