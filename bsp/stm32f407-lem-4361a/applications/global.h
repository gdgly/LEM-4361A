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
	RtSt_Starting=0,			// ������
	RtSt_StandbyOK,          	// ��������
	RtSt_InCharging,           	// �����
	RtSt_DisCharging,          	// �ŵ���
	RtSt_Charged,				// ��ŵ���ɣ�3��ش�����
	RtSt_Fault,           		// ����
	RtSt_Update,				// ������
}ROUTER_WORKSTATE;/*·����״̬*/


typedef struct
{
	char AssetNum[23];				//·�����ʲ���� �ַ��� maxlen=22
	rt_uint8_t WorkState;			//·��������״̬
}ROUTER_IFO_UNIT;/*·������Ϣ��Ԫ*/
extern ROUTER_IFO_UNIT RouterIfo;
	
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
}KEY_INFO_UNIT;/*·������Կ��Ϣ��Ԫ*/



/******************************** ���׮�����Ϣ ***********************************/	//zcx190807
typedef enum 
{
	ChgSt_Standby=0,            //��������
	ChgSt_InCharging,           //�����
	ChgSt_Fault,            	//����
}PILE_WORKSTATE;/*���׮״̬*/

typedef enum
{
	GUN_A=1,
	GUN_B,
}GUN_NUM;/*ǹ��� {Aǹ��1����Bǹ��2��}*/

typedef enum
{
	SEV_ENABLE=0,
	SEV_DISABLE,
}PILE_SERVICE;/*׮������ {���ã�0����ͣ�ã�1��}*/

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

typedef struct
{
	unsigned char PileNum[17];			//���׮���         visible-string��SIZE(16)����
	rt_uint8_t PileIdent;       		//���ӿڱ�ʶ(A/B)
	unsigned char PileInstallAddr[41];	//���׮�İ�װ��ַ   visible-string��
	unsigned long minChargePow;			//���׮��ͳ�繦�� double-long����λ��W�����㣺-1����
	unsigned long ulChargeRatePow;		//������ʶ���� double-long����λ��W�����㣺-1��
	rt_uint8_t AwakeSupport;			//�Ƿ�֧�ֻ���    {0:��֧�� 1��֧��}
	rt_uint8_t WorkState;				//����״̬
}PILE_IFO_UNIT;/*���׮��Ϣ��Ԫ*/

/******************************** ������Ϣ ***********************************/		//zcx190710
typedef enum 
{
	NO_FAU=0,
	MEMORY_FAU,		//	�ն������ڴ���ϣ�0��
	CLOCK_FAU,		//  ʱ�ӹ���        ��1��
	BOARD_FAU,		//  ����ͨ�Ź���    ��2��
	METER_FAU,		//  485�������     ��3��
	SCREEN_FAU,		//  ��ʾ�����      ��4��
	HPLC_FAU,		//  �ز�ͨ���쳣    ��5��
	NANDFLSH_FAU,	//	NandFLASH��ʼ������  ��6��
	ESAM_FAU,		//  ESAM����        ��7��
	BLE_FAU,		//	����ģ�����     ��8��
	BATTERY_FAU,	//	��Դģ����� 	��9��
	CANCOM_FAU,		//  ���׮ͨ�Ź��� ��10��
	CHGPILE_FAU,	//  ���׮�豸���� ��11��
	ORDRECOED_FAU, 	//	���ض�����¼�� ��12��
	RTC_FAU,		//	RTCͨ�Ź��� ��13��
}ROUTER_FAU;/*·������������*/

typedef union 
{
	rt_err_t Total;
	struct
	{
		rt_err_t Memory_Fau:1;		//	�ն������ڴ���ϣ�0��
		rt_err_t Clock_Fau:1;		//  ʱ�ӹ���        ��1��
		rt_err_t Board_Fau:1;		//  ����ͨ�Ź���    ��2��
		rt_err_t MeterCom_Fau:1;	//  485�������     ��3��
		rt_err_t Screen_Fau:1;		//  ��ʾ�����      ��4��
		rt_err_t Hplc_Fau:1;		//  �ز�ͨ���쳣    ��5��
		rt_err_t NandFlash_Fau:1;	//	NandFLASH��ʼ������  ��6��
		rt_err_t ESAM_Fau:1;		//  ESAM����        ��7��
		rt_err_t Ble_Fau:1;			//	����ģ�����     ��8��
		rt_err_t Battery_Fau:1;		//	��Դģ����� 	��9��
		rt_err_t CanCom_Fau:1;		//  ���׮ͨ�Ź��� ��10��
		rt_err_t ChgPile_Fau:1;		//  ���׮�豸���� ��11��
		rt_err_t OrdRecord_Fau:1; 	//	���ض�����¼�� ��12��
		rt_err_t RTC_Fau:1;         //	RTCͨ�Ź���  (13)	(�Զ���)
	}
	Bit;
}ROUTER_FAULT;
extern ROUTER_FAULT Fault;
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
