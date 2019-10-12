#ifndef __CHARGEPILE_H__
#define __CHARGEPILE_H__
#include <meter.h>

/* ������-�¼����ƿ� */
extern struct rt_event ChargePileEvent;
/****************�궨��**********************************/
//�����¼�����
typedef enum {
	NO_EVENT                  =0x00000000,
	ChargeStartOK_EVENT       =0x00000001,        //�����ɹ��¼�		      
	ChargeStartER_EVENT       =0x00000002,        //����ʧ���¼�			      
	ChargeStopOK_EVENT        =0x00000004,        //ֹͣ�ɹ��¼�		
	ChargeStopER_EVENT        =0x00000008,        //ֹͣʧ���¼�	
	SetPowerOK_EVENT		  =0x00000010,		  //�����·��ɹ��¼�
	SetPowerER_EVENT		  =0x00000020,		  //�����·�ʧ���¼�	
	PileComFau_EVENT      	  =0x00000040,        //ͨ���ж��¼�
				
} PILE_EVENT_TYPE;
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
typedef struct
{
	ScmMeter_Analog PileMeter;
	uint32_t  PWM_Duty;        // ռ�ձ� 1λС��
	uint32_t StartReson;
	uint32_t StopReson;

}ChargPilePara_TypeDef;

typedef enum {
	Cmd_ChargeStart=0,                  //0
	Cmd_ChargeStartResp,
	Cmd_ChargeStop,                     //
	Cmd_ChargeStopResp,
	Cmd_SetPower,                       //
	Cmd_GetPower,
	Cmd_SetPowerResp, 
	Cmd_RdVertion,                      //
	Cmd_RdVoltCurrPara,                 //
	
	End_cmdListNum,
}COMM_CMD_P;
#define COMM_CMD_P rt_uint32_t





extern rt_uint8_t ChargepileDataGetSet(COMM_CMD_P cmd,void *STR_SetPara);



















#endif

