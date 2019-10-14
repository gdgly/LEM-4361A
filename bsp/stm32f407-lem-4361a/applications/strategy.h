#ifndef __STRATEGY_H__
#define __STRATEGY_H__

#include <string.h>
#include <stdio.h>
#include "global.h"
#include "chargepile.h"

extern ChargPilePara_TypeDef ChargePilePara;
extern rt_err_t DevState_Judge(void);

extern struct rt_semaphore rt_sem_bluetoothfau;

extern rt_uint8_t DeviceState;
extern rt_uint8_t ChgpileState;
extern rt_bool_t DeviceFauFlag;



/******************************** �������������Ϣ ***********************************/
typedef enum
{
	GUN_A=1,
	GUN_B,
}GUN_NUM;/*ǹ��� {Aǹ��1����Bǹ��2��}*/

typedef enum
{
	DISORDER=0,
	ORDER,
}CHARGE_MODE;/*���ģʽ {������0��������1��}*/

typedef enum
{
	EXE_ING=1,
	EXE_END,
	EXE_FAILED,
}EXESTATE;/*ִ��״̬ {1������ִ�� 2��ִ�н��� 3��ִ��ʧ��}*/

typedef enum
{
	STANDBY=1,
	CHARGING,
	FAULT,
}PILESTATE;/*���׮״̬��1������ 2������ 3�����ϣ�*/

typedef enum
{
	SEV_ENABLE=0,
	SEV_DISABLE,
}PILE_SERVICE;/*׮������ {���ã�0����ͣ�ã�1��}*/

typedef enum
{
	CONNECT=0,
	DISCONNECT,
}PILE_COM_STATE;/*��׮ͨ��״̬ {������0�����쳣��1��}*/

typedef enum
{
	CTRL_START=1,
	CTRL_STOP,
	CTRL_ADJPOW,
}CTRL_TYPE;/*{1������  2��ֹͣ  3����������}*/



/******************************* ��ͣ��� *************************************/
			
typedef struct
{
	char cAssetNO[23];		//·�����ʲ����  visible-string��SIZE(22)��
	GUN_NUM gunAB;		//��繦���趨ֵ����λ��kW�����㣺-4��
}StartStopChg;
/******************************* ���ƻ� *************************************/
typedef struct
{
	STR_SYSTEM_TIME strDecStartTime;//��ʼʱ��
	STR_SYSTEM_TIME strDecStopTime;	//����ʱ��
	unsigned long ulChargePow;		//��繦���趨ֵ����λ��kW�����㣺-4��
}CHARGE_TIMESOLT;/*ʱ�θ��ɵ�Ԫ*/

typedef struct
{
	char cRequestNO[17];			//���뵥��  octet-string��SIZE(16)��
	char cUserID[65];   			//�û�id  visible-string��SIZE(64)��
	unsigned char ucDecMaker;		//������  {��վ��1������������2��}
	unsigned char ucDecType; 		//��������{���ɣ�1�� ��������2��}
	STR_SYSTEM_TIME strDecTime;		//����ʱ��
	char cAssetNO[23];				//·�����ʲ����  visible-string��SIZE(22)��
	unsigned char GunNum;			//ǹ���	{Aǹ��1����Bǹ��2��}
	unsigned long ulChargeReqEle;	//��������������λ��kWh�����㣺-2��
	unsigned long ulChargeRatePow;	//������� ����λ��kW�����㣺-4��
	unsigned char ucChargeMode;		//���ģʽ {������0��������1��}
	unsigned char ucTimeSlotNum;	//ʱ�������
	CHARGE_TIMESOLT strChargeTimeSolts[50];//ʱ������ݣ����50��

}CHARGE_STRATEGY;/*���ƻ���*/
extern CHARGE_STRATEGY Chg_Strategy;//�·��ƻ���
extern CHARGE_STRATEGY Adj_Chg_Strategy;//����ƻ���

typedef struct
{
	char cRequestNO[17];	//���뵥��  octet-string��SIZE(16)��
	char cAssetNO[23];		//·�����ʲ����  visible-string��SIZE(22)��
	unsigned char cSucIdle;	//�ɹ���ʧ��ԭ��:{0���ɹ� 1��ʧ�� 255������}
}CHARGE_STRATEGY_RSP;/*���ƻ�����Ӧ*/

typedef struct
{
	char cRequestNO[17];			//���뵥��  octet-string��SIZE(16)��
	char cAssetNO[23];				//·�����ʲ����  visible-string��SIZE(22)��
	unsigned char exeState;			//ִ��״̬ {1������ִ�� 2��ִ�н��� 3��ִ��ʧ��}
	unsigned char ucTimeSlotNum;	//ʱ�������
										//���ƽ��
	unsigned long ulEleBottomValue[5]; 	//����ʾֵ��ֵ������״�ִ��ʱʾֵ������λ��kWh�����㣺-2��
	unsigned long ulEleActualValue[5]; 	//��ǰ����ʾֵ����λ��kWh�����㣺-2��
	unsigned long ucChargeEle[5];		//�ѳ��������λ��kWh�����㣺-2��
	unsigned long ucChargeTime;		//�ѳ�ʱ�䣨��λ��s��
	unsigned long ucPlanPower;		//�ƻ���繦�ʣ���λ��W�����㣺-1��
	unsigned long ucActualPower;	//��ǰ��繦�ʣ���λ��W�����㣺-1��
	unsigned short ucVoltage;		//��ǰ����ѹ����λ��V�����㣺-1��
	unsigned int ucCurrent;			//��ǰ����������λ��A�����㣺-3��
	unsigned char ChgPileState;		//���׮״̬��1������ 2������ 3�����ϣ�
}CHARGE_EXE_STATE;/*·��������״̬  �� ���ƻ���ִ��״̬*/
extern CHARGE_EXE_STATE Chg_ExeState;

typedef struct
{
	char cRequestNO[17];			//���뵥��  octet-string��SIZE(16)��
	char cUserID[65];   			//�û�id  visible-string��SIZE(64)��
	char cAssetNO[23];				//·�����ʲ����  visible-string��SIZE(22)��
	unsigned char GunNum;			//ǹ���	{Aǹ��1����Bǹ��2��}
	unsigned long ulChargeReqEle;	//��������������λ��kWh�����㣺-2��
	STR_SYSTEM_TIME	PlanUnChg_TimeStamp;//	�ƻ��ó�ʱ��
	unsigned char ChargeMode;			//	���ģʽ {������0��������1��}
	char Token[33];   					//	�û���¼����  visible-string��SIZE(32)��
}CHARGE_APPLY;/*������뵥(BLE)*/

/******************************* ������ *************************************/
typedef struct
{
	char OrderSn[17];			//������  octet-string��SIZE(16)��
	unsigned char CtrlType;		//��������{1������  2��ֹͣ  3����������}
	unsigned char StartType;	//��������{1��4G����  2:��������}
	unsigned char StopType;		//ͣ������{1��4Gͣ��  2:����ͣ��}
	unsigned long SetPower;		//�趨��繦�ʣ���λ��W�����㣺-1��
	unsigned char cSucIdle;		//�ɹ���ʧ��ԭ��:{0���ɹ� 1��ʧ�� 255������}
}CTL_CHARGE;/*������������*/



/******************************** �¼���Ϣ��¼ ***********************************/
typedef struct
{
	unsigned long OrderNum;			//��¼���
	STR_SYSTEM_TIME OnlineTimestamp;		//����ʱ��
	STR_SYSTEM_TIME OfflineTimestamp;		//����ʱ��
	unsigned char ChannelState;				//ͨ��״̬
	unsigned char AutualState;				//״̬�仯 {���ߣ�0���� ���ߣ�1��}
	unsigned char OfflineReason;			//����ԭ�� {δ֪��0����ͣ�磨1�����ŵ��仯��2��}
}ONLINE_STATE;/*�������״̬*/

typedef struct
{
	unsigned long OrderNum;			//	�¼���¼��� 
	STR_SYSTEM_TIME StartTimestamp;	//  �¼�����ʱ��  
	STR_SYSTEM_TIME FinishTimestamp;//  �¼�����ʱ��  
	unsigned char Reason;			//  �¼�����ԭ��     
	unsigned char ChannelState;		//  �¼��ϱ�״̬ = ͨ���ϱ�״̬
	char RequestNO[17];				//	������뵥��   ��SIZE(16)��
	char AssetNO[23];				//	·�����ʲ���� visible-string��SIZE(22)��
	char Data[33];					//  ��n�������������Ե����� 
}PLAN_FAIL_EVENT;/*���ƻ�����ʧ�ܼ�¼��Ԫ*/

typedef struct
{
	unsigned long OrderNum;				//	�¼���¼��� 
	STR_SYSTEM_TIME StartTimestamp;		//  �¼�����ʱ��  
	STR_SYSTEM_TIME FinishTimestamp;	//  �¼�����ʱ��  
	unsigned char Reason;				//  �¼�����ԭ��     
	unsigned char ChannelState;			//  �¼��ϱ�״̬ = ͨ���ϱ�״̬
	char RequestNO[17];					//	������뵥��   ��SIZE(16)��
	char cUserID[65];   				//	�û�id  visible-string��SIZE(64)��
	char AssetNO[23];					//	·�����ʲ���� visible-string��SIZE(22)��
	unsigned long ChargeReqEle;			//	��������������λ��kWh�����㣺-2��
	STR_SYSTEM_TIME RequestTimeStamp;	//	�������ʱ��
	STR_SYSTEM_TIME	PlanUnChg_TimeStamp;//	�ƻ��ó�ʱ��
	unsigned char ChargeMode;			//	���ģʽ {������0��������1��}
	char Token[39];   					//	�û���¼����  visible-string��SIZE(38)��
	char UserAccount[10];				//  ����û��˺�  visible-string��SIZE(9)�� 
	char Data[33];						//  ��n�������������Ե�����
}PLAN_OFFER_EVENT;/*���ƻ��ϱ���¼��Ԫ*/

typedef struct
{
	unsigned long OrderNum;				//	�¼���¼��� 
	STR_SYSTEM_TIME StartTimestamp;		//  �¼�����ʱ��  
	STR_SYSTEM_TIME FinishTimestamp;	//  �¼�����ʱ��  
	unsigned char Reason;				//  �¼�����ԭ��     
	unsigned char ChannelState;			//  �¼��ϱ�״̬ = ͨ���ϱ�״̬
	unsigned int TotalFault;			//	�ܹ���
	char Fau[32];						//	�������״̬
}ORDER_CHG_EVENT;/*�������¼���¼��Ԫ*/

typedef struct
{
	unsigned long OrderNum;				//	�¼���¼��� 
	STR_SYSTEM_TIME StartTimestamp;		//  �¼�����ʱ��  
	STR_SYSTEM_TIME FinishTimestamp;	//  �¼�����ʱ��  
	unsigned char Reason;				//  �¼�����ԭ��     
	unsigned char ChannelState;			//  �¼��ϱ�״̬ = ͨ���ϱ�״̬
	char RequestNO[17];					//	������뵥��   ��SIZE(16)��
	char cUserID[65];   				//	�û�id  visible-string��SIZE(64)��
	char AssetNO[23];					//	·�����ʲ���� visible-string��SIZE(22)��
	unsigned long ChargeReqEle;			//	��������������λ��kWh�����㣺-2��
	STR_SYSTEM_TIME RequestTimeStamp;	//	�������ʱ��
	STR_SYSTEM_TIME	PlanUnChg_TimeStamp;//	�ƻ��ó�ʱ��
	unsigned char ChargeMode;			//	���ģʽ {������0��������1��}
	unsigned long StartMeterValue;		//	����ʱ�����ֵ
	unsigned long StopMeterValue;		//	ֹͣʱ�����ֵ
	STR_SYSTEM_TIME	ChgStartTime;		//	�������ʱ��
	STR_SYSTEM_TIME ChgStopTime;		//	���ֹͣʱ��
	unsigned long ucChargeEle;			//	�ѳ��������λ��kWh�����㣺-2��
	unsigned long ucChargeTime;			//	�ѳ�ʱ�䣨��λ��s��
}CHG_ORDER_EVENT;/*��綩���¼���¼��Ԫ*/

#endif

