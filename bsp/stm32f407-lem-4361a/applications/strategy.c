#include <rtthread.h>
#include <rtdevice.h>
#include "strategy.h"
#include "chargepile.h"
#include "698.h"
#include "meter.h"
#include "energycon.h"
#include <board.h>

#ifndef FALSE
#define FALSE         0
#endif
#ifndef TRUE
#define TRUE          1
#endif
#define THREAD_STRATEGY_PRIORITY     8
#define THREAD_STRATEGY_STACK_SIZE   1024
#define THREAD_STRATEGY_TIMESLICE    5

#ifndef SUCCESSFUL
#define SUCCESSFUL    0
#endif
#ifndef FAILED
#define FAILED        1
#endif

#define RELAYA_PIN    GET_PIN(F, 2)
#define RELAYB_PIN    GET_PIN(F, 3)


rt_uint8_t DeviceState;
rt_uint8_t ChgpileState;
rt_bool_t DeviceFauFlag;
///////////////////////////////////////////////////////////////////
//�������ԭ��
static char *err_strfault[] = 
{
           "                       ",            /* ERR_OK          0  */
           "�ն������ڴ���ϣ�   ",               /* ERR             1  */
           "ʱ�ӹ��ϣ�         ",                 /* ERR             2  */
           "����ͨ�Ź��ϣ�	       ",            /* ERR             3  */
           "485������ϣ�         ",              /* ERR             4  */
           "��ʾ����ϣ�         ",               /* ERR             5  */
           "�ز�ͨ���쳣��       ",               /* ERR             6  */
           "NandFLASH��ʼ������       ",        /* ERR             7  */
           "ESAM����         ",                 /* ERR             8  */
           "����ģ����ϣ�         ",             /* ERR             9  */
           "��Դģ����ϣ�         ",             /* ERR             10 */
           "���׮ͨ�Ź��ϣ�        ",            /* ERR             11 */
           "���׮�豸���ϣ�         ",           /* ERR             12 */
	       "���ض�����¼����       ",             /* ERR             13 */
};




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

//extern struct rt_event PowerCtrlEvent;
//extern struct rt_event HplcEvent;



//��ʱ���
static rt_timer_t StartChgResp;
static rt_timer_t StopChgResp;
static rt_timer_t AdjustPowerResp;
	
static rt_uint8_t strategy_stack[THREAD_STRATEGY_STACK_SIZE];//�̶߳�ջ
static struct rt_thread strategy;

ChargPilePara_TypeDef ChargePilePara;
CHARGE_STRATEGY Chg_Strategy;
CHARGE_STRATEGY_RSP Chg_StrategyRsp;
CHARGE_STRATEGY Adj_Chg_Strategy;
CHARGE_STRATEGY_RSP Adj_Chg_StrategyRsp;
CHARGE_EXE_STATE Chg_ExeState;
CTL_CHARGE Ctrl_StartRsp;
CTL_CHARGE Ctrl_StopRsp;


unsigned char PileWorkState;
rt_bool_t startchg_flag;
rt_bool_t stopchg_flag;
/**************************************************************
 * ��������: StartChgResp_Timeout 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: ������糬ʱ����
 **************************************************************/
static void StartChgResp_Timeout(void *parameter)
{
    rt_lprintf("StartChgResp event is timeout!\n");
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
    rt_lprintf("StopChgResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_ChargeStopResp,0);
}
/**************************************************************
 * ��������: timer_create_init 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: ��ʱ��
 **************************************************************/
void timer_create_init()
{
    /* ���������ظ���ʱ�� */
	 StartChgResp = rt_timer_create("StartChgResp",  /* ��ʱ�������� StartChgResp */
									StartChgResp_Timeout, /* ��ʱʱ�ص��Ĵ����� */
									RT_NULL, /* ��ʱ��������ڲ��� */
									5000, /* ��ʱ���ȣ���OS TickΪ��λ����5000��OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* һ���Զ�ʱ�� */
	/* ����ͣ���ظ���ʱ�� */
	 StopChgResp = rt_timer_create("StopChgResp",  /* ��ʱ�������� StartChgResp */
									StopChgResp_Timeout, /* ��ʱʱ�ص��Ĵ����� */
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

	
	//�յ����ƻ�
//	if(rt_event_recv(&PowerCtrlEvent, ChgPlanIssue_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &chgplanIssue) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanIssue,&Chg_Strategy,0);//ȡֵ
		if((Chg_Strategy.ucChargeMode == 1)&&(Chg_Strategy.ucDecType == 1))
			rt_sem_release(&rx_sem_energycon);
		
		memcpy(&Chg_StrategyRsp,&Chg_Strategy,40);
		Chg_StrategyRsp.cSucIdle = 0;
		
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanIssueAck,&Chg_StrategyRsp,0);//�ظ�		
	}
	
	//�յ����ƻ�����
//	if(rt_event_recv(&PowerCtrlEvent, ChgPlanAdjust_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &chgplanIssueAdj) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanAdjust,&Adj_Chg_Strategy,0);//ȡֵ	
		if((Chg_Strategy.ucChargeMode == 1)&&(Chg_Strategy.ucDecType == 2))
			rt_sem_release(&rx_sem_energycon);
		
		memcpy(&Adj_Chg_StrategyRsp,&Adj_Chg_Strategy,40);
		Chg_StrategyRsp.cSucIdle = 0;
		
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanAdjustAck,&Adj_Chg_StrategyRsp,0);//�ظ�		
	}
	
	//�յ������������
//	if(rt_event_recv(&PowerCtrlEvent, StartChg_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &startchg) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		startchg_flag = TRUE;
		rt_lprintf("[hplc]  (%s)  �յ������������  \n",__func__);
		
		if(DeviceFauFlag != TRUE)
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
			Ctrl_StartRsp.cSucIdle = FAILED;
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_StartRsp,0);//�ظ�	
		}		   	
	}
	//�յ�ֹͣ�������
//	if(rt_event_recv(&PowerCtrlEvent, StopChg_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stopchg) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		stopchg_flag = TRUE;		
		rt_lprintf("[hplc]  (%s)  �յ�ֹͣ�������  \n",__func__);  
			
		if(DeviceFauFlag != TRUE)
		{
			ChargepileDataGetSet(Cmd_ChargeStop,0);	
			
			/* ��ʼͣ���ظ���ʱ */
			if (StopChgResp != RT_NULL)
				rt_timer_start(StopChgResp);
			else
				rt_lprintf("StartChgResp timer create error\n");
		}
		else
		{
			Ctrl_StopRsp.cSucIdle = FAILED;
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_StopRsp,0);//�ظ�
		}
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
	rt_uint32_t start_result,stop_result;
	
	if(startchg_flag == TRUE)
	{
		if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	//�����ɹ�
		{
			rt_timer_stop(StartChgResp);
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,0,0);
			
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//��λ
				rt_lprintf("start charge successful!\n");
			}
			
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", start_result);	
		}
		else if(rt_event_recv(&ChargePileEvent, ChargeStartER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	//����ʧ��
		{
			rt_timer_stop(StartChgResp);
			p_rst = ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara);//��ȡʧ��ԭ��
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//��λ
				rt_lprintf("start charge failed,reason:%d!\n",ChargePilePara.StartReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Chg_Strategy,0);
			
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", start_result);
		}
	}
	
	if(stopchg_flag == TRUE)
	{
		if(rt_event_recv(&ChargePileEvent, ChargeStopOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		//ͣ���ɹ�
		{
			rt_timer_stop(StopChgResp);
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&ChargePilePara,0);
			stopchg_flag = FALSE;
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", stop_result);	
		}
		else if(rt_event_recv(&ChargePileEvent, ChargeStartER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		//ͣ��ʧ��
		{
			ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara);//��ȡʧ��ԭ��
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,0,0);
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", stop_result);
		}
	}
}

/********************************************************************  
*	�� �� ��: DevState_Judge()
*	����˵��: ·����״̬�ж�
*	��    ��: ��
*	�� �� ֵ: ���ϴ���
********************************************************************/ 
rt_err_t DevState_Judge(void)
{
	rt_err_t fau;
	fau = 0;
	
	if((System_Time_STR.Year > 50)||(System_Time_STR.Month > 12)||(System_Time_STR.Day > 31)
		||(System_Time_STR.Hour > 23)||(System_Time_STR.Minute > 60))
	{
		fau |= (1<<(Clock_FAULT-1));				
		rt_lprintf("%s\r\n",(const char*)err_strfault[Clock_FAULT]);
	}
	
//	if(rt_device_find("lcd"))
//	{
//		fau |= (1<<(Screen_FAULT-1));				
//		rt_lprintf("%s\r\n",(const char*)err_strfault[Screen_FAULT]);
//	}
	
//	if(rt_sem_trytake(&rt_sem_meterfau) == RT_EOK)
//	{
//		fau |= (1<<(MeterCom_FAULT-1));
//		rt_lprintf("%s\r\n",(const char*)err_strfault[MeterCom_FAULT]);
//	}
//	
//	if(rt_sem_trytake(&rt_sem_nandfau) == RT_EOK)
//	{
//		fau |= (1<<(NandF_FAULT-1));		
//		rt_lprintf("%s\r\n",(const char*)err_strfault[NandF_FAULT]);
//	}
	
//	if(rt_sem_trytake(&rt_sem_bluetoothfau) == RT_EOK)
//	{
//		fau |= (1<<(Bluetooth_FAULT-1));
//		rt_lprintf("%s\r\n",(const char*)err_strfault[Bluetooth_FAULT]);
//	}
		
	return fau;
	
}

static void strategy_thread_entry(void *parameter)
{
	rt_err_t res,fau;
	
	rt_pin_mode(RELAYA_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAYB_PIN, PIN_MODE_OUTPUT);
	RELAY_ON();//�ϵ����ϼ̵���
	
	DeviceFauFlag = FALSE;
	rt_thread_mdelay(100);
	
	while (1)
	{
		fau = DevState_Judge();
		if(fau != 0)
		{
			DeviceFauFlag = TRUE;
		}
		else
		{
			DeviceFauFlag = FALSE;
		}
		
		PileData_RecProcess();
		
		CtrlData_RecProcess();
		
		
		
		
						
		rt_thread_mdelay(1000);
	}
}

int strategy_thread_init(void)
{
	rt_err_t res;
	
	/* ��ʼ����ʱ�� */
    timer_create_init();
	
	res=rt_thread_init(&strategy,
											"strategy",
											strategy_thread_entry,
											RT_NULL,
											strategy_stack,
											THREAD_STRATEGY_STACK_SIZE,
											THREAD_STRATEGY_PRIORITY,
											THREAD_STRATEGY_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&strategy);
	}
	return res;
}


#if defined (RT_STRATEGY_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(strategy_thread_init);
#endif
MSH_CMD_EXPORT(strategy_thread_init, strategy thread run);



