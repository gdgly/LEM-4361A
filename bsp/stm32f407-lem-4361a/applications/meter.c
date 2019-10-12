#include <rtthread.h>
#include <rtdevice.h>
#include <meter.h>
#include <string.h>
#include <stdio.h>
#include <global.h>
#include <storage.h>
#include <analog.h>

//#define DEVICE_NAME			"uart5"//����5�豸�� RT_METER_USART

/* ���ڽ����¼���־*/
#define UART_RX_EVENT (1 << 5)

#define THREAD_METER_PRIORITY     25
#define THREAD_METER_STACK_SIZE   2048
#define THREAD_METER_TIMESLICE    5

//rt_thread_t meter_sig;					//meter thread
//rt_uint8_t meter_signal_flag;

static rt_device_t meter_serial;				//meter device

static struct rt_thread meter;					//meter thread
static struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT; /* ����Ĭ�����ò���*/

static rt_uint8_t meter_stack[THREAD_METER_STACK_SIZE];//�̶߳�ջ

static struct rt_event meter_uartevent;//���ڽ������ݵ��¼�

//static ScmStorage_Msg stStorage_msg;

CCMRAM static ScmUart_Comm stMeter;//meter �������ݽṹ��
static ScmMeter_PriceModle stMeter_PriceModle;//�Ʒ�ģ��
static ScmMeter_Analog stMeter_Analog;//meter ģ�������ݽṹ��
static ScmMeter_HisData stMeter_HisData;//meter��ʷ���ݻ���ṹ��
static rt_uint32_t ulMeter_Half[48];		//ÿ��ÿ���Сʱ�ĵ���

//static ScmMeter_Power	ulMeter_Day;
static ScmMeter_Power	ulMeter_MonthOld;
//static ScmMeter_Power	ulMeter_Month;

static rt_uint32_t s_ulevmin_start;//ÿ���Сʱ��ʼ�ĵ���ֵ
static rt_uint8_t s_ucMeter_TotalStart;//ÿ���Сʱ��¼��ʼ�����ı�־
static rt_uint8_t s_ucMeter_ClearHalf;//ÿ���Сʱ��¼��ʼ�����ı�־
static rt_uint8_t s_ucMeter_ClearMonth;

rt_uint8_t g_ucMeter_rxbuf[100];//���ڽ��ջ���
rt_uint8_t g_ucMeter_rxlenth;//�������ݳ���

static void cmMeter_electricity_calc(void);//ͳ����ʷ������Ϣ
static void cmMeter_GJFModeInit(ScmMeter_PriceModle *pSTR);

//unsigned char cmMeterStateUpdate(ScmUart_Comm *pSTR);
static void cmMeterModbus_RecProtocal(ScmUart_Comm *pSTR);

static Power_Analog_TypeDef	ScmAnalog;

static rt_uint8_t save_meter_flag;


//static ScmMeter_Analog get_Analog;// ������

static rt_err_t meter_rx_ind(rt_device_t dev, rt_size_t size)
{
	g_ucMeter_rxlenth = size;//�������ݳ���
	rt_event_send(&meter_uartevent, UART_RX_EVENT);//�������ݽ����¼�
  return RT_EOK;
}

static void save_meter_data(Power_Analog_TypeDef* analog)//ϵͳ���� ���������Ϣ
{
	rt_uint32_t l_ulmeter_time;
	
	get_analog_data(analog);
	
	if((analog->Pow_5V < 4000)&&(!save_meter_flag))//ϵͳ���� �����źŸ�meter �洢����
	{
		l_ulmeter_time= 0x20;
		l_ulmeter_time = l_ulmeter_time<<8|System_Time_STR.Year;
		l_ulmeter_time = l_ulmeter_time<<8|System_Time_STR.Month;
		l_ulmeter_time = l_ulmeter_time<<8|System_Time_STR.Day;
		SetStorageData(Cmd_MeterPowerWr,&stMeter_HisData,l_ulmeter_time);//���������Ϣ
	
	SetStorageData(Cmd_MeterHalfPowerWr,&ulMeter_Half,l_ulmeter_time);//���������Ϣ
	}
	else if(analog->Pow_5V > 4500)//ϵͳ�ָ�  �����־
	{
		save_meter_flag = 0;
	}
}



static void meter_thread_entry(void *parameter)//meter �߳�
{
	rt_err_t res;
	rt_uint8_t l_uchour;
	rt_uint32_t e;
	int ret;
	rt_uint8_t buf[8]={0x01,0x03,0x00,0x48,0x00,0x08,0xC4,0x1A};
	
	meter_serial = rt_device_find(RT_METER_USART);//�����豸
	
	if(meter_serial != RT_NULL)
	{
		config.baud_rate = BAUD_RATE_4800;
		config.data_bits = DATA_BITS_8;
		config.stop_bits = STOP_BITS_2;
		config.parity = PARITY_NONE;
		rt_device_control(meter_serial, RT_DEVICE_CTRL_CONFIG, &config);//���ô��ڲ���
		if (rt_device_open(meter_serial, RT_DEVICE_FLAG_DMA_RX) == RT_EOK)//���豸
		{
			rt_lprintf("[Meter]:Open serial %s sucess!\r\n",RT_METER_USART);
		}
	}
	else
	{
		res = RT_ERROR;
		rt_lprintf("[Meter]:Open serial %s err!\r\n",RT_METER_USART);
		return;
	}
	
	rt_event_init(&meter_uartevent, "uart_rx_event", RT_IPC_FLAG_FIFO);//��ʼ���¼�����
	
	rt_device_set_rx_indicate(meter_serial, meter_rx_ind);//���ô��ڽ��ջص�����
	
	memset(&stMeter_Analog,0,sizeof(ScmMeter_Analog));
	memset(&stMeter_HisData,0,sizeof(ScmMeter_HisData));
	memset(&ulMeter_Half,0,sizeof(ulMeter_Half));
	
	
//	rt_signal_install(SIGUSR1, meter_sig_handle);
//  rt_signal_unmask(SIGUSR1);
	
	rt_thread_mdelay(5000);
	
	ret = GetStorageData(Cmd_MeterGJFModeRd,&stMeter_PriceModle,sizeof(stMeter_PriceModle));//��ȡ�Ʒ�ģ��
	if(ret < 0)
	{
		cmMeter_GJFModeInit(&stMeter_PriceModle);
		SetStorageData(Cmd_MeterGJFModeWr,&stMeter_PriceModle,sizeof(stMeter_PriceModle));
		rt_lprintf("[Meter]:JFmode is not exist,clear stMeter_PriceModle!\r\n");
	}
	
	BCD_toInt(&l_uchour,&System_Time_STR.Hour,1);
	
	if(l_uchour<24)
	{
		ret = GetStorageData(Cmd_MeterHalfPowerRd,&ulMeter_Half,sizeof(ulMeter_Half));//��ȡ��Сʱ����
		if(ret < 0)
		{
			SetStorageData(Cmd_MeterHalfPowerWr,&ulMeter_Half,sizeof(ulMeter_Half));
			rt_lprintf("[Meter]:half meter is not exist,clear half meter!\r\n");
		}
		s_ulevmin_start = stMeter_Analog.ulMeterTotal - ulMeter_Half[l_uchour*2];//����Сʱ�ڴ������� ���ȥ��Сʱ�ڴ洢�ĵ��� ��Ϊ��ʱ�ε���ʼֵ
	}
	
	while (1)
	{
		rt_device_write(meter_serial, 0, &buf, sizeof(buf));
		
		res = rt_event_recv(&meter_uartevent, UART_RX_EVENT, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 1000, &e);
		if(res== RT_EOK)
		{		
			if(rt_device_read(meter_serial, 0, g_ucMeter_rxbuf, g_ucMeter_rxlenth) != 1)
			{		
				cmMeterModbus_RecProtocal(&stMeter);
				my_printf((char*)stMeter.Rx_data,stMeter.DataRx_len,MY_HEX,0,"[Meter]:RX:");				
			}
		}
		else
		{
				rt_lprintf("[Meter]:meter_serial sem timeout. reply:%s\r\n", res);
		}
		cmMeter_electricity_calc();//ÿ��ÿ�ռ��ƽ�ȵ�������
		save_meter_data(&ScmAnalog);
		
		
//		cmMeter_get_data(EMMETER_ANALOG,&get_Analog);
//		
		rt_thread_mdelay(2000);
	}
}


/*
*********************************************************************************************************
*	�� �� ��: MeterModbusStateUpdate
*	����˵��: Modbus��Լ��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
unsigned char cmMeterStateUpdate(ScmUart_Comm *pSTR)
{
	rt_uint32_t temp = 0;
	rt_uint8_t tmp_arry[4];
	rt_uint16_t Crc16;
	
	Crc16 = (unsigned int)CRC_16(&pSTR->Rx_data[0],pSTR->DataRx_len-2);
	if(((Crc16&0xff)==pSTR->Rx_data[pSTR->DataRx_len-2])&&(((Crc16>>8)&0xff)==pSTR->Rx_data[pSTR->DataRx_len-1]))
	{		
		tmp_arry[3] = pSTR->Rx_data[3];
		tmp_arry[2] = pSTR->Rx_data[4];
		tmp_arry[1] = pSTR->Rx_data[5];
		tmp_arry[0] = pSTR->Rx_data[6];	
		memcpy(&temp,tmp_arry,sizeof(char)*4);
		stMeter_Analog.ulVol = (unsigned long)(temp/1000);//XXXXXXX.X ������ѹ
				
		tmp_arry[3] = pSTR->Rx_data[7];
		tmp_arry[2] = pSTR->Rx_data[8];
		tmp_arry[1] = pSTR->Rx_data[9];
		tmp_arry[0] = pSTR->Rx_data[10];
		memcpy(&temp,tmp_arry,sizeof(char)*4);
		stMeter_Analog.ulCur = (unsigned long)(temp/100);//XXXX.XX ��������

		tmp_arry[3] = pSTR->Rx_data[11];
		tmp_arry[2] = pSTR->Rx_data[12];
		tmp_arry[1] = pSTR->Rx_data[13];
		tmp_arry[0] = pSTR->Rx_data[14];
		memcpy(&temp,tmp_arry,sizeof(char)*4);
		stMeter_Analog.ulAcPwr = (unsigned long)(temp/10);    // XXXXXX.XXX	�й�����
									
		tmp_arry[3] = pSTR->Rx_data[15];
		tmp_arry[2] = pSTR->Rx_data[16];
		tmp_arry[1] = pSTR->Rx_data[17];
		tmp_arry[0] = pSTR->Rx_data[18];
		memcpy(&temp,tmp_arry,sizeof(char)*4);
		stMeter_Analog.ulMeterTotal = (unsigned long)(temp/10);    // XXXXXX.XXX		�й��ܵ��
		
		
		tmp_arry[3] = pSTR->Rx_data[19];
		tmp_arry[2] = pSTR->Rx_data[20];
		tmp_arry[1] = pSTR->Rx_data[21];
		tmp_arry[0] = pSTR->Rx_data[22];
		memcpy(&temp,tmp_arry,sizeof(char)*4);
		stMeter_Analog.ulPwrFactor = (unsigned long)(temp/10);    // XXXXXX.XX		��������
		
		tmp_arry[3] = pSTR->Rx_data[31];
		tmp_arry[2] = pSTR->Rx_data[32];
		tmp_arry[1] = pSTR->Rx_data[33];
		tmp_arry[0] = pSTR->Rx_data[34];
		memcpy(&temp,tmp_arry,sizeof(char)*4);
		stMeter_Analog.ulFrequency = (unsigned long)(temp);    // XXXXXX.XX		Ƶ��
		
		rt_lprintf("[Meter]:ChargVa = %d.%dV----",stMeter_Analog.ulVol/10,stMeter_Analog.ulVol%10);
		rt_lprintf("ChargIa = %d.%02dA----",stMeter_Analog.ulCur/100,stMeter_Analog.ulCur%100);
		rt_lprintf("MeterTotal = %d.%03dKWH----\r\n",stMeter_Analog.ulMeterTotal/1000,stMeter_Analog.ulMeterTotal%1000);
		
		rt_lprintf("[Meter]:AcPwr = %d.%03dW----",stMeter_Analog.ulAcPwr/1000,stMeter_Analog.ulAcPwr%1000);
		rt_lprintf("PwrFactor = %d.%02d ----",stMeter_Analog.ulPwrFactor/100,stMeter_Analog.ulPwrFactor%100);
		rt_lprintf("Frequency = %d.%02dHZ----\r\n",stMeter_Analog.ulFrequency/100,stMeter_Analog.ulFrequency%100);
				
		return 0;
	}
	else
	{

		rt_lprintf("[Meter]:CRC16 ERR!!! CRC16 = %04X  Rec_crc=%02X%02X\r\n",Crc16,pSTR->Rx_data[pSTR->DataRx_len-2],pSTR->Rx_data[pSTR->DataRx_len-1]);
		return 1;
	}
}
/********************************************************************  
*	�� �� ��: MeterModbus_RecProtocal
*	����˵��: Modbus�ײ����ݽ��պ���
*	��    ��: ��
*	�� �� ֵ: ��
********************************************************************/ 

void cmMeterModbus_RecProtocal(ScmUart_Comm *pSTR)
{   
	rt_uint8_t lenth,i,rx_ptr;
	
	rx_ptr = 0;
	for(i=0;i<g_ucMeter_rxlenth;i++)
	{
		if((g_ucMeter_rxbuf[rx_ptr] == 0x01)&&(g_ucMeter_rxbuf[rx_ptr+1] == 0x03)&&(g_ucMeter_rxbuf[rx_ptr+2] == 0x20)&&(rx_ptr<98))
		{
			lenth = g_ucMeter_rxbuf[rx_ptr+2]+5;
			pSTR->DataRx_len = lenth;
			memcpy(pSTR->Rx_data,&g_ucMeter_rxbuf[rx_ptr],lenth);
			cmMeterStateUpdate(pSTR);
			break;
		}
		else
		{
			rx_ptr++;
			if(rx_ptr>97)
			{
				rx_ptr = 0;
				break;
			}
		}
	}		
}

static void cmMeter_GJFModeInit(ScmMeter_PriceModle *pSTR)//ɽ�������Ʒ���Ϣ
{
	rt_uint8_t i;
	
	memset(&pSTR->uiJFmodID,0,8);
	pSTR->EffectiveTime.Year = 0x19;
	pSTR->EffectiveTime.Month = 0x01;
	pSTR->EffectiveTime.Day = 0x01;
	pSTR->EffectiveTime.Hour = 0x00;
	pSTR->EffectiveTime.Minute = 0x00;
	pSTR->EffectiveTime.Second = 0x00;
	
	pSTR->unEffectiveTime.Year = 0x99;
	pSTR->unEffectiveTime.Month = 0x12;
	pSTR->unEffectiveTime.Day = 0x31;
	pSTR->unEffectiveTime.Hour = 0x00;
	pSTR->unEffectiveTime.Minute = 0x00;
	pSTR->unEffectiveTime.Second = 0x00;
	
	pSTR->state = 0x01;
	pSTR->style = 0x01;
	pSTR->count = 4;
	pSTR->ulPriceNo[0] = 10947;
	pSTR->ulPriceNo[1] = 9693;
	pSTR->ulPriceNo[2] = 6559;
	pSTR->ulPriceNo[3] = 3425;
	
	for(i = 0;i < 2;i++)//10:30~11:30 �� 21-22
		pSTR->ulTimeNo[21+i] = 0x02;
		
	for(i = 0;i < 4;i++)//19:00~21:00 �� 38-41
		pSTR->ulTimeNo[38+i] = 0x02;
	
	for(i = 0;i < 4;i++)//8:30~10:30 �� 17-20
		pSTR->ulTimeNo[17+i] = 0x02;
		
	for(i = 0;i < 6;i++)//16:00~19:00 �� 32-37
		pSTR->ulTimeNo[32+i] = 0x02;
	
	
	for(i = 0;i < 3;i++)//7:00~8:30 ƽ 14-16
		pSTR->ulTimeNo[14+i] = 0x02;
		
	for(i = 0;i < 9;i++)//11:30~16:00 ƽ 23-31
		pSTR->ulTimeNo[23+i] = 0x02;
		
	for(i = 0;i < 4;i++)//21:00~23:00 ƽ 42-45
		pSTR->ulTimeNo[42+i] = 0x02;
	
	for(i = 0;i < 14;i++)//23:00~7:00 �� 0-13
		pSTR->ulTimeNo[i] = 0x03;
		
	pSTR->ulTimeNo[46] = 0x03;//23:00~7:00 ��
	pSTR->ulTimeNo[47] = 0x03;
}

static void cmMeter_electricity_calc(void)
{
	rt_uint8_t i;
	rt_uint8_t l_ucmonth,l_ucDay,l_uchour,l_ucminute;
	rt_uint32_t l_ulmeter_day_sum[4],l_ulmeter_month_sum[4];
	rt_uint32_t l_ulmeter_time;
	
	memset(l_ulmeter_day_sum,0,sizeof(l_ulmeter_day_sum));
	memset(l_ulmeter_month_sum,0,sizeof(l_ulmeter_month_sum));
	
	BCD_toInt(&l_ucmonth,&System_Time_STR.Month,1);
	BCD_toInt(&l_ucDay,&System_Time_STR.Day,1);
	BCD_toInt(&l_uchour,&System_Time_STR.Hour,1);
	BCD_toInt(&l_ucminute,&System_Time_STR.Minute,1);
	
	if((l_ucmonth>12)||(l_ucmonth<1)||(l_ucDay>31)||(l_ucDay<1)||(l_uchour>24)||(l_ucminute>60))//ʱ��Ϸ���ʼ�������
	{
		rt_lprintf("[Meter]:meter electricity calc time error!!!\r\n");//ʱ��μ������  ����
		return;
	}
	
	l_ulmeter_time= 0x20;
	l_ulmeter_time = l_ulmeter_time<<8|System_Time_STR.Year;
	l_ulmeter_time = l_ulmeter_time<<8|System_Time_STR.Month;
	l_ulmeter_time = l_ulmeter_time<<8|System_Time_STR.Day;
	
	if((!l_uchour)&&(!s_ucMeter_ClearHalf))//�ճ����������Ϣ  �����յ�����Ϣ
	{
		s_ucMeter_ClearHalf = 1;
		memset(ulMeter_Half,0,sizeof(ulMeter_Half));
		memcpy(&ulMeter_MonthOld,&stMeter_HisData.ulMeter_Month,sizeof(ScmMeter_Power));
		
		SetStorageData(Cmd_MeterPowerWr,&stMeter_HisData,l_ulmeter_time);//���������Ϣ

		rt_lprintf("[Meter]:new day start,save day meter to nandflash!\r\n");
	}
	else if(l_uchour)
	{
		s_ucMeter_ClearHalf = 0;
	}
	
	if((!(l_ucDay-1))&&(!s_ucMeter_ClearMonth))//�³� �������µ�����Ϣ ������� ����ͳ��
	{
		SetStorageData(Cmd_MeterPowerWr,&stMeter_HisData,l_ulmeter_time);//���������Ϣ
		
		s_ucMeter_ClearMonth = 1;
		memset(&ulMeter_MonthOld,0,sizeof(ScmMeter_Power));
		memset(&stMeter_HisData.ulMeter_Month,0,sizeof(ScmMeter_Power));

		rt_lprintf("[Meter]:new month start,save month meter to nandflash!\r\n");
	}
	else if(l_ucDay-1)
	{
		s_ucMeter_ClearMonth = 0;
	}

	if(l_ucminute%30)
	{
		s_ucMeter_TotalStart = 0;
		ulMeter_Half[2*l_uchour+l_ucminute/30] = stMeter_Analog.ulMeterTotal-s_ulevmin_start;
	}
	else
	{
		if(!s_ucMeter_TotalStart)
		{
			s_ucMeter_TotalStart = 1;
			s_ulevmin_start = stMeter_Analog.ulMeterTotal;
		}		
	}
	for(i=0;i<48;i++)
	{
		l_ulmeter_day_sum[stMeter_PriceModle.ulTimeNo[i]] +=ulMeter_Half[i];	//ʵʱ���㵱�ռ��ƽ���ܵ���
	}
	stMeter_HisData.ulMeter_Day.ulPowerJ = l_ulmeter_day_sum[0];//ĳ���ܼ����
	stMeter_HisData.ulMeter_Day.ulPowerF = l_ulmeter_day_sum[1];//ĳ���ܷ����
	stMeter_HisData.ulMeter_Day.ulPowerP = l_ulmeter_day_sum[2];//ĳ����ƽ����
	stMeter_HisData.ulMeter_Day.ulPowerG = l_ulmeter_day_sum[3];//ĳ���ܹȵ���
	
	stMeter_HisData.ulMeter_Day.ulPowerT = stMeter_HisData.ulMeter_Day.ulPowerJ+stMeter_HisData.ulMeter_Day.ulPowerF\
																				+stMeter_HisData.ulMeter_Day.ulPowerP+stMeter_HisData.ulMeter_Day.ulPowerG;//ĳ���ܵ���
		
	stMeter_HisData.ulMeter_Month.ulPowerJ = ulMeter_MonthOld.ulPowerJ+stMeter_HisData.ulMeter_Day.ulPowerJ;//�¼���
	stMeter_HisData.ulMeter_Month.ulPowerJ = ulMeter_MonthOld.ulPowerF+stMeter_HisData.ulMeter_Day.ulPowerF;//�·���
	stMeter_HisData.ulMeter_Month.ulPowerJ = ulMeter_MonthOld.ulPowerP+stMeter_HisData.ulMeter_Day.ulPowerP;//��ƽ��
	stMeter_HisData.ulMeter_Month.ulPowerJ = ulMeter_MonthOld.ulPowerG+stMeter_HisData.ulMeter_Day.ulPowerG;//�¹���
	
	stMeter_HisData.ulMeter_Month.ulPowerT = stMeter_HisData.ulMeter_Month.ulPowerJ+stMeter_HisData.ulMeter_Month.ulPowerF\
																					+stMeter_HisData.ulMeter_Month.ulPowerP+stMeter_HisData.ulMeter_Month.ulPowerG;//����
}

void cmMeter_get_data(unsigned char cmd,void* str_data)//�����̵߳��ú��� ��ȡMeter������Ϣ
{
	switch(cmd)
	{
		case EMMETER_ANALOG:
			memcpy(&((ScmMeter_Analog*)str_data)->ulVol,&stMeter_Analog.ulVol,sizeof(ScmMeter_Analog));
		break;
		case EMMETER_HISDATA:
//			memcpy(&((ScmMeter_HisData*)str_data)->ulMeter_Total,&stMeter_HisData.ulMeter_Total,sizeof(ScmMeter_HisData));
		break;
		default:
			break;
	}
}


int meter_thread_init(void)
{
	rt_err_t res;
	
	res=rt_thread_init(&meter,
											"meter",
											meter_thread_entry,
											RT_NULL,
											meter_stack,
											THREAD_METER_STACK_SIZE,
											THREAD_METER_PRIORITY,
											THREAD_METER_TIMESLICE);
	if (res == RT_EOK) 
	{
//			meter_sig = &meter;
			rt_thread_startup(&meter);
	}
	return res;
}


#if defined (RT_METER_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(meter_thread_init);
#endif
MSH_CMD_EXPORT(meter_thread_init, meter thread run);


