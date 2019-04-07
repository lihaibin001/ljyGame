#ifndef __CANAPP_H__
#define __CANAPP_H__

#include "can.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "sys.h"
#include "ws2812b.h"
#include "ws2812b_conf.h"
extern uint8_t plate_color;
extern uint8_t plate_status;
extern TimerHandle_t g_led_off_timer;
void CanAppInit(void);
void CanAppSendMsg(can_frame_t *pFrame);
#endif //__CANAPP_H__
