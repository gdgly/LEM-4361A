#ifndef __METER_H__
#define __METER_H__

//#include "monitor.h"
#include "global.h"


//typedef struct{
//	unsigned char DataRx_len;
//	unsigned char Rx_data[100];
//	unsigned char DataTx_len;
//	unsigned char Tx_data[100];
//	
//	void (*SendBefor)(void); 	  /* ��ʼ����֮ǰ�Ļص�����ָ�루��Ҫ����RS485�л�������ģʽ�� */
//	unsigned char (*SendProtocal)(void); /* ������ϵĻص�����ָ�루��Ҫ����RS485������ģʽ�л�Ϊ����ģʽ�� */
//	void (*RecProtocal)(void);	/* �����յ����ݵĻص�����ָ�� */
//}ScmMeter;

//�Ʒ�ģ��
typedef struct 
{
	unsigned char	uiJFmodID[8];

	STR_SYSTEM_TIME  EffectiveTime;	     // ��Чʱ��
	STR_SYSTEM_TIME  unEffectiveTime;	  // ʧЧʱ��
	unsigned char state;               //ִ��״̬
	unsigned char	style;				 				//��������
	unsigned char ulTimeNo[48];		     //48�����ʺ� 0:00~24:00
	unsigned char count;			         //��Ч������
	unsigned long ulPriceNo[48];       // 48�����
}ScmMeter_PriceModle;

//��ȱ�ģ����
typedef struct 
{
	unsigned long ulVol;         // ������ѹ
	unsigned long ulCur;         // ��������
	unsigned long ulAcPwr;         // ˲ʱ�й�����
	unsigned long ulMeterTotal;     //�й��ܵ���
	unsigned long ulPwrFactor;     //��������
	unsigned long ulFrequency;     //Ƶ��
	
}ScmMeter_Analog;

typedef struct //�����ṹ��
{
	unsigned long ulPowerT;         // �ܵ���
	unsigned long ulPowerJ;         // ��
	unsigned long ulPowerF;         // ��
	unsigned long ulPowerP;         // ƽ
	unsigned long ulPowerG;         // ��
}ScmMeter_Power;


//��ȱ���ʷ������Ϣ
typedef struct 
{
	ScmMeter_Power	ulMeter_Day;			//ÿ�ռ��ƽ���ܵ���
	ScmMeter_Power 	ulMeter_Month;     //ÿ�¼��ƽ���ܵ���
}ScmMeter_HisData;

enum cmEMMETER
{
	EMMETER_ANALOG = 0,//ģ��������
	EMMETER_HISDATA,//������ʷ����
};

//**************�û����ú���*******************//
extern void cmMeter_get_data(unsigned char cmd,void* str_data);


//static rt_err_t rt_send_mq(rt_mq_t mq, char* name, ScmStorage_Msg msg, void* args);

#endif

