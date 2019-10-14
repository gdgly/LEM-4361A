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

/******************************** ·���������Ϣ ***********************************/	//zcx190710
typedef enum {
	DevSt_Starting=0,			// ������
	DevSt_StandbyOK,          	// ��������
	DevSt_InCharging,           // �����
	DevSt_DisCharging,          // �ŵ���
	DevSt_Charged,				// ��ŵ���ɣ�3��ش�����
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
}DEVICE_FAULT;/*·������������*/


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


//typedef union 
//{
//	rt_uint32_t ALARM;
//	struct
//	{
//		rt_uint32_t BLE_COMM:1;           //bit0//����ͨ�Ź���
//		rt_uint32_t METER_COMM:1;         //bit1//����ģ��ͨ�Ź���
//		rt_uint32_t METER_VOL_HIGH:1;     //bit2//��ѹ����
//		rt_uint32_t METER_VOL_LOW:1;      //bit3//��ѹ����
//		rt_uint32_t METER_CUR_HIGH:1;     //bit4//����
//		rt_uint32_t STORAGE_NAND:1;       //bit5//flash����
//		rt_uint32_t RTC_COMM:1;          	//bit6//rtcͨ�Ź���
//		rt_uint32_t ESAM_COMM:1;      		//bit7//esamͨ�Ź���
//		rt_uint32_t HPLC_COMM:1;          //bit8//hplcͨ�Ź���
//	}
//	B;
//} 
//Systerm_Alarm;


//////////////////////////////////////////////////////////////////////////////////


extern unsigned long timebin2long(unsigned char *buf);
CCMRAM extern char Printf_Buffer[1024];
CCMRAM extern char Srintf_Buffer[1024];


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
