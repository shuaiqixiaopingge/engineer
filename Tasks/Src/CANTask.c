/**
  ******************************************************************************
  * File Name          : CANTask.c
  * Description        : CAN通信任务
  ******************************************************************************
  *
  * Copyright (c) 2018 Team TPP-Shanghai Jiao Tong University
  * All rights reserved.
  *
  ******************************************************************************
  */
#include "includes.h"

#define CanRxGetU16(canRxMsg, num) (((uint16_t)canRxMsg.Data[num * 2] << 8) | (uint16_t)canRxMsg.Data[num * 2 + 1])
uint8_t isRcanStarted_CM = 0, isRcanStarted_AUX = 0;
CanRxMsgTypeDef CMCanRxMsg, AUXCanRxMsg;
Motor820RRxMsg_t CMFLRx,CMFRRx,CMBLRx,CMBRRx;
Motor820RRxMsg_t AMUD1Rx,AMUD2Rx,AMFBRx,AMSIDERx,WINDRx;
uint8_t can1_update = 1;
uint8_t can_type = 1;
uint8_t can2_update = 1;
/********************CAN发送*****************************/
//CAN数据标记发送，保证发送资源正常
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
	if(hcan == &CMMOTOR_CAN){
		can1_update = 1;
	}
	else if(hcan == &AUXMOTOR_CAN)
	{
		can2_update = 1;
	}
}

/********************CAN******************************/
void InitCanReception()
{
	//http://www.eeworld.com.cn/mcu/article_2016122732674_3.html
	CMMOTOR_CAN.pRxMsg = &CMCanRxMsg;
	/*##-- Configure the CAN1 Filter ###########################################*/
	CAN_FilterConfTypeDef  sFilterConfig;
	sFilterConfig.FilterNumber = 0;//14 - 27//14
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = 0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.BankNumber = 14;
  HAL_CAN_ConfigFilter(&CMMOTOR_CAN, &sFilterConfig);
	if(HAL_CAN_Receive_IT(&CMMOTOR_CAN, CAN_FIFO0) != HAL_OK){
		Error_Handler(); 
	}
	isRcanStarted_CM = 1;
	
	AUXMOTOR_CAN.pRxMsg = &AUXCanRxMsg;
	/*##-- Configure the CAN2 Filter ###########################################*/
	CAN_FilterConfTypeDef sFilterConfig2;
	sFilterConfig2.FilterNumber = 14;//14 - 27//14
	sFilterConfig2.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig2.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig2.FilterIdHigh = 0x0000;
  sFilterConfig2.FilterIdLow = 0x0000;
  sFilterConfig2.FilterMaskIdHigh = 0x0000;
  sFilterConfig2.FilterMaskIdLow = 0x0000;
  sFilterConfig2.FilterFIFOAssignment = 0;
  sFilterConfig2.FilterActivation = ENABLE;
  sFilterConfig2.BankNumber = 14;
  HAL_CAN_ConfigFilter(&AUXMOTOR_CAN, &sFilterConfig2);
	if(HAL_CAN_Receive_IT(&AUXMOTOR_CAN, CAN_FIFO0) != HAL_OK){
		Error_Handler(); 
	}
	isRcanStarted_AUX = 1;
}

//CAN接收中断入口函数
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan){
	if(hcan == &CMMOTOR_CAN){//CAN1数据
		switch(CMCanRxMsg.StdId){
			case CMFL_RXID:
				CMFLRx.angle = CanRxGetU16(CMCanRxMsg, 0);
				CMFLRx.RotateSpeed = CanRxGetU16(CMCanRxMsg, 1);
				break;
			case CMFR_RXID:
				CMFRRx.angle = CanRxGetU16(CMCanRxMsg, 0);
				CMFRRx.RotateSpeed = CanRxGetU16(CMCanRxMsg, 1);
				break;
			case CMBL_RXID:
				CMBLRx.angle = CanRxGetU16(CMCanRxMsg, 0);
				CMBLRx.RotateSpeed = CanRxGetU16(CMCanRxMsg, 1);
				break;
			case CMBR_RXID:
				CMBRRx.angle = CanRxGetU16(CMCanRxMsg, 0);
				CMBRRx.RotateSpeed = CanRxGetU16(CMCanRxMsg, 1);
				break;
			//以下两个正式版要注释掉delete
			case 0x205u :
				break;
			case 0x206u :
				break;
			default:
			Error_Handler();
		}
		if(HAL_CAN_Receive_IT(&CMMOTOR_CAN, CAN_FIFO0) != HAL_OK){
			isRcanStarted_CM = 0;
		}else{
			isRcanStarted_CM = 1;
		}
	}
	else if(hcan == &AUXMOTOR_CAN)//CAN2数据
	{
		switch(AUXCanRxMsg.StdId)
		{
			case AMUD1_RXID:
				AMUD1Rx.angle = CanRxGetU16(AUXCanRxMsg, 0);
				AMUD1Rx.RotateSpeed = CanRxGetU16(AUXCanRxMsg, 1);
				break;
			case AMUD2_RXID:
				AMUD2Rx.angle = CanRxGetU16(AUXCanRxMsg, 0);
				AMUD2Rx.RotateSpeed = CanRxGetU16(AUXCanRxMsg, 1);
				break;
			case AMFB_RXID:
				AMFBRx.angle = CanRxGetU16(AUXCanRxMsg, 0);
				AMFBRx.RotateSpeed = CanRxGetU16(AUXCanRxMsg, 1);
				break;
			case AMSIDE_RXID:
				AMSIDERx.angle = CanRxGetU16(AUXCanRxMsg, 0);
				AMSIDERx.RotateSpeed = CanRxGetU16(AUXCanRxMsg, 1);
				break;
			case WIND_RXID:
				WINDRx.angle = CanRxGetU16(AUXCanRxMsg, 0);
				WINDRx.RotateSpeed = CanRxGetU16(AUXCanRxMsg, 1);
				break;
			//以下一个正式版要注释掉delete
			case 0x401u:
				break;
			default:
			Error_Handler();
		}
		if(HAL_CAN_Receive_IT(&AUXMOTOR_CAN, CAN_FIFO0) != HAL_OK)
		{
			isRcanStarted_AUX = 0;
		}else{
			isRcanStarted_AUX = 1;
		}
	}
}

//delete
//单轴陀螺仪初始化，在主控制任务中，开机三秒后执行
#define ZGYRO_TXID   0x404u
#define ZGYRO_CAN hcan2
void GYRO_RST(void)
{
	CanTxMsgTypeDef pData;
	ZGYRO_CAN.pTxMsg = &pData;
	
	ZGYRO_CAN.pTxMsg->StdId = ZGYRO_TXID;
	ZGYRO_CAN.pTxMsg->ExtId = 0;
	ZGYRO_CAN.pTxMsg->IDE = CAN_ID_STD;
	ZGYRO_CAN.pTxMsg->RTR = CAN_RTR_DATA;
	ZGYRO_CAN.pTxMsg->DLC = 0x08;
	ZGYRO_CAN.pTxMsg->Data[0] = 0x00;
	ZGYRO_CAN.pTxMsg->Data[1] = 0x01;
	ZGYRO_CAN.pTxMsg->Data[2] = 0x02;
	ZGYRO_CAN.pTxMsg->Data[3] = 0x03;
	ZGYRO_CAN.pTxMsg->Data[4] = 0x04;
	ZGYRO_CAN.pTxMsg->Data[5] = 0x05;
	ZGYRO_CAN.pTxMsg->Data[6] = 0x06;
	ZGYRO_CAN.pTxMsg->Data[7] = 0x07;

	if(can2_update)
	{
		HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
		HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
		HAL_NVIC_DisableIRQ(USART1_IRQn);
		HAL_NVIC_DisableIRQ(DMA2_Stream2_IRQn);
		HAL_NVIC_DisableIRQ(TIM7_IRQn);
		#ifdef DEBUG_MODE
			HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
		#endif
		if(HAL_CAN_Transmit_IT(&ZGYRO_CAN) != HAL_OK)
		{
			Error_Handler();
		}
		can2_update = 0;
		HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
		HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
		HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
		HAL_NVIC_EnableIRQ(TIM7_IRQn);
		#ifdef DEBUG_MODE
			HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
		#endif
  }
}
