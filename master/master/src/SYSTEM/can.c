#include "can.h"
#include "string.h"

#ifdef CAN_DEBUG
#define DEBUG(...) printf()
#else
#define DEBUG(...)
#endif

#ifdef CAN_ERROR_DEBUG
#define ERROR_DEBUG(...) printf()
#else
#define ERROR_DEBUG(...)
#endif

#define RX_BUFF_AMOUT 10

static RxBuff_t rxBuff;
static const CAN_TypeDef *pCanController[canControllerIdxNum] = {CAN1};
static const uint8_t baudList[CAN_Baud_Num] = {6, 12, 24, 48};
static CanHandler_t handler[canControllerIdxNum] = {
		{.pRxBuff = &rxBuff}
};

static void filterConfig(const canFIrlterList_t *pFirlterList)
{
	uint8_t i;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

	for(i = 0; i < pFirlterList->num; i++)
	{
		if(pFirlterList->pFilter[i].type == CAN_ID_STANDRD)
		{
			CAN_FilterInitStructure.CAN_FilterIdHigh = pFirlterList->pFilter[i].id << 5;
			CAN_FilterInitStructure.CAN_FilterIdLow = 0;
		}
		else
		{
			CAN_FilterInitStructure.CAN_FilterIdHigh = (pFirlterList->pFilter[i].id >> 16) & 0x1FFF;
			CAN_FilterInitStructure.CAN_FilterIdLow = pFirlterList->pFilter[i].id;
		}
		CAN_FilterInitStructure.CAN_FilterNumber = i;
		//CAN_FilterInitStructure.CAN_FilterFIFOAssignment = i / 2;
		CAN_FilterInit(&CAN_FilterInitStructure);
	}
}

static void filterDisbale(void)
{
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterActivation = DISABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
}

void CanInit(CanControllerIdx_t controller, CanBaud_t baud, pHanlderCb cb, const canFIrlterList_t *pFirlterList)
{
	if(controller >= canControllerIdxNum)
	{
		ERROR_DEBUG("[CAN] Can controller out of rannge\r\n");
		while(1)
			;
	}
	if(baud >= CAN_Baud_Num)
	{
		ERROR_DEBUG("[CAN] Baud out off range\r\n");
		while(1)
			;
	}

	CAN_TypeDef *pCan = (CAN_TypeDef *)pCanController[controller];
	CAN_InitTypeDef CAN_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = ENABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = ENABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = (uint16_t)baudList[canControllerIdxNum];

	CAN_Init(pCan, &CAN_InitStructure);
	if(pFirlterList != NULL)
	{
		filterConfig(pFirlterList);
	}
	else
	{
		filterDisbale();
	}
	switch((uint32_t)controller)
	{
		case 0:
			NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
			NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
			NVIC_Init(&NVIC_InitStructure);
			NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
			NVIC_Init(&NVIC_InitStructure);
			break;
		default:
			break;

	}

//	CAN_ITConfig(pCan, CAN_IT_TME, ENABLE);
	CAN_ITConfig(pCan, CAN_IT_FMP0, ENABLE);
	CAN_ITConfig(pCan, CAN_IT_BOF, ENABLE);
	handler[controller].cb = cb;
}

void CanDeinit(CanControllerIdx_t controller)
{
	if(controller >= canControllerIdxNum)
	{
		ERROR_DEBUG("[CAN] Can controller out of rannge\r\n");
		while(1)
			;
	}
	CAN_TypeDef *pCan = (CAN_TypeDef *)pCanController[controller];
	if(pCan != NULL)
	{
		if(pCan == CAN1)
		{
			RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, DISABLE);
			RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, ENABLE);
		}
		else if(pCan == CAN2)
		{
			RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN2, DISABLE);
			RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN2, ENABLE);
		}
	}
}
RET_t CanSend_MSG(CanControllerIdx_t controller, const can_frame_t *pFrame)
{
	uint8_t status;
	if(controller >= canControllerIdxNum)
	{
		ERROR_DEBUG("[CAN] Can controller out of rannge\r\n");
		return RET_PARAM_ERR;
	}
	if(pFrame == NULL)
	{
		ERROR_DEBUG("[CAN] Send message is null\r\n");
		return RET_PARAM_ERR;
	}
	else
	{
		CanTxMsg TxMessage;
		TxMessage.Data[0] = pFrame->dataByte0;
		TxMessage.Data[1] = pFrame->dataByte1;
		TxMessage.Data[2] = pFrame->dataByte2;
		TxMessage.Data[3] = pFrame->dataByte3;
		TxMessage.Data[4] = pFrame->dataByte4;
		TxMessage.Data[5] = pFrame->dataByte5;
		TxMessage.Data[6] = pFrame->dataByte6;
		TxMessage.Data[7] = pFrame->dataByte7;
		TxMessage.DLC = pFrame->length;
		if(pFrame->format == CAN_ID_STANDRD)
		{
			TxMessage.StdId = pFrame->id;
			TxMessage.IDE = CAN_Id_Standard;
		}
		else
		{
			TxMessage.ExtId = pFrame->id;
			TxMessage.IDE = CAN_RTR_Remote;
		}
		if(pFrame->type == CAN_TYPE_DATA)
		{
			TxMessage.RTR = CAN_RTR_Data;
		}
		else
		{
			TxMessage.RTR = CAN_RTR_Remote;
		}
		status = CAN_Transmit(CAN1, &TxMessage);
		if(status == CAN_TxStatus_NoMailBox)
		{
			ERROR_DEBUG("[CAN] Transmit fail: %d\r\n", status);
			return RET_PHY_ERR;
		}
		return RET_OK;
	}
}

RET_t CanGet_MSG(CanControllerIdx_t controller, can_frame_t *pFrame)
{
	if(controller >= canControllerIdxNum || pFrame == NULL)
	{
		return RET_PARAM_ERR;
	}

	if(handler[controller].pRxBuff->data_cnt == 0) {
		return RET_ERR;
	}

	memcpy(pFrame, &handler[controller].pRxBuff->frame[handler[controller].pRxBuff->out], sizeof(can_frame_t));
	handler[controller].pRxBuff->out++;
	if(handler[controller].pRxBuff->out == RX_BUFF_AMOUT) {
		handler[controller].pRxBuff->out = 0;
	}
	if(handler[controller].pRxBuff->data_cnt != 0) {
		handler[controller].pRxBuff->data_cnt--;
	}
	return RET_OK;
}

void CAN1_RX0_IRQHandler(void)
{

	if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
		if(handler[0].pRxBuff->data_cnt == RX_BUFF_AMOUT) {
			return ;
		}
		CanRxMsg RxMessage;
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte0 = RxMessage.Data[0];
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte1 = RxMessage.Data[1];
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte2 = RxMessage.Data[2];
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte3 = RxMessage.Data[3];
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte4 = RxMessage.Data[4];
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte5 = RxMessage.Data[5];
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte6 = RxMessage.Data[6];
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].dataByte7 = RxMessage.Data[7];
		if(RxMessage.IDE == CAN_Id_Extended)
		{
			handler[0].pRxBuff->frame[handler[0].pRxBuff->in].format = CAN_ID_EXTEND;
			handler[0].pRxBuff->frame[handler[0].pRxBuff->in].id = RxMessage.ExtId;
		}
		else
		{
			handler[0].pRxBuff->frame[handler[0].pRxBuff->in].id = RxMessage.StdId;
		}
		if(RxMessage.RTR == CAN_RTR_Remote)
		{
			handler[0].pRxBuff->frame[handler[0].pRxBuff->in].type = CAN_TYPE_REMOTE;
		}
		handler[0].pRxBuff->frame[handler[0].pRxBuff->in].length = RxMessage.DLC;
		handler[0].pRxBuff->in++;
		if(handler[0].pRxBuff->in == RX_BUFF_AMOUT) {
			handler[0].pRxBuff->in = 0;
		}
		handler[0].pRxBuff->data_cnt++;
		if(handler[0].cb)
		{
			handler[0].cb(canControllerIdx1, CAN_RX_DATA);
		}

	}
}

void CAN1_TX_IRQHandler(void)
{
	if(CAN_GetITStatus(CAN1, CAN_IT_TME) != RESET)
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
		if(handler[canControllerIdx1].cb)
		{
			handler[canControllerIdx1].cb(canControllerIdx1, CAN_TX_COMPLETE);
		}
	}
}

void CAN1_RX1_IRQHandler(void)
{
}

void CAN1_SCE_IRQHandler(void)
{
	uint8_t error = 0;
	if(CAN_GetITStatus(CAN1, CAN_IT_BOF) == SET)
	{
		error = CAN_BUSSOFF_ERR;
	}
	if(handler[canControllerIdx1].cb)
	{
		handler[canControllerIdx1].cb(canControllerIdx1, error);
	}
}


void CAN2_TX_IRQHandler(void)
{

}
void CAN2_RX0_IRQHandler(void)
{

}
void CAN2_RX1_IRQHandler(void)
{

}
void CAN2_SCE_IRQHandler(void)
{

}

