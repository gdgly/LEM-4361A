#ifndef __ANALOG_H__
#define __ANALOG_H__

#include <rtdevice.h>


/***********ģ����YC�ṹ��******************************/
typedef struct        //ģ������������ṹ����
{	
	unsigned long  Bat_vol;		    //��ص�ѹ  3λС��
	unsigned long  Pow_5V;       //������  3λС��
	unsigned long  Pow_3V;          //������ѹ  1λС��
}Power_Analog_TypeDef;


extern int get_analog_data(Power_Analog_TypeDef *analog);

#endif

