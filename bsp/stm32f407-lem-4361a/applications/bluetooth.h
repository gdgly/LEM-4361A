#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include <rtdevice.h>


#define  GET_REQUEST 						5
#define  SET_REQUEST 						6	
#define  ACTION_REQUEST			 	7
#define  REPORT_RESPONSE			  8
#define  PROXY_REQUEST				  9
#define  SECURITY_REQUEST     	16

#define GET_REQUEST_NOMAL			1	
#define ACTION_REQUEST_NOMAL			1

#define PLAINTEXT							0
#define CIPHERTEXT						1


#define ADDR_MAX_LINE					6
#define APDU_MAX_LENTH				512
#define APDU_USER_MAX_LENTH		512

struct _698_BLE_ADDR        //P   ��ַ��a
{
	union 
	{
		unsigned char SA;//��������ַ
		struct{
		unsigned char uclenth:4;//bit 0~2 +1 = ��ַ�򳤶�
		unsigned char uclogic:2;
		unsigned char uctype:2;
		}B;
	}S_ADDR;
	unsigned char addr[ADDR_MAX_LINE];//������sa����
	unsigned char CA;//�ͻ�����ַ
};

struct _698_APDU
{
	unsigned char apdu_cmd;//APDU����
	unsigned char apdu_data[APDU_MAX_LENTH];
};

//struct _698_BLE_GETREQUEST
//{
//	unsigned char apdu_attitude;//APUD��������
//	unsigned char apdu_piid;
//	
//	union 
//	{
//		unsigned char ucoad[4];//����
//		unsigned long uload;
//	}apdu_oad;
//	
//	unsigned char apdu_data[APDU_USER_MAX_LENTH];
//};

struct _698_BLE_GETREQUEST_NORMAL
{
	unsigned char apdu_attitude;//APUD��������
	unsigned char apdu_piid;
	
	union 
	{
		unsigned char ucoad[4];//����
		unsigned long uload;
	}apdu_oad;
	
	unsigned char time;
};

struct _698_BLE_METER_ADDR
{
	unsigned char	data;
	unsigned char data_type;
	unsigned char addr_len;
	unsigned char addr[6];
	unsigned char optional;
	unsigned char time;
};
 

struct _698_BLE_FRAME       //698Э��ṹ
{
	unsigned char head;  //��ʼ֡ͷ = 0x68
	union 
	{
		unsigned char uclenth[2];//����
		unsigned int ullenth;
	}datalenth;//֡����
	
	union 
	{
		unsigned char ucControl;//������
		struct{
		unsigned char ucFuncode:3;//bit 0~2
		unsigned char ucSC:1;
		unsigned char ucTemp:1;
		unsigned char ucFrame:1;
		unsigned char ucPRM:1;//���ݷ���
		unsigned char ucDIR:1;//���ݷ���
		}B;
	}control;//������

	struct _698_BLE_ADDR _698_ADDR;//��ַ��a
	
	union 
	{
		unsigned char ucHcs[2];//����
		unsigned int ulHcs;
	}HCS;
	
	struct _698_APDU apdu;
	
	union 
	{
		unsigned char ucFcs[2];//����
		unsigned int ulFcs;
	}FCS;
	unsigned char end;//�����ַ� = 0x16	
};
//#pragma pack ()

//enum{
// BLUETOOTH_CALLNUM=0xA0,        	//0xA0						//��ȡ׮���
// BLUETOOTH_SETNUM,     			//0xA1           //����׮���
// BLUETOOTH_SETPASSWORD,    	//0xA2           //���ó������
// BLUETOOTH_CHARGECHECK,    	//0xA3           //���������֤
// BLUETOOTH_STARTCHARGE,	    //0xA4           //�������
// BLUETOOTH_STOPCHARGE,	    //0xA5           //ֹͣ���
// BLUETOOTH_REALDATA,		    //0xA6           //ʵʱ����
// BLUETOOTH_CHARGERECOARD,   //0xA7            //����¼
// BLUETOOTH_SETPOWER,		    //0xA8            //���ù���
// BLUETOOTH_SETLOCK,			    //0xA9            //����������
// BLUETOOTH_SETCLOCK,		    //0xAA            //��ʱ
// BLUETOOTH_SETNAME,			    //0xAB            //������������
// BLUETOOTH_RESERCHARGE,	    //0xAC            //ԤԼ���
// BLUETOOTH_READVERSION,	    //0xAD            //��ȡ����汾��
// BLUETOOTH_BEAT,				    //0xAE            //��������
// BLUETOOTH_READPASSWORD,    //0xAF            //��ȡ�������
// BLUETOOTH_TIMESTARTCHARGE,	//0x81            //��ʱ���
// BLUETOOTH_TIMESTOPCHARGE,  //0x82            //��ʱֹͣ���
// BLUETOOTH_UPDATESOFT,     //0xC1            //���³���
// BLUETOOTH_RESETWIFI,		    //0xC5            //wifiģ��ָ�����Ĭ��
// BLUETOOTH_SETKC,    				//0xE0            //����������
// BLUETOOTH_READRAJDATA,    	//0xE1            //��ȡУ׼ϵ��
// BLUETOOTH_SETRAJDATA,	    //0xE2            //����У׼ϵ��
// BLUETOOTH_CLEARNUM,       //0xE3            //���׮��
// BLUETOOTH_SETMDINFO,    		//0xE4            //���ó�����Ϣ
// BLUETOOTH_SETGPRSIP,		    //0xE5						//��������ģ��IP
// BLUETOOTH_READGPRSIP,      //0xE6            //��ȡ����ģ��IP
// BLUETOOTH_SETJFMOD,		    //0xE7            //���üƷ�ģ��
// BLUETOOTH_RESETBLU,		    //0xE8            //����ģ��ָ�����Ĭ��
// BLUETOOTH_SETPARKLOCK,     //0xE9            //��λ������
// BLUETOOTH_SETWHITEID,    	//0xEB            //���ð�����
// BLUETOOTH_DEBUG,				    //0xEF            //����DEBUGģ��
//}BLUETOOTH_CMD;


/////**********�ڲ�����****************************************/
//unsigned char BLEStateUpdate(rt_device_t dev);

//void BLE_ProtocalRec(rt_device_t dev);

//rt_uint8_t SendBLUETOOTH_Frame(rt_device_t dev,rt_uint32_t cmd,rt_uint8_t reason);

//extern u8 SendMR_Frame(STR_UART_MSG_CTRL *pUart,u32 cmd);
//extern u8 MeterModbusStateUpdate(u8 *pArray,u8 ArrayLen,u32 cmd);
//extern void MeterModbus_RecProtocal(void);


#endif

