#ifndef __CANAPP_H__
#define __CANAPP_H__

#include "can.h"

extern uint8_t plate_color;
extern uint8_t plate_status;


void CanAppInit(void);
void CanAppSendMsg(can_frame_t *pFrame);
#endif //__CANAPP_H__
