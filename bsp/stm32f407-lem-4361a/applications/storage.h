#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <rtthread.h>
#include <rtdevice.h>

/***********��Դ·�����������ýṹ��******************************/
typedef struct
{
	char cAssetNum[23];//·�����ʲ���� �ַ��� maxlen=22
	
}ScmRouterPata_Msg;//·��������
extern ScmRouterPata_Msg  stRouterPata_msg;
/****************************************************************/

typedef struct
{
	char cPlieAddr[17];//���׮���  �ַ��� maxlen=16
	unsigned char ucInterfaceNum;//���ӿڱ�ʶ
	char cInstallation[40];//���׮��װ��ַ  maxlen = 40
	unsigned long ulMinimunPow;//��С��繦��
	unsigned long ulRatePow;//�����
	unsigned char ucWake;//�Ƿ�֧�ֻ���
}ScmChargePilePara;//���׮����




typedef enum {
	Cmd_MeterNumWr=0,                   //0
	Cmd_MeterNumRd,
	Cmd_MeterPowerWr,                   //0
	Cmd_MeterPowerRd,
	Cmd_MeterGJFModeWr,                 //0
	Cmd_MeterGJFModeRd,	
	Cmd_MeterHalfPowerWr,               //0
	Cmd_MeterHalfPowerRd,
	Cmd_MeterAnalogWr,                  //0
	Cmd_MeterAnalogRd,
	Cmd_HistoryRecordWr,                /*��綩���¼���¼��Ԫ*/
	Cmd_HistoryRecordRd,	
	Cmd_OrderChargeWr,                  /*�������¼���¼��Ԫ*/
	Cmd_OrderChargeRd,		
	Cmd_PlanOfferWr,                    /*���ƻ��ϱ���¼��Ԫ*/
	Cmd_PlanOfferRd,
	Cmd_PlanFailWr,                     /*���ƻ�����ʧ�ܼ�¼��Ԫ*/
	Cmd_PlanFailRd,	
	Cmd_OnlineStateWr,                  /*�������״̬*/
	Cmd_OnlineStateRd,		
	
	
	
	End_Sto_cmdListNum,
}STORAGE_CMD_ENUM;
#define STORAGE_CMD_ENUM rt_uint32_t


extern int GetStorageData(STORAGE_CMD_ENUM cmd,void *STO_GetPara,rt_uint32_t datalen);
extern int SetStorageData(STORAGE_CMD_ENUM cmd,void *STO_SetPara,rt_uint32_t datalen);


#endif

