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
/******************************** ·���������Ϣ ***********************************/	//zcx190710
typedef enum {
	DevSt_StandbyOK=0,          // ��������
	DevSt_InCharging,           // �����
	DevSt_DisCharging,          // �ŵ���
	DevSt_Fault,           		// ����
	DevSt_Update,				// ������
}DEVICE_WORKSTATE;/*·����״̬*/

typedef struct
{
	char UserID[65];   			//�û�id  visible-string��SIZE(64)��
	char Token[39];   			//�û���¼����  visible-string��SIZE(38)��
	unsigned char AccountState;	//�˻�״̬ {0��������1��Ƿ��}
}WHITE_LIST;/*·����������*/

typedef struct
{
	char kAddress[17];   		//ͨѶ��ַ	visible-string��SIZE(16)��
	char MeterNum[9];   		//���  visible-string��SIZE(8)��
	char KeyNum[9];				//��Կ��Ϣ	visible-string��SIZE(8)��	
}KEYINFO_UNIT;/*·������Կ��Ϣ��Ԫ*/

/******************************** ���׮�����Ϣ ***********************************/	//zcx190807
typedef enum {
	ChgSt_Standby=0,            //��������
	ChgSt_InCharging,           //�����
	ChgSt_Fault,            	//����
}CHGPILE_WORKSTATE;/*���׮״̬*/

/******************************** ������Ϣ ***********************************/		//zcx190710
typedef enum 
{
//	NO_FAULT			=0x00000000,
//	Memory_FAULT		=0x00000001,		//	�ն������ڴ���ϣ�0��
//	Clock_FAULT			=0x00000002,		//  ʱ�ӹ���        ��1��
//	Board_FAULT			=0x00000004,		//  ����ͨ�Ź���    ��2��
//	MeterCom_FAULT		=0x00000008,		//  485�������     ��3��
//	Screen_FAULT		=0x00000010,		//  ��ʾ�����      ��4��
//	CarrierChl_FAULT	=0x00000020,		//  �ز�ͨ���쳣    ��5��
//	NandF_FAULT			=0x00000040,		//	NandFLASH��ʼ������  ��6��
//	ESAM_FAULT			=0x00000080,		//  ESAM����        ��7��
//	Bluetooth_FAULT		=0x00000100,		//	����ģ�����     ��8��
//	Battery_FAULT		=0x00000200,		//	��Դģ����� 	��9��
//	CanCom_FAULT		=0x00000400,		//  ���׮ͨ�Ź��� ��10��
//	ChgPile_FAULT		=0x00000800,		//  ���׮�豸���� ��11��
//	OrdRecord_FAULT		=0x00001000, 		//	���ض�����¼�� ��12��
	NO_FAULT=0,
	Memory_FAULT,		//	�ն������ڴ���ϣ�0��
	Clock_FAULT,		//  ʱ�ӹ���        ��1��
	Board_FAULT,		//  ����ͨ�Ź���    ��2��
	MeterCom_FAULT,		//  485�������     ��3��
	Screen_FAULT,		//  ��ʾ�����      ��4��
	CarrierChl_FAULT,	//  �ز�ͨ���쳣    ��5��
	NandF_FAULT,		//	NandFLASH��ʼ������  ��6��
	ESAM_FAULT,			//  ESAM����        ��7��
	Bluetooth_FAULT,	//	����ģ�����     ��8��
	Battery_FAULT,		//	��Դģ����� 	��9��
	CanCom_FAULT,		//  ���׮ͨ�Ź��� ��10��
	ChgPile_FAULT,		//  ���׮�豸���� ��11��
	OrdRecord_FAULT, 	//	���ض�����¼�� ��12��
}DEVICE_FAULT;/*�豸��������*/


typedef enum 
{
	PILE_NOFAULT        =0x00000000,
	PILE_Memory_FAULT	=0x00000001,	//	�ն������ڴ���ϣ�0��
	PILE_Clock_FAULT	=0x00000002,	//  ʱ�ӹ���        ��1��
	PILE_Board_FAULT	=0x00000004,	//  ����ͨ�Ź���    ��2��
	PILE_MeterCom_FAULT	=0x00000008,	//  485�������    ��3��
	PILE_Screen_FAULT	=0x00000010,	//  ��ʾ�����      ��4��
	CardOffLine_FAULT	=0x00000020,	//	������ͨѶ�ж�  ��5��
	PILE_ESAM_FAULT		=0x00000040,	//  ESAM����        ��6��
	StopEct_FAULT		=0x00000080,	//  ��ͣ��ť�������ϣ�7��
	Arrester_FAULT		=0x00000100,	//	����������		��8��
	GunHoming_FAULT		=0x00000200,	//	���ǹδ��λ		��9��
	OverV_FAULT			=0x00000400,	//	�����ѹ�澯		��10��
	UnderV_FAULT		=0x00000800,	//	����Ƿѹ�澯		��11��
	Pilot_FAULT			=0x00001000,	//	����г������Ƶ����澯��12��
	Connect_FAULT		=0x00002000,	//	�����Ӵ�������	��13��
	OverI_Warning		=0x00004000,	//	��������澯		��14��
	OverI_FAULT			=0x00008000,	//	���������������	��15��
	ACCir_FAULT			=0x00010000,	//	������·������	��16��
	GunLock_FAULT		=0x00020000,	//	���ӿڵ��������ϣ�17��
	GunOverTemp_FAULT	=0x00040000,	//	���ӿڹ��¹���	��18��
	CC_FAULT			=0x00080000,	//	�������״̬CC�쳣��19��
	CP_FAULT			=0x00100000,	//	������״̬CP�쳣��20��
	PE_FAULT			=0x00200000,	//	PE���߹���		��21��
	Dooropen_FAULT		=0x00400000,	//	���Ŵ򿪹���		(22)
	Other_FAULT			=0x00800000,	//	������������	��23��
}PILE_FAULT;/*���׮��������*/


/******************************** �������������Ϣ ***********************************/
typedef struct
{
	char OrderSn[17];	//������  octet-string��SIZE(16)��
	unsigned char StartType;
	unsigned char StopType;
	unsigned char cSucIdle;	//�ɹ���ʧ��ԭ��:{0���ɹ�1��ʧ��255������}
}CTL_CHARGE;/*������������*/



typedef struct
{
	STR_SYSTEM_TIME strDecStartTime;//��ʼʱ��
	STR_SYSTEM_TIME strDecStopTime;	//����ʱ��
//	unsigned char PowDensity;		//�����ܶȣ����ʵ��ڵĿ����ȣ�����15���ӣ�5���ӣ�1���ӵȣ�
	unsigned long ulChargePow;		//��繦���趨ֵ
}CHARGE_TIMESOLT;/*ʱ�θ��ɵ�Ԫ*/

typedef struct
{
	char cRequestNO[17];			//���뵥��  octet-string��SIZE(16)��
	char cUserID[65];   			//�û�id  visible-string��SIZE(64)��
	unsigned char ucDecMaker;		//������  {��վ��1������������2��}
	unsigned char ucDecType; 		//��������{���ɣ�1�� ��������2��}
	STR_SYSTEM_TIME strDecTime;		//����ʱ��
	char cAssetNO[23];				//·�����ʲ����  visible-string��SIZE(22)��
	unsigned long ulChargeReqEle;	//��������������λ��kWh�����㣺-2��
	unsigned long ulChargeRatePow;	//������� ����λ��kW�����㣺-4��
	unsigned char ucChargeMode;		//���ģʽ {������0��������1��}
	unsigned char ucTimeSlotNum;	//ʱ�������
	CHARGE_TIMESOLT strChargeTimeSolts[50];//ʱ������ݣ����50��

}CHARGE_STRATEGY;/*���ƻ���*/
extern CHARGE_STRATEGY Chg_Strategy;
extern CHARGE_STRATEGY Adj_Chg_Strategy;

typedef struct
{
	char cRequestNO[17];	//���뵥��  octet-string��SIZE(16)��
	char cAssetNO[23];		//·�����ʲ����  visible-string��SIZE(22)��
	unsigned char cSucIdle;	//�ɹ���ʧ��ԭ��:{0���ɹ�1��ʧ��255������}
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
}CHARGE_EXE_STATE;/*���ƻ���ִ��״̬*/
extern CHARGE_EXE_STATE Chg_ExeState;


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

