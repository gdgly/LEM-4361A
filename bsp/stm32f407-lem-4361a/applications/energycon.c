#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <stdio.h>
#include <global.h>
#include "energycon.h"
#include "chargepile.h"
#include "strategy.h"
#include "698.h"
#include "meter.h"
#include "storage.h"
#include <board.h>


#define THREAD_ENERGYCON_PRIORITY     18
#define THREAD_ENERGYCON_STACK_SIZE   1024
#define THREAD_ENERGYCON_TIMESLICE    20

#define RELAYA_PIN    GET_PIN(F, 2)
#define RELAYB_PIN    GET_PIN(F, 3)

#ifndef FALSE
#define FALSE         0
#endif
#ifndef TRUE
#define TRUE          1
#endif
#ifndef SUCCESSFUL
#define SUCCESSFUL    0
#endif
#ifndef FAILED
#define FAILED        1
#endif
#ifndef ORTHERS
#define ORTHERS       255
#endif

static rt_uint8_t energycon_stack[THREAD_ENERGYCON_STACK_SIZE];//�̶߳�ջ

struct rt_thread energycon;
struct rt_semaphore rx_sem_energycon;     //���ڽ������ݵ��ź���

CTL_CHARGE Ctrl_Start;
CTL_CHARGE Ctrl_Stop;
CTL_CHARGE Ctrl_PowerAdj;
CTL_CHARGE_EVENT CtrlCharge_Event;

//ָ���־
static rt_bool_t startchg_flag;
static rt_bool_t stopchg_flag;
static rt_bool_t adjpower_flag;

//��ʱ���
static rt_timer_t StartChgResp;
static rt_timer_t StopChgResp;
static rt_timer_t PowerAdjResp;

static unsigned char count;
static unsigned char SetPowerFinishFlag[50];
static char cRequestNO_Old[17];
static char cRequestNO_New[17];



void RELAY_ON(void)//���ϼ̵���
{
	rt_pin_write(RELAYA_PIN, PIN_LOW);
	rt_pin_write(RELAYB_PIN, PIN_HIGH);
}

void RELAY_OFF(void)//�Ͽ��̵���
{
	rt_pin_write(RELAYB_PIN, PIN_LOW);
	rt_pin_write(RELAYA_PIN, PIN_HIGH);
}
/**************************************************************
 * ��������: StartChgResp_Timeout 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: ������糬ʱ����
 **************************************************************/
static void StartChgResp_Timeout(void *parameter)
{
    rt_lprintf("[strategy] : StartChgResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_ChargeStartResp,0);
}
/**************************************************************
 * ��������: StopChgResp_Timeout 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: ֹͣ��糬ʱ����
 **************************************************************/
static void StopChgResp_Timeout(void *parameter)
{
    rt_lprintf("[strategy] : StopChgResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_ChargeStopResp,0);
}
/**************************************************************
 * ��������: PowAdjResp_Timeout 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: �������ʳ�ʱ����
 **************************************************************/
static void PowAdjResp_Timeout(void *parameter)
{
    rt_lprintf("[strategy] : PowerAdjResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_SetPowerResp,0);
}
/**************************************************************
 * ��������: timer_create_init 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: ��ʱ��
 **************************************************************/
static void timer_create_init()
{
    /* ���������ظ���ʱ�� */
	 StartChgResp = rt_timer_create("StartChgResp",  /* ��ʱ�������� StartChgResp */
									StartChgResp_Timeout, /* ��ʱʱ�ص��Ĵ����� */
									RT_NULL, /* ��ʱ��������ڲ��� */
									5000, /* ��ʱ���ȣ���OS TickΪ��λ����5000��OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* һ���Զ�ʱ�� */
	/* ����ͣ���ظ���ʱ�� */
	 StopChgResp = rt_timer_create("StopChgResp",  /* ��ʱ�������� StopChgResp */
									StopChgResp_Timeout, /* ��ʱʱ�ص��Ĵ����� */
									RT_NULL, /* ��ʱ��������ڲ��� */
									5000, /* ��ʱ���ȣ���OS TickΪ��λ����5000��OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* һ���Զ�ʱ�� */
	/* �����������ʻظ���ʱ�� */
	 PowerAdjResp = rt_timer_create("PowerAdjResp",  /* ��ʱ�������� PowerAdjResp */
									PowAdjResp_Timeout, /* ��ʱʱ�ص��Ĵ����� */
									RT_NULL, /* ��ʱ��������ڲ��� */
									5000, /* ��ʱ���ȣ���OS TickΪ��λ����5000��OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* һ���Զ�ʱ�� */
}
/*  */

/********************************************************************  
*	�� �� ��: CtrlData_RecProcess()
*	����˵��: ���������ݽ��մ�����
*	��    ��: ��
*	�� �� ֵ: ��
********************************************************************/ 
static void CtrlData_RecProcess(void)
{
	rt_uint8_t c_rst;
	rt_uint32_t chgplanIssue,chgplanIssueAdj,startchg,stopchg;
	rt_uint32_t EventCmd;
	EventCmd = strategy_event_get();
	
	switch(EventCmd)
	{	
		//�յ������������
		case StartChg_EVENT:
		{
			startchg_flag = TRUE;
			c_rst = CtrlUnit_RecResp(Cmd_StartChg,&Ctrl_Start,0);//ȡֵ
			rt_lprintf("[strategy]  (%s)  �յ������������  \n",__func__);
			memcpy(&CtrlCharge_Event,&Ctrl_Start,41);
			CtrlCharge_Event.CtrlType = CTRL_START;
			
			if(Fault.Total != TRUE)
			{
				if(memcmp(&RouterIfo.AssetNum,&Ctrl_Start.cAssetNO,22) == 0)//У���ʲ�һ����
				{
					ChargepileDataGetSet(Cmd_ChargeStart,0);	
				
					/* ��ʼ�����ظ���ʱ */
					if (StartChgResp != RT_NULL)
						rt_timer_start(StartChgResp);
					else
						rt_lprintf("StartChgResp timer create error\n");							
				}
				else
				{
					Ctrl_Start.cSucIdle = ORTHERS;
					c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);//�ظ�
				}			
			}
			else
			{
				Ctrl_Start.cSucIdle = FAILED;	
				c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);//�ظ�			
			}		
			CtrlCharge_Event.cSucIdle = Ctrl_Start.cSucIdle;			
			break;
		}
		//�յ�ֹͣ�������
		case StopChg_EVENT:
		{
			stopchg_flag = TRUE;	
			c_rst = CtrlUnit_RecResp(Cmd_StopChg,&Ctrl_Stop,0);//ȡֵ			
			rt_lprintf("[strategy]  (%s)  �յ�ֹͣ�������  \n",__func__); 
			memcpy(&CtrlCharge_Event,&Ctrl_Stop,41);			
			CtrlCharge_Event.CtrlType = CTRL_STOP;
			
			if(Fault.Total != TRUE)
			{
				if((memcmp(&RouterIfo.AssetNum,&Ctrl_Stop.cAssetNO,22) == 0)//У���ʲ�һ����
					||(memcmp(&Ctrl_Start.OrderSn,&Ctrl_Stop.OrderSn,16) == 0))//У����ͣ����һ����		
				{
					ChargepileDataGetSet(Cmd_ChargeStop,0);	
				
					/* ��ʼͣ���ظ���ʱ */
					if (StopChgResp != RT_NULL)
						rt_timer_start(StopChgResp);
					else
						rt_lprintf("StopChgResp timer create error\n");
				}
				else
				{
					Ctrl_Stop.cSucIdle = ORTHERS;
					c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);//�ظ�
				}
			}
			else
			{
				Ctrl_Stop.cSucIdle = FAILED;
				c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);//�ظ�
			}
			CtrlCharge_Event.cSucIdle = Ctrl_Stop.cSucIdle;
			break;
		}
		//�յ�������������
		case PowerAdj_EVENT:
		{
			adjpower_flag = TRUE;
			c_rst = CtrlUnit_RecResp(Cmd_PowerAdj,&Ctrl_PowerAdj,0);//ȡֵ	
			rt_lprintf("[strategy]  (%s)  �յ�������������  \n",__func__);  
			memcpy(&CtrlCharge_Event,&Ctrl_PowerAdj,41);
			CtrlCharge_Event.CtrlType = CTRL_ADJPOW;
			
			if(Fault.Total != TRUE)
			{
				if(memcmp(&RouterIfo.AssetNum,&Ctrl_PowerAdj.cAssetNO,22) == 0)//У���ʲ�һ����
				{
					ChargePilePara_Set.PWM_Duty = Ctrl_PowerAdj.SetPower*10/132;//���ʻ���: D(��һλС��)=I/60*1000=P/(60*220)*1000
					ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara_Set);	
					
					/* ��ʼ���ʵ����ظ���ʱ */
					if (PowerAdjResp != RT_NULL)
						rt_timer_start(PowerAdjResp);
					else
						rt_lprintf("[strategy] : PowerAdjResp timer create error\n");
				}
				else
				{
					Ctrl_PowerAdj.cSucIdle = ORTHERS;
					c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);//�ظ�
				}
			}
			else
			{
				Ctrl_PowerAdj.cSucIdle = FAILED;
				c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);//�ظ�
			}
			Ctrl_Stop.cSucIdle = Ctrl_PowerAdj.cSucIdle;
			break;
		}

		default:
			break;
	}
}
/********************************************************************  
*	�� �� ��: PileData_RecProcess()
*	����˵��: ��׮���ݽ��մ�����
*	��    ��: ��
*	�� �� ֵ: ��
********************************************************************/ 
static void PileData_RecProcess(void)
{
	rt_uint8_t c_rst,p_rst;
	rt_uint32_t start_result,stop_result,adjpow_result;
	
	if(startchg_flag == TRUE)
	{
		//�����ɹ�
		if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	
		{
			rt_timer_stop(StartChgResp);
			
			Ctrl_Start.cSucIdle = SUCCESSFUL;
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);
			
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//��λ
				rt_lprintf("[strategy] : start charge successful!\n");
			}
		}
		//����ʧ��
		else if(rt_event_recv(&ChargePileEvent, ChargeStartER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	
		{
			rt_timer_stop(StartChgResp);
			
			Ctrl_Start.cSucIdle = FAILED;
			p_rst = ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara_Get);//��ȡʧ��ԭ��
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//��λ
				rt_lprintf("[strategy] : start charge failed,reason:%d!\n",ChargePilePara_Get.StartReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);		
		}
		rt_lprintf("[strategy] : ChargePileEvent 0x%02X\n", start_result);
	}
	
	if(stopchg_flag == TRUE)
	{
		//ͣ���ɹ�
		if(rt_event_recv(&ChargePileEvent, ChargeStopOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		
		{
			rt_timer_stop(StopChgResp);
			
			Ctrl_Stop.cSucIdle = SUCCESSFUL;
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);
						
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				stopchg_flag = FALSE;//��λ
				rt_lprintf("[strategy] : stop charge successful!\n");
			}
		}
		//ͣ��ʧ��
		else if(rt_event_recv(&ChargePileEvent, ChargeStopER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		
		{
			rt_timer_stop(StopChgResp);
			
			Ctrl_Start.cSucIdle = FAILED;
			p_rst = ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara_Get);//��ȡʧ��ԭ��
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				stopchg_flag = FALSE;//��λ
				rt_lprintf("[strategy] : stop charge failed,reason:%d!\n",ChargePilePara_Get.StopReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);			
		}
		rt_lprintf("[strategy] : ChargePileEvent 0x%02X\n", stop_result);
	}
	
	if(adjpower_flag == TRUE)
	{
		//�������ʳɹ�
		if(rt_event_recv(&ChargePileEvent, SetPowerOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &adjpow_result) == RT_EOK)	
		{
			rt_timer_stop(PowerAdjResp);
			
			Ctrl_PowerAdj.cSucIdle = SUCCESSFUL;
			c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);
			
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				adjpower_flag = FALSE;//��λ
				rt_lprintf("[strategy] : start charge successful!\n");
			}
		}
		//��������ʧ��
		else if(rt_event_recv(&ChargePileEvent, SetPowerER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &adjpow_result) == RT_EOK)	
		{
			rt_timer_stop(PowerAdjResp);
			
			Ctrl_PowerAdj.cSucIdle = FAILED;
			p_rst = ChargepileDataGetSet(Cmd_SetPowerResp,&ChargePilePara_Get);//��ȡʧ��ԭ��
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				adjpower_flag = FALSE;//��λ
//				rt_lprintf("[strategy] : adjust power failed,reason:%d!\n",ChargePilePara_Get.AdjPowerReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);		
		}
		rt_lprintf("[strategy] : chargepile:ChargePileEvent 0x%02X\n", adjpow_result);
	}
}
/********************************************************************  
*	�� �� ��: TimeSolt_PilePowerCtrl()
*	����˵��: ��ʱ�ν��е�׮���ʿ���
*	��    ��: ��
*	�� �� ֵ: ��
********************************************************************/ 
static void TimeSolt_PilePowerCtrl(void)
{
	count = 0;
	memset(&SetPowerFinishFlag,0,50);
	while(1)
	{
		if((Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Year >= System_Time_STR.Year)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Month >= System_Time_STR.Month)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Day >= System_Time_STR.Day)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Hour >= System_Time_STR.Hour)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Minute >= System_Time_STR.Minute)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Second >= System_Time_STR.Second))	//��λ��ǰ�����ƻ�����ʼʱ���
		{
			Chg_ExeState.ucPlanPower = Chg_Strategy.strChargeTimeSolts[count].ulChargePow;
			
			ChargePilePara_Set.PWM_Duty = Chg_ExeState.ucPlanPower*10/132;//���ʻ���
			ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara_Set);
			
			Chg_ExeState.exeState = EXE_ING;
			memcpy(cRequestNO_Old,cRequestNO_New,sizeof(cRequestNO_Old));
			break;
		}
		
		count++;
	}
}



static void energycon_thread_entry(void *parameter)
{
	rt_err_t res;
	rt_err_t ret = RT_EOK;
	rt_uint32_t* r_str;
	unsigned char i = 0;
	
	rt_pin_mode(RELAYA_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAYB_PIN, PIN_MODE_OUTPUT);
	RELAY_ON();//�ϵ����ϼ̵���
	
	rt_thread_mdelay(100);
	
	while (1)
	{
//		if((res = rt_sem_take(&rx_sem_energycon, 1000)) == RT_EOK)
//		{	
//			memcpy(cRequestNO_New,Chg_Strategy.cRequestNO,sizeof(cRequestNO_New));		
//		}	
		PileData_RecProcess();	
		CtrlData_RecProcess();
		
		if(memcmp(cRequestNO_Old,cRequestNO_New,sizeof(cRequestNO_Old)) != 0)
		{
			TimeSolt_PilePowerCtrl(); 
		}
		
		for(i=count;i<50;i++)
		{
			if((Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Year >= System_Time_STR.Year)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Month >= System_Time_STR.Month)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Day >= System_Time_STR.Day)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Hour >= System_Time_STR.Hour)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Minute >= System_Time_STR.Minute)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Second >= System_Time_STR.Second))	//��λ��ǰ�����ƻ�����ʼʱ���
			{
				ChargePilePara_Set.PWM_Duty = Chg_Strategy.strChargeTimeSolts[i].ulChargePow;
				if(SetPowerFinishFlag[i] == 0)//���Ʒ���һ��
				{
					ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara_Set);
					SetPowerFinishFlag[i] = 1;
				}
				break;
			}
		}
		
		rt_thread_mdelay(1000);
	}
}

int energycon_thread_init(void)
{
	rt_err_t res;
	
	/* ��ʼ����ʱ�� */
    timer_create_init();
	
	/*��ʼ���ź���*/
	rt_sem_init(&rx_sem_energycon, "rx_sem_energycon", 0, RT_IPC_FLAG_FIFO);
	
	res=rt_thread_init(&energycon,
											"energycon",
											energycon_thread_entry,
											RT_NULL,
											energycon_stack,
											THREAD_ENERGYCON_STACK_SIZE,
											THREAD_ENERGYCON_PRIORITY,
											THREAD_ENERGYCON_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&energycon);
	}
	return res;
}


#if defined (RT_ENERGYCON_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(energycon_thread_init);
#endif
MSH_CMD_EXPORT(energycon_thread_init, energycon thread run);





