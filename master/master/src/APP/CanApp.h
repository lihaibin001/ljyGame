#ifndef __CANAPP_H__
#define __CANAPP_H__

#include "can.h"

typedef enum {
	CanApp_Ok,
	CanApp_bus_err,
	CanApp_timeout,
}CanAppRet_t;

void CanAppInit(void);
RET_t CanAppSendMsg(can_frame_t *pFrame);
#endif //__CANAPP_H__
