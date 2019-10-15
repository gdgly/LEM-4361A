#ifndef __ENERGYCON_H__
#define __ENERGYCON_H__

#include <string.h>
#include <stdio.h>
#include "global.h"
#include "chargepile.h"

extern ChargPilePara_TypeDef ChargePilePara_Set;
extern ChargPilePara_TypeDef ChargePilePara_Get;


extern struct rt_thread energycon;
extern struct rt_semaphore rx_sem_energycon;

/******************************* ������ *************************************/
typedef struct
{
	char OrderSn[17];			//������  octet-string��SIZE(16)��
	char cAssetNO[23];			//·�����ʲ����  visible-string��SIZE(22)��
	unsigned char GunNum;		//ǹ���	{Aǹ��1����Bǹ��2��}
	unsigned long SetPower;		//�趨��繦�ʣ���λ��W�����㣺-1��
	unsigned char cSucIdle;		//�ɹ���ʧ��ԭ��:{0���ɹ� 1��ʧ�� 255������}
}CTL_CHARGE;/*������������*/

#endif

