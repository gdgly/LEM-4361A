#ifndef __GLOBAL_H
#define __GLOBAL_H	

#include <rtthread.h>
#include  <rtconfig.h>
#include  <string.h>
#include  <stdarg.h>

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//LEM_4242A Board
//ϵͳʱ�ӳ�ʼ��	
//³������@LNINT
//������̳:www.openedv.com
//��������:2014/5/2
//�汾��V1.0
//��Ȩ���У����ؾ���
//Copyright(C) ɽ��³�����ܼ������޹�˾ 2017-2099
//All rights reserved
//********************************************************************************
//�޸�˵��
//��
//////////////////////////////////////////////////////////////////////////////////


#define CCMRAM __attribute__((section("ccmram")))

#define MY_HEX	1
#define MY_CHAR 2

extern unsigned char DEBUG_MSH;

typedef struct{
	rt_uint8_t DataRx_len;
	rt_uint8_t Rx_data[1024];
	rt_uint8_t DataTx_len;
	rt_uint8_t Tx_data[1024];

}ScmUart_Comm;

//////////////////////////////////////////////////////////////////////////////////
typedef struct 				
{
	unsigned char  Second;        // ��
	unsigned char  Minute;        // ��
	unsigned char  Hour;          // ʱ
	unsigned char  Day;           // ��
	
	unsigned char  Week;          //����
	
	unsigned char  Month;         // ��
	unsigned char  Year;          // �� ����λ
}STR_SYSTEM_TIME;
extern STR_SYSTEM_TIME System_Time_STR;




typedef union 
{
	rt_uint32_t ALARM;
	struct
	{
		rt_uint32_t BLE_COMM:1;           //bit0//����ͨ�Ź���
		rt_uint32_t METER_COMM:1;         //bit1//����ģ��ͨ�Ź���
		rt_uint32_t METER_VOL_HIGH:1;     //bit2//��ѹ����
		rt_uint32_t METER_VOL_LOW:1;      //bit3//��ѹ����
		rt_uint32_t METER_CUR_HIGH:1;     //bit4//����
		rt_uint32_t STORAGE_NAND:1;       //bit5//flash����
		rt_uint32_t RTC_COMM:1;          	//bit6//rtcͨ�Ź���
		rt_uint32_t ESAM_COMM:1;      		//bit7//esamͨ�Ź���
		rt_uint32_t HPLC_COMM:1;          //bit8//hplcͨ�Ź���
	}
	B;
}ROUTER_ALARM;//·����������Ϣ


typedef union 
{
	rt_uint32_t ALARM;
	struct
	{
		rt_uint32_t  ConnState:1;         //��������״̬  	    0 ���� 1δ����
		rt_uint32_t  StopEctFau:1;        //��ͣ��������        0 ���� 1�쳣
		rt_uint32_t	ArresterFau:1;       //����������	        0 ���� 1�쳣
		rt_uint32_t  GunOut:1;            //���ǹδ��λ		0 ���� 1�쳣
		rt_uint32_t  ChgTemptOVFau:1;     //���׮���¹���		
		rt_uint32_t  VOVWarn:1;           //�����ѹ��ѹ
		rt_uint32_t  VLVWarn:1;           //�����ѹǷѹ
		rt_uint32_t  OutConState:1;       //����Ӵ���״̬      0������1�쳣
		rt_uint32_t  PilotFau:1;		   //����г������Ƶ�������        
		rt_uint32_t  ACConFau:1;		   //�����Ӵ�������	    0������1�쳣
		rt_uint32_t  OutCurrAla:1;		   //��������澯		            
		rt_uint32_t  OverCurrFau:1;	   //�����������	    0������1����
		rt_uint32_t  CurrentOutFlag:1;    //���������ʱ��ʶ	TRUE��ʼ����   FALSEֹͣ����
		rt_uint32_t  CurrentOutCount:1;   //���������ʱ��ʱ	
		rt_uint32_t  CCFau:1;	  		   //�����ǹͷ�쳣�Ͽ�			 							
		rt_uint32_t  ACCirFau:1;		   //������·������
		rt_uint32_t  LockState:1;		   //���ӿڵ�����״̬
		rt_uint32_t  LockFauState:1;	   //���ӿڵ���������״̬
		rt_uint32_t  GunTemptOVFau:1;     //���ӿڹ��¹���				
		rt_uint32_t  CC:1;				   //�������״̬CC����4    1ǹ��λ׮  0ǹ�뿪׮
		rt_uint32_t  CP:2;				   //������״̬CP����1
		rt_uint32_t  PEFau:1;			   //PE���߹���
		rt_uint32_t  DooropenFau:1;	   //���Ŵ򿪹���
		rt_uint32_t  ChgTemptOVAla:1;	   //���׮���¸澯				
		rt_uint32_t  GunTemptOVAla:1;	   //���ǹ���¸澯				
		rt_uint32_t  Contactoradjoin:1;   //�Ӵ���ճ��
		rt_uint32_t  OthWarnVal:1;        //��������ֵ
	}
	B;
}CHARGEPILE_ALARM;//���׮������Ϣ

typedef struct 
{
	rt_uint32_t WorkState;    //  01æµ    10����   11 ��ͣ//ϵͳ����״̬
	
	ROUTER_ALARM ScmRouter_Alarm;
	CHARGEPILE_ALARM ScmChargePile_Alarm;
}Systerm_State;


//////////////////////////////////////////////////////////////////////////////////


extern unsigned long timebin2long(unsigned char *buf);
CCMRAM extern char Printf_Buffer[1024];
CCMRAM extern char Sprintf_Buffer[1024];


extern const char ProgramVersion[8]; // �汾��
extern void Kick_Dog(void);

extern unsigned char str2bcd(char*src,unsigned char *dest);
extern void BCD_toInt(unsigned char *data1,unsigned char *data2,unsigned char len);
extern void Int_toBCD(unsigned char *data1,unsigned char *data2,unsigned char len);
extern unsigned char bcd2str(unsigned char *src,char *dest,unsigned char count);
extern unsigned char CRC7(unsigned char *ptr,unsigned int cnt);
//extern unsigned short CRC16_CCITT_ISO(unsigned char *ptr, unsigned int count);
extern unsigned int CRC_16(unsigned char *ptr, unsigned int nComDataBufSize);
extern unsigned char CRC7(unsigned char *ptr,unsigned int count);
extern unsigned char XOR_Check(unsigned char *pData, unsigned int Len);


extern void my_printf(char* buf,rt_uint32_t datalenth,rt_uint8_t type,rt_uint8_t cmd,char* function);
////////////////////////////////////////////////////////////////////////////////// 

#endif
