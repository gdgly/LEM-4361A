#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <rtthread.h>
#include <global.h>



//typedef struct 				
//{
//	unsigned char  Second;        // ��
//	unsigned char  Minute;        // ��
//	unsigned char  Hour;          // ʱ
//	unsigned char  Week;          //����
//	
//	unsigned char  Day;           // �� 
//	unsigned char  Month;         // ��
//	unsigned char  Year;          // �� ����λ
//}STR_SYSTEM_TIME;

//extern STR_SYSTEM_TIME System_Time_STR;

/***********ģ����YC�ṹ��******************************/
//typedef struct        //ģ������������ṹ����
//{
//	union
//	{	
//		unsigned long  ChargVa;		    //����ѹ  1λС��
//		unsigned long  ChargIa;       //������  3λС��
//		unsigned long  V_cp;          //������ѹ  1λС��
//		unsigned long  Tempa;         //�¶�1     1λС��
//		unsigned long  Tempb;         //�¶�1     1λС��
//		
//		unsigned long lResult[5];
//	}Analog;
//}YC_Analog_TypeDef;



/////**********�û����ú���****************************************/
//extern YC_Analog_TypeDef  STR_YC_Analog_A;

extern rt_err_t Set_rtc_time(STR_SYSTEM_TIME *time);


///**********�ڲ�����****************************************/


#endif

