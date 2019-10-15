#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>   //�жϺ���ͷ�ļ� rt_hw_interrupt_disable rt_hw_interrupt_enable
#include <string.h>
#include <stdio.h>
#include <global.h>
#include <storage.h>
#include <meter.h>
#include "strategy.h"
#include "global.h"

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <uffs/uffs_fd.h>
#endif

#define THREAD_STORAGE_PRIORITY     19
#define THREAD_STORAGE_STACK_SIZE   1024*4
#define THREAD_STORAGE_TIMESLICE    5

#define WRITE    1
#define READ     2

static char* get_name(char* strtemp,rt_uint8_t*fpname,rt_uint8_t *fpcnt);
static char* get_pvalue(char* strtemp,rt_uint32_t*fpvalue,rt_uint8_t cnt);
static rt_uint8_t str2num(rt_uint8_t*src,rt_uint32_t *dest);
static rt_uint8_t str2nnum(rt_uint8_t*src,rt_uint8_t *dest);
static rt_uint32_t pow_df(rt_uint8_t m,rt_uint8_t n);

static struct rt_thread storage;
static rt_uint8_t storage_stack[THREAD_STORAGE_STACK_SIZE];//�̶߳�ջ
static rt_mutex_t storage_ReWr_mutex = RT_NULL;
#define MAX_DIR_NUM    14

#define MAX_MALLOC_NUM    2048

///* ����������ƿ� */
//rt_mailbox_t m_save_mail = RT_NULL;

const char* _dir_name[MAX_DIR_NUM]={
	"/SysPara",
	"/ChargePilePara",
	"/Strategy",
	"/Strategy/OrderCharge",
	"/Strategy/PlanOffer",
	"/Strategy/PlanFail",
	"/Strategy/OnlineState",
	"/Meter",
	"/HistoryRecord",
	"/ChargeRecord",
	"/PERecord",
	"/AlarmRecord",
	"/EventRecord",
	"/LOG",
};

//ROUTER_IFO_UNIT RouterIfo;

__align(4) char *Para_Buff = NULL; //�̻�����buff
const char* METER_POWER_PATH=(const char*)"/Meter";
const char* LOG_PATH=(const char*)"/LOG";
const char* CHARGE_RECORD_PATH=(const char*)"/ChargeRecord";
const char* HISTORY_RECORD_PATH=(const char*)"/HistoryRecord";

const char* ORDER_CHARGE_PATH=(const char*)"/Strategy/OrderCharge";
const char* PLAN_OFFER_PATH=(const char*)"/Strategy/PlanOffer";
const char* PLAN_FAIL_PATH=(const char*)"/Strategy/PlanFail";
const char* ONLINE_STATE_PATH=(const char*)"/Strategy/OnlineState";

const char* NAND_LOG_PATH_FILE=(const char*)"/LOG/log.txt";
const char* METER_ANALOG_PATH_FILE=(const char*)"/Meter/analog.txt";
const char* ROUTER_PARA_PATH_FILE=(const char*)"/SysPara/RouterPata.txt";
const char* METER_GJFMode_PATH_FILE=(const char*)"/Meter/GJFMode.txt";
const char* METER_HALF_POWER_PATH_FILE=(const char*)"/Meter/MeterHalfPower.txt";

static int Meter_Anolag_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Router_Para_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Meter_Power_Storage(const char *file,void *Storage_Para,rt_uint32_t YMD,rt_uint32_t cmd);
static int Meter_GJFMode_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Meter_HalfPower_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Charge_Record_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);

static int Order_Charge_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);
static int Plan_Offer_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);	
static int Plan_Fail_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);
static int Online_State_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);

static  int Log_Process(void);
static  int NAND_LogWrite(char* Pnand_Log,rt_uint32_t *nand_LogLen);
static  int LOG_BackUP(void);
static  int Catalogue_Del_Oldest(const char *PATH);
static  int Find_path_file(const char *PATH,int number,char *pathfile,rt_uint32_t *filenum);

static void storage_thread_entry(void *parameter)
{
//	extern int df(const char *path);
//	df("/");
//	extern void ls(const char *pathname);
//	ls("/Meter");
//	ls("/LOG");	
	rt_thread_delay(100);	
	while (1)
	{
//		/* lock scheduler */
//		rt_enter_critical();
///***************************************************************************************************/
////		Meter_AnalogW.ulVol--;// ������ѹ
////		Meter_AnalogW.ulCur--;// ��������
////		Meter_AnalogW.ulAcPwr--;// ˲ʱ�й�����
////		Meter_AnalogW.ulMeterTotal--;//�й��ܵ���
////		Meter_AnalogW.ulPwrFactor--;//��������
////		Meter_AnalogW.ulFrequency--;//Ƶ��

////		Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,WRITE);

////		Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,READ);
///***************************************************************************************************/		
////		ScmMeter_HisData pMeter_HisData;
////		pMeter_HisData.ulMeter_Day.ulPowerT = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerJ = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerF = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerP = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerG = 100;
////		
////		pMeter_HisData.ulMeter_Month.ulPowerT = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerJ = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerF = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerP = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerG = 200;
////		
////		SetStorageData(Cmd_MeterPowerWr,&pMeter_HisData,0x20190801);
////		SetStorageData(Cmd_MeterPowerWr,&pMeter_HisData,0x20190802);
////		SetStorageData(Cmd_MeterPowerWr,&pMeter_HisData,0x20190803);
////		GetStorageData(Cmd_MeterPowerRd,&pMeter_HisData,0x20190802);
////		memset(&pMeter_HisData,0x00,sizeof(pMeter_HisData));		
///***************************************************************************************************/		
//		ScmMeter_PriceModle pPriceModleData;

//	    int i = 0;
//		for(i=0;i<8;i++)
//		{
//			pPriceModleData.uiJFmodID[i] = i+1;
//			
//		}

//		pPriceModleData.EffectiveTime.Second = System_Time_STR.Second;        // ��
//		pPriceModleData.EffectiveTime.Minute = System_Time_STR.Minute;        // ��
//		pPriceModleData.EffectiveTime.Hour = System_Time_STR.Hour;          // ʱ
//		pPriceModleData.EffectiveTime.Day = System_Time_STR.Day;           // �� 
//		pPriceModleData.EffectiveTime.Month = System_Time_STR.Month;         // ��
//		pPriceModleData.EffectiveTime.Year = System_Time_STR.Year;          // �� ����λ		

//		pPriceModleData.unEffectiveTime.Second = System_Time_STR.Second;        // ��
//		pPriceModleData.unEffectiveTime.Minute = System_Time_STR.Minute;        // ��
//		pPriceModleData.unEffectiveTime.Hour = System_Time_STR.Hour;          // ʱ
//		pPriceModleData.unEffectiveTime.Day = System_Time_STR.Day;           // �� 
//		pPriceModleData.unEffectiveTime.Month = System_Time_STR.Month;         // ��
//		pPriceModleData.unEffectiveTime.Year = System_Time_STR.Year;          // �� ����λ			
//		
//		pPriceModleData.state = 200;//ִ��״̬
//		pPriceModleData.style =30;//��������
//		
//		for(i=0;i<48;i++)
//		{
//			pPriceModleData.ulTimeNo[i] = i+1;
//		}		

//		pPriceModleData.count = 48;//��Ч������
//		
//		for(i=0;i<48;i++)
//		{
//			pPriceModleData.ulPriceNo[i] = i+1;
//			
//		}
//		
//		SetStorageData(Cmd_MeterGJFModeWr,&pPriceModleData,0x20190803);	
//	
//		rt_lprintf("�洢���\n");
//		
//		ScmMeter_PriceModle pPriceModleDataRR;

//		GetStorageData(Cmd_MeterGJFModeRd,&pPriceModleDataRR,0x20190802);
//		
//		
//		for(i=0;i<8;i++)
//		{
//			rt_lprintf("pPriceModleData.uiJFmodID[%d]=%d\n",i,pPriceModleData.uiJFmodID[i]);
//		}

//		rt_lprintf("pPriceModleData.EffectiveTime.Year=%02X\n",pPriceModleData.EffectiveTime.Year);	
//		rt_lprintf("pPriceModleData.EffectiveTime.Month=%02X\n",pPriceModleData.EffectiveTime.Month);
//		rt_lprintf("pPriceModleData.EffectiveTime.Day=%02X\n",pPriceModleData.EffectiveTime.Day);	
//		rt_lprintf("pPriceModleData.EffectiveTime.Hour=%02X\n",pPriceModleData.EffectiveTime.Hour);
//		rt_lprintf("pPriceModleData.EffectiveTime.Minute=%02X\n",pPriceModleData.EffectiveTime.Minute);	
//		rt_lprintf("pPriceModleData.EffectiveTime.Second=%02X\n",pPriceModleData.EffectiveTime.Second);

//		rt_lprintf("pPriceModleData.unEffectiveTime.Year=%02X\n",pPriceModleData.unEffectiveTime.Year);	
//		rt_lprintf("pPriceModleData.unEffectiveTime.Month=%02X\n",pPriceModleData.unEffectiveTime.Month);
//		rt_lprintf("pPriceModleData.unEffectiveTime.Day=%02X\n",pPriceModleData.unEffectiveTime.Day);	
//		rt_lprintf("pPriceModleData.unEffectiveTime.Hour=%02X\n",pPriceModleData.unEffectiveTime.Hour);
//		rt_lprintf("pPriceModleData.unEffectiveTime.Minute=%02X\n",pPriceModleData.unEffectiveTime.Minute);	
//		rt_lprintf("pPriceModleData.unEffectiveTime.Second=%02X\n",pPriceModleData.unEffectiveTime.Second);

//		rt_lprintf("pPriceModleData.state=%d\n",pPriceModleData.state);	
//		rt_lprintf("pPriceModleData.style=%d\n",pPriceModleData.style);	

//		for(i=0;i<48;i++)
//		{
//			rt_lprintf("pPriceModleData.ulTimeNo[%d]=%d\n",i,pPriceModleData.ulTimeNo[i]);
//		}	
//		
//		rt_lprintf("pPriceModleData.count=%d\n",pPriceModleData.count);
//		
//		for(i=0;i<48;i++)
//		{
//			rt_lprintf("pPriceModleData.ulPriceNo[%d]=%d\n",i,pPriceModleData.ulPriceNo[i]);
//		}	

//		memset(&pPriceModleData,0x00,sizeof(pPriceModleData));
///***************************************************************************************************/
///***************************************************************************************************/		
////		static rt_uint32_t pulMeter_Half[48]; 
////		int i = 0;
////		for(i=0;i<48;i++)
////		{
////			pulMeter_Half[i] = i+1;		
////			
////		}
////		
////		SetStorageData(Cmd_MeterHalfPowerWr,&pulMeter_Half,0x20190803);	
////	
////		rt_lprintf("�洢���\n");
////		
////		static rt_uint32_t pulMeter_HalfRd[48]; 
////		GetStorageData(Cmd_MeterHalfPowerRd,&pulMeter_HalfRd,0x20190802);
////		for(i=0;i<48;i++)
////		{
////			rt_lprintf("pulMeter_HalfRd[%d]=%d\n",i,pulMeter_HalfRd[i]);
////		}	

////		memset(pulMeter_HalfRd,0x00,sizeof(pulMeter_HalfRd));
///***************************************************************************************************/
//		
//		/* unlock scheduler */
//		rt_exit_critical();	

//		Log_Process();// ����log��С�洢LOG����
		
//		df("/");
//		extern void list_mem(void);
//		list_mem();		
//		static struct statfs freespace;
//		int rval = statfs((const char*)("/"),&freespace);
//		if(RT_EOK == rval)
//		{
//			rt_lprintf("Storage:freespace.f_bfree=%u\n",freespace.f_bfree); /* free blocks in file system */
//			if(freespace.f_bfree < 1024/16)//ʣ��ռ�С��1/16=8MBʱ��ɾ��LOG�ļ������ϵ���־  1024/16
//			{
//				Catalogue_Del_Oldest(LOG_PATH);
//			}
//		}
//		else
//		{
//			rt_lprintf("Storage:��ȡNANDFLASHʣ��ռ�ʧ��=%d\n",rval);	
//		}

		rt_thread_mdelay(1000);
	}
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG����
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/


extern rt_uint32_t LogBufferLen;
extern char LogBuffer[4096];

static int Log_Process(void)
{
	if(LogBufferLen > sizeof(LogBuffer)/2)	
	{	
		//�洢��FLASH
		NAND_LogWrite(LogBuffer,&LogBufferLen);	
	}
	else
	{
//		rt_lprintf("[storage]:logδ��\n");
	}
	
    return 0;	
}
/*********************************************************************************************************
** Function name:		NAND_LogWrite
** Descriptions:		LOG����
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int NAND_LogWrite(char* Pnand_Log,rt_uint32_t *nand_LogLen)   //��������� Ϊ����д�ļ�ʱ���ֹ��log
{	   	   
	rt_uint8_t rval=0;			//����ֵ	
	
	/* ֻд����ĩβ��Ӵ� */
	int fd= open(NAND_LOG_PATH_FILE,O_WRONLY | O_CREAT | O_APPEND);
	if(fd >= 0)
	{
		rt_lprintf("[Storage]:%s�ļ��򿪳ɹ�\n",NAND_LOG_PATH_FILE);
	}
	else
	{
		strcpy((char*)Pnand_Log,"");//���LogBuffer֮��strlen(LogBuffer) = 0;
		rt_lprintf("[Storage]:%s�ļ���ʧ�� fd=%d\n",NAND_LOG_PATH_FILE,fd);
		rval = 1;
		return rval;
	}
	
	static struct stat logstat;
	rval = fstat(fd,&logstat);
	if(rval == RT_EOK)
	{
		rt_lprintf("[Storage]:log�ֽ���=%u\n",logstat.st_size);
	}
	else
	{
		rt_lprintf("[Storage]:log�ļ�״̬����=%d\n",rval);	
	}
	
	int writelen = write(fd,Pnand_Log,*nand_LogLen);//д���ײ�   ����ֵ0���ɹ�	
	strcpy((char*)Pnand_Log,"");//���USBLogBuffer֮��strlen(USBLogBuffer) = 0;
	if(writelen > 0)
	{
		rt_lprintf("[Storage]:log�ļ�д��ɹ� writelen=%d\n",writelen);
	}
	else
	{
		extern rt_thread_t rt_current_thread;
		rt_lprintf("[Storage]:log�ļ�д��ʧ�� writelen=%d,error=%d\n",writelen,rt_current_thread->error);
		rval = 2;
	}
	
	if(close(fd) != UENOERR)
	{
		rt_lprintf("[Storage]:log�ļ��ر�ʧ��\n");
		rval = 3;
		return rval; 			
	}
	else
	{
		rt_lprintf("[Storage]:log�ļ��رճɹ�\n");
	}
		
	if(logstat.st_size > 3*1024*1024)//3MB
	{
		rt_lprintf("[Storage]:LOG�ļ�>3MB\n");
		LOG_BackUP();
	}
	
	return rval;
}
/*
*********************************************************************************************************
*	�� �� ��: LOG_BakeUP
*	����˵��: ��LOG�ļ��ƶ���ͬĿ¼��ʱ�������ļ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static int LOG_BackUP(void)
{
	rt_uint8_t rval=0;			//����ֵ	  
	char timestamp[32];
	char logBack_file[64];

	sprintf((char*)logBack_file,"%s","/LOG");	
	sprintf((char*)timestamp,"/%02X%02X%02X%02X%02X%02X.TXT",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
															 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
	strcat((char*)logBack_file,(const char*)timestamp);
	rval = unlink((const char*)logBack_file);//Ŀ���ļ��д��ڸ��ļ�,��ɾ��
	if(rval == UENOERR)
	{
		rt_lprintf("%s �ļ�ɾ���ɹ� fd=%d\n",logBack_file,rval);
	}
	else
	{
		rt_lprintf("%s ɾ���ļ������� fd=%d\n",logBack_file,rval);
	}

	rval = rename((const char*)NAND_LOG_PATH_FILE,(const char*)logBack_file);//��Դ�ļ����ƶ���Ŀ���ļ���  ���ֱ��ֲ���
	if(rval == UENOERR)
	{
		rt_lprintf("LOG_BackUP:%s to %s�ɹ�\n",NAND_LOG_PATH_FILE,logBack_file);
	}
	else
	{
		rt_lprintf("LOG_BackUP:%s to %sʧ��,rval=%u\n",NAND_LOG_PATH_FILE,logBack_file,rval);
	}			
	
	return rval;
}
/*********************************************************************************************************
** Function name:		Catalogue_Del_Oldest
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Catalogue_Del_Oldest(const char *PATH)
{
	int result;
	DIR *dirp;
	struct dirent *fd;
	rt_uint32_t filenum = 0;
	char oldest_file[] = "9999-991231080808.TXT";//Ĭ�����ϵ��ļ�
	char oldest_path_file[] = "/AAAAAAAAAAAA/BBBBBBBBBBBA/9999-991231080808.TXT";	
	
	/* ��PATH Ŀ¼ */
	dirp = opendir(PATH);
	if(dirp == RT_NULL)
	{
		rt_lprintf("opendir %s error\n",PATH);
		result = -1;
	}
	else
	{
		rt_lprintf("opendir %s ok\n",PATH);
	}

	/* ��ȡĿ¼ */
	while ((fd = readdir(dirp)) != RT_NULL)
	{
		rt_lprintf("found %s\n", fd->d_name);
		if(strcmp((const char*)oldest_file,(const char*)fd->d_name) > 0)
		{
			strcpy((char*)oldest_file,(const char*)fd->d_name);
		}
		rt_lprintf("oldest_file:%s\n",(const char*)oldest_file);
		filenum++;	
	}
	rt_lprintf("%s has %d files\n",PATH,filenum);

	sprintf((char*)oldest_path_file,"%s/",PATH);
	strcat((char*)oldest_path_file,(const char*)oldest_file);	
	result = unlink((const char*)oldest_path_file); //ɾ��oldest_file ��������·��
	if(UENOERR == result)
	{
		rt_lprintf("%s �ļ�ɾ���ɹ�\n",oldest_path_file);
	}
	else
	{
		rt_lprintf("%s ɾ���ļ������� result=%d\n",oldest_path_file,result);
	}
//	/* ��ȡĿ¼���Ķ�ȡλ�� */
//	off_t offset = telldir(dirp);	
//	seekdir(dirp,offset);	
//	rt_lprintf("current offset=%d\n",offset);
//	/* �����ȡĿ¼Ϊ��ͷλ�� */
//	rewinddir(dirp);		

	/* �ر�Ŀ¼ */
	result = closedir(dirp);	
	if(result == RT_NULL)
	{
		rt_lprintf("closedir %s ok\n",PATH);
	}
	else
	{
		rt_lprintf("closedir %s error\n",PATH);
		result = -2;
	}	
	return result;
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//	int result;
//	struct dirent *fd;
//	int filenum = 0;
//	char oldest_path_file[64];
//	char history_log[40][32]={0};

//	/* ��Ŀ¼ */
//	DIR *dirp = opendir(PATH);
//	if(dirp == RT_NULL)
//	{
//		rt_lprintf("opendir %s error\n",PATH);
//		result = -1;
//		return result;
//	}
//	else
//	{
//		rt_lprintf("opendir %s ok\n",PATH);
//	}

//	/* ��ȡĿ¼ */
//	while ((fd = readdir(dirp)) != RT_NULL)
//	{
//		rt_lprintf("found %s\n", fd->d_name);
//		strcpy((char*)history_log[filenum],(const char*)fd->d_name);
//		rt_lprintf("history_log[%d]=%s\n",filenum,history_log[filenum]);
//		filenum++;
//	}
//	
//	rt_lprintf("filenum=%d\n",filenum);
//	char temp_file[] = "999999999999.TXT";//��ʱ�ļ�
//	for(int i=0;i<filenum-1;i++)
//	{
//		for(int j=0;j<filenum-i-1;j++)
//		{
//			if(strcmp((char*)history_log[j],(char*)history_log[j+1]) > 0) //Ѱ�������ļ�������ǰ��
//			{
//				strcpy((char*)temp_file,(char*)history_log[j]);
//				strcpy((char*)(char*)history_log[j],(char*)history_log[j+1]);
//				strcpy((char*)(char*)history_log[j+1],(char*)temp_file);
//			}
//		}
//	}

//	for(int i=0;i<filenum;i++)
//	{
//		rt_lprintf("history_log[%d]=%s\n",i,history_log[i]);
//	}
//	
//	/* ������Ҫ��Ŀ���ļ�����·�� */
//	sprintf((char*)oldest_path_file,"%s/",PATH);
//	strcat((char*)oldest_path_file,(const char*)history_log[0]);
//	result = unlink((const char*)oldest_path_file); //ɾ��oldest_file ��������·��
//	if(UENOERR == result)
//	{
//		rt_lprintf("%s �ļ�ɾ���ɹ�\n",oldest_path_file);
//	}
//	else
//	{
//		rt_lprintf("%s ɾ���ļ������� result=%d\n",oldest_path_file,result);
//	}
//	
//	/* �ر�Ŀ¼ */
//	result = closedir(dirp);	
//	if(result == RT_NULL)
//	{
//		rt_lprintf("closedir %s ok\n",PATH);
//	}
//	else
//	{
//		rt_lprintf("closedir %s error\n",PATH);
//		result = -2;
//	}	
//	return result;
}
/*********************************************************************************************************
** Function name:		Find_path_file
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#define MAX_History_num_10    10
static int Find_path_file(const char *PATH,int number,char *pathfile,rt_uint32_t *filenum)
{
	int result;
	struct dirent *fd;
	*filenum = 0;
	
	char history_record[MAX_History_num_10][24]={0};	
	
	/* ��Ŀ¼ */
	DIR *dirp = opendir(PATH);
	if(dirp == RT_NULL)
	{
		rt_lprintf("opendir %s error\n",PATH);
		result = -1;
		return result;
	}
	else
	{
		rt_lprintf("opendir %s ok\n",PATH);
	}
	/* ��ȡĿ¼�ļ����� */
	while ((fd = readdir(dirp)) != RT_NULL)
	{
		(*filenum)++;
	}
	rt_lprintf("current filenum=%d\n",*filenum);
	/* ɾ�������ļ� */
	if(*filenum > MAX_History_num_10)
	{
		/* �ر�Ŀ¼ */
		result = closedir(dirp);	
		if(result == RT_NULL)
		{
			rt_lprintf("closedir %s ok\n",PATH);
		}
		else
		{
			rt_lprintf("closedir %s error\n",PATH);
			result = -2;
			return result;
		}
		
		rt_lprintf("����: too many filenum=%d\n",*filenum);
		int i;
		for(int i=0;i<*filenum - MAX_History_num_10;i++)
		{
			Catalogue_Del_Oldest(PATH);
		}
		/* ���δ�Ŀ¼ */
		dirp = opendir(PATH);
		if(dirp == RT_NULL)
		{
			rt_lprintf("opendir %s error\n",PATH);
			result = -1;
			return result;
		}
		else
		{
			rt_lprintf("opendir %s ok\n",PATH);
		}		
	}
	else
	{
		/* �����ȡĿ¼Ϊ��ͷλ�� */
		rewinddir(dirp);
	}
	
	/* ���filenum����ȡĿ¼ */
	(*filenum) = 0;
	while ((fd = readdir(dirp)) != RT_NULL)
	{
		rt_lprintf("found %s\n", fd->d_name);
		if(*filenum < MAX_History_num_10)
		{
			strcpy((char*)history_record[*filenum],(const char*)fd->d_name);
			rt_lprintf("history_record[%d]=%s\n",*filenum,history_record[*filenum]);
		}
		(*filenum)++;
	}
	/* �ر�Ŀ¼ */
	result = closedir(dirp);	
	if(result == RT_NULL)
	{
		rt_lprintf("closedir %s ok\n",PATH);
	}
	else
	{
		rt_lprintf("closedir %s error\n",PATH);
		result = -2;
		return result;
	}	
	
	/* ð������ */
	RT_ASSERT(*filenum <= MAX_History_num_10);
	char temp_file[] = "9999-999999999999.TXT";//��ʱ�ļ�
	for(int i=0;i<(*filenum)-1;i++)
	{
		for(int j=0;j<(*filenum)-i-1;j++)
		{
			if(strcmp((char*)history_record[j],(char*)history_record[j+1]) < 0) //Ѱ�������ļ�������ǰ��
			{
				strcpy((char*)temp_file,(char*)history_record[j+1]);
				strcpy((char*)(char*)history_record[j+1],(char*)history_record[j]);
				strcpy((char*)(char*)history_record[j],(char*)temp_file);
			}
		}
	}
	/* �г���ʷ��¼ */
	for(int i=0;i<(*filenum);i++)
	{
		rt_lprintf("history_record[%d]=%s\n",i,history_record[i]);
	}
	
	/* ���·��ص�Ŀ���ļ�����·�� */
	sprintf((char*)pathfile,"%s/",PATH);
	strcat((char*)pathfile,(const char*)history_record[number]);
		
	return result;
}
/*********************************************************************************************************
** Function name:		Meter_Anolag_Storage
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_Anolag_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)
{
	int fd,writelen = 0,readlen = 0;
	char buffer[64];
	ScmMeter_Analog* pMeter_Anolag = (ScmMeter_Analog*)Storage_Para;
	
	if(cmd == WRITE)//���浽����write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"ulVol=%u\n",(rt_uint32_t)pMeter_Anolag->ulVol);// ������ѹ
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulCur=%u\n",(rt_uint32_t)pMeter_Anolag->ulCur);// ��������
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulAcPwr=%u\n",(rt_uint32_t)pMeter_Anolag->ulAcPwr);// ˲ʱ�й�����
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulMeterTotal=%u\n",(rt_uint32_t)pMeter_Anolag->ulMeterTotal);//�й��ܵ���
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPwrFactor=%u\n",(rt_uint32_t)pMeter_Anolag->ulPwrFactor);//��������
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulFrequency=%u\n",(rt_uint32_t)pMeter_Anolag->ulFrequency);//Ƶ��
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			return -1;
		}
		else
		{
//			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}
		/* Opens the file, if it is existing. If not, a new file is created. */
		fd= open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("%s�ļ��򿪳ɹ� fd=%d\n",file,fd);
		}
		else
		{
			rt_lprintf("%s�ļ���ʧ�� fd=%d\n",file,fd);
		}
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:�ļ�д��ɹ� writelen=%d\n",writelen);
		}
		else
		{
			rt_lprintf("[storage]:�ļ�д��ʧ�� writelen=%d\n",writelen);
		}				
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}			
	}
	else if(cmd == READ) //�ӱ���read
	{
		fd= open(file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("%s�ļ��򿪳ɹ�\n",file);
		}
		else
		{
			rt_lprintf("%s�ļ���ʧ�� fd=%d\n",file,fd);
		}
		readlen=read(fd,Para_Buff,strlen(Para_Buff));	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:�ļ���ȡ�ɹ� readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:�ļ���ȡʧ�� readlen=%d\n",readlen);
		}
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen < RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:�ļ���ȡ����\n%s\n",Para_Buff);
		}
	
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //����¼32���ֽ�	

////////////////////������ѹ//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulVol")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulVol,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulVol=%u;\n",pMeter_Anolag->ulVol);
			}			
			else
			{
				rt_lprintf("�ļ��������� ulVol=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		} 			
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////��������////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulCur")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulCur,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulCur=%u;\n",pMeter_Anolag->ulCur);
			}			
			else
			{
				rt_lprintf("�ļ��������� ulCur=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////˲ʱ�й�����////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulAcPwr")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulAcPwr,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulAcPwr=%u;\n",pMeter_Anolag->ulAcPwr);
			}			
			else
			{
				rt_lprintf("�ļ��������� ulAcPwr=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////�й��ܵ���////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulMeterTotal")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulMeterTotal,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulMeterTotal=%u;\n",pMeter_Anolag->ulMeterTotal);
			}			
			else
			{
				rt_lprintf("�ļ��������� ulMeterTotal=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////��������////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPwrFactor")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulPwrFactor,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPwrFactor=%u;\n",pMeter_Anolag->ulPwrFactor);
			}			
			else
			{
				rt_lprintf("�ļ��������� ulPwrFactor=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////Ƶ��///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulFrequency")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulFrequency,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulFrequency=%u;\n",pMeter_Anolag->ulFrequency);
			}			
			else
			{
				rt_lprintf("[storage]:�ļ��������� ulFrequency=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("[storage]:namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:�ļ���Ч����\n");
	}
	
	return 0;
}
/*********************************************************************************************************
** Function name:		Router_Para_Storage
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Router_Para_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)	
{
	int fd,writelen = 0,readlen = 0;
	char buffer[64];
	
	if(cmd == WRITE)//���浽����write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		strcat((char*)Para_Buff,(const char*)"cAssetNum=");
		for(int i=0;i<sizeof(RouterIfo.AssetNum);i++)
		{
			sprintf((char*)buffer,"%02X",RouterIfo.AssetNum[i]);// ���
			strcat((char*)Para_Buff,(const char*)buffer);
		}
		strcat((char*)Para_Buff,(const char*)"\n");		
		
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow\n");
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}
		
/************************************************************************************************/			
		/* Opens the file, if it is existing. If not, a new file is created. */
		int fd= open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",file,fd);

			return -2; 			
		}		
/************************************************************************************************/			
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:�ļ�д��ɹ� writelen=%d\n",writelen);
		}
		else
		{
			rt_lprintf("[storage]:�ļ�д��ʧ�� writelen=%d\n",writelen);
		}				
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		fd= open(file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",file,fd);
		}
        /***************************************************************************************/
		readlen=read(fd,Para_Buff,strlen(Para_Buff));	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:�ļ���ȡ�ɹ� readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:�ļ���ȡʧ�� readlen=%d\n",readlen);
		}
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:�ļ���ȡ����\n%s\n",Para_Buff);
		}
	
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //����¼32���ֽ�	

////////////////////�ʲ����//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"cAssetNum")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&RouterIfo.AssetNum,sizeof(RouterIfo.AssetNum));//���ص�ǰ�ļ���ָ��
				rt_lprintf("[storage]:cAssetNum��%s\n",RouterIfo.AssetNum);

			}			
			else
			{
				rt_lprintf("[storage]:�ļ��������� ulVol=%s\n",fpname); 
			}
			namelen=0;		
		}
		else
		{
            rt_lprintf("[storage]:namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:�ļ���Ч����\n");
	}

	return 0;	
	
}
/*********************************************************************************************************
** Function name:		Meter_GJFMode_Storage
** Descriptions:		LOG����
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_GJFMode_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)	
{
	int fd,writelen = 0,readlen = 0;
	char buffadd[32];
	int i = 0;
	ScmMeter_PriceModle* pMeter_GJFMode = (ScmMeter_PriceModle*)Storage_Para;

	rt_lprintf("Meter_GJFMode_Storage:file = %s\n",(char*)file);	
	
	if(cmd == WRITE)//���浽����write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffadd,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",\
								System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
								System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffadd);

		/****************************************************************************************/	
			//�Ʒ�ģ��ID
			sprintf((char*)buffadd,"uiJFmodID=%02X%02X%02X%02X%02X%02X%02X%02X\n",\
									pMeter_GJFMode->uiJFmodID[0],pMeter_GJFMode->uiJFmodID[1],\
									pMeter_GJFMode->uiJFmodID[2],pMeter_GJFMode->uiJFmodID[3],
									pMeter_GJFMode->uiJFmodID[4],pMeter_GJFMode->uiJFmodID[5],\
									pMeter_GJFMode->uiJFmodID[6],pMeter_GJFMode->uiJFmodID[7]); 
			strcat((char*)Para_Buff,(const char*)buffadd);
		/****************************************************************************************/
			//��Чʱ��	
			sprintf((char*)buffadd,"EffectiveTime=%02X%02X%02X%02X%02X%02X\n",\
									pMeter_GJFMode->EffectiveTime.Second,\
									pMeter_GJFMode->EffectiveTime.Minute,\
									pMeter_GJFMode->EffectiveTime.Hour,\
									pMeter_GJFMode->EffectiveTime.Day,\
									pMeter_GJFMode->EffectiveTime.Month,\
									pMeter_GJFMode->EffectiveTime.Year); 
			strcat((char*)Para_Buff,(const char*)buffadd);				
		/****************************************************************************************/	
		/****************************************************************************************/
			//ʧЧʱ��	
			sprintf((char*)buffadd,"unEffectiveTime=%02X%02X%02X%02X%02X%02X\n",\
									pMeter_GJFMode->unEffectiveTime.Second,\
									pMeter_GJFMode->unEffectiveTime.Minute,\
									pMeter_GJFMode->unEffectiveTime.Hour,\
									pMeter_GJFMode->unEffectiveTime.Day,\
									pMeter_GJFMode->unEffectiveTime.Month,\
									pMeter_GJFMode->unEffectiveTime.Year); 
			strcat((char*)Para_Buff,(const char*)buffadd);		
		/****************************************************************************************/
		/****************************************************************************************/	
			//ִ��״̬
			sprintf((char*)buffadd,"state=%u\n",pMeter_GJFMode->state);
			strcat((char*)Para_Buff,(const char*)buffadd);
		/****************************************************************************************/
			//��������
			sprintf((char*)buffadd,"style=%u\n",pMeter_GJFMode->style);
			strcat((char*)Para_Buff,(const char*)buffadd);		
		/****************************************************************************************/			
			//48�����ʺ� 0:00~24:00
			for(i=0;i<48;i++)
			{
				sprintf((char*)buffadd,"ulTimeNo[%u]=%u\n",i,pMeter_GJFMode->ulTimeNo[i]); 
				strcat((char*)Para_Buff,(const char*)buffadd);			
			}
		/****************************************************************************************/
			//��Ч������
			sprintf((char*)buffadd,"count=%u\n",pMeter_GJFMode->count); //�洢�Ʒ�ģ�ͷ��ʸ���
			strcat((char*)Para_Buff,(const char*)buffadd);
		/****************************************************************************************/	
			// count�����
			for(i=0;i<pMeter_GJFMode->count;i++)
			{
				sprintf((char*)buffadd,"ulPriceNo[%u]=%u\n",i,(rt_uint32_t)pMeter_GJFMode->ulPriceNo[i]); 
				strcat((char*)Para_Buff,(const char*)buffadd);		
				
			}
		/****************************************************************************************/
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",file,fd);
			return -2;		
		}
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//��fd_put����ʹ�÷�����ļ�����
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
/************************************************************************************************/			
/************************************************************************************************/				
		if(fp->size <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:�ļ�д����\n%s\n",Para_Buff);
		}
		
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		fd= open(file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",file,fd);
			return -2;
		}
        /***************************************************************************************/	
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:�Ʒ�ģ���ļ���ȡ�ɹ� readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:�Ʒ�ģ���ļ���ȡʧ�� readlen=%d\n",readlen);
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}
		
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:�ļ���ȡ����\n%s\n",Para_Buff);
		}

		/************************************************************************************/		
		/************************************************************************************/		
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //����¼32���ֽ�
		rt_uint8_t fpnameRd[32] = {0};      //����¼32���ֽ�	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////��Ҫ����uiJFmodID//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);
		if(namelen)
		{
			sprintf((char*)fpnameRd,"uiJFmodID"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->uiJFmodID,8);
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pMeter_GJFMode->uiJFmodID[0],pMeter_GJFMode->uiJFmodID[1],pMeter_GJFMode->uiJFmodID[2],pMeter_GJFMode->uiJFmodID[3],\
							pMeter_GJFMode->uiJFmodID[4],pMeter_GJFMode->uiJFmodID[5],pMeter_GJFMode->uiJFmodID[6],pMeter_GJFMode->uiJFmodID[7]);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");	
		}
//////////////////��Чʱ��//////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"EffectiveTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->EffectiveTime.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pMeter_GJFMode->EffectiveTime.Year,\
							pMeter_GJFMode->EffectiveTime.Month,\
							pMeter_GJFMode->EffectiveTime.Day,\
							pMeter_GJFMode->EffectiveTime.Hour,\
							pMeter_GJFMode->EffectiveTime.Minute,\
							pMeter_GJFMode->EffectiveTime.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
//////////////////ʧЧʱ��//////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"unEffectiveTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->unEffectiveTime.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pMeter_GJFMode->unEffectiveTime.Year,\
							pMeter_GJFMode->unEffectiveTime.Month,\
							pMeter_GJFMode->unEffectiveTime.Day,\
							pMeter_GJFMode->unEffectiveTime.Hour,\
							pMeter_GJFMode->unEffectiveTime.Minute,\
							pMeter_GJFMode->unEffectiveTime.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
////////////////////////////////////////////////////////////////		
///////////////////ִ��״̬/////////////////////////////////////////////////			
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"state"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)			
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->state,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->state);
			}			
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
///////////////////��������/////////////////////////////////////////////////			
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"style"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->style,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->style);
			}			
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}		
////////////////////�Ʒ�ģ��/////////////////////////////////////////////////////////////
		for(i=0;i<48;i++)
		{
			sprintf((char*)fpnameRd,"ulTimeNo[%u]",i); 
		    fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
			if(namelen)//��������
			{
				if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
				{
					fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->ulTimeNo[i],1);//���ص�ǰ�ļ���ָ��
					rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->ulTimeNo[i]);
				}
				else
				{
					rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
				}
				namelen=0;
			}
			else
			{
				rt_lprintf("namelen=0\n");
			}
		}
///////////////////�Ʒ�ģ�ͷ��ʸ���/////////////////////////////////////////////////			
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"count"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->count,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->count);
			}			
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		} 		
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////�Ʒ�ģ�ͷ��ʼ۸�//////////////////////////////////////////////////////////
		for(i=0;i<pMeter_GJFMode->count;i++)//
		{
			sprintf((char*)fpnameRd,"ulPriceNo[%u]",i); 

		    fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
			if(namelen)//��������
			{
				if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
				{
					fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->ulPriceNo[i],1);//���ص�ǰ�ļ���ָ��
					rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->ulPriceNo[i]);
				}
				else
				{
					rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
				}
				namelen=0;
			}
			else
			{
				rt_lprintf("namelen=0\n");
			}
		}
        rt_lprintf("pMeter_GJFMode file read OK\n");		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:�ļ���Ч����\n");
	}

	return 0;	
	
}
/*********************************************************************************************************
** Function name:		Meter_HalfPower_Storage
** Descriptions:		Meter_HalfPower_Storage����
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_HalfPower_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffadd[32];
	int i = 0;
	rt_uint32_t* ulMeter_Half = (rt_uint32_t*)Storage_Para;

	rt_lprintf("Meter_HalfPower_Storage:file = %s\n",(char*)file);	
	
	if(cmd == WRITE)//���浽����write
	{
		int writelen = 0;
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffadd,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",\
								System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
								System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffadd);

		/****************************************************************************************/			
		//48��HalfPower 0:00~24:00
		for(i=0;i<48;i++)
		{
			sprintf((char*)buffadd,"ulMeter_Half[%u]=%u\n",i,ulMeter_Half[i]); 
			strcat((char*)Para_Buff,(const char*)buffadd);			
		}
		/****************************************************************************************/
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",file,fd);

			return -2; 			
		}
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//��fd_put����ʹ�÷�����ļ�����
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
/************************************************************************************************/			
/************************************************************************************************/
		if(fp->size <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:�ļ�д����\n%s\n",Para_Buff);
		}
		
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		int readlen = 0;
		fd= open(file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",file,fd);
			return -2; 
		}
        /***************************************************************************************/	
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:��Сʱ�����ļ���ȡ�ɹ� readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:��Сʱ�����ļ���ȡʧ�� readlen=%d\n",readlen);
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:�ļ��ر�ʧ��\n");
		}
		
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:��Сʱ�����ļ���ȡ����\n%s\n",Para_Buff);
		}

		/************************************************************************************/		
		/************************************************************************************/		
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //����¼32���ֽ�
		rt_uint8_t fpnameRd[32] = {0};      //����¼32���ֽ�	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////��Сʱ����////////////////////////////////////////////////////////////
		for(i=0;i<48;i++)
		{
			sprintf((char*)fpnameRd,"ulMeter_Half[%u]",i); 
		    fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
			if(namelen)//��������
			{
				if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
				{
					fpoint = get_pvalue(fpoint,(rt_uint32_t*)&ulMeter_Half[i],1);//���ص�ǰ�ļ���ָ��
					rt_lprintf("%s=%u;\n",fpnameRd,ulMeter_Half[i]);
				}
				else
				{
					rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
				}
				namelen=0;
			}
			else
			{
				rt_lprintf("namelen=0\n");
			}	

		}
        rt_lprintf("ulMeter_Half file read OK\n");		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:�ļ���Ч����\n");
	}

	return 0;	
	
}
/*********************************************************************************************************
** Function name:		Meter_Power_Storage
** Descriptions:		LOG����
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_Power_Storage(const char *file,void *Storage_Para,rt_uint32_t YMD,rt_uint32_t cmd)	
{
	int fd,writelen = 0,readlen = 0;
	char buffer[32]; 
	char path[14];
	char path_file[64];
	ScmMeter_HisData* pMeter_HisData = (ScmMeter_HisData*)Storage_Para;

	sprintf((char*)path_file,"%s",METER_POWER_PATH);	
	sprintf((char*)path,"/%06X.txt",YMD>>8);
	strcat((char*)path_file,(const char*)path);
	rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);	
	
	if(cmd == WRITE)//���浽����write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"%08X\n",YMD);// ��-20190801
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"ulPowerTday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerT);// ��-�ܵ���
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerJday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerJ);// ��-��
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerFday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerF);// ��-��
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerPday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerP);// ��-ƽ
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerGday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerG);// ��-��
		strcat((char*)Para_Buff,(const char*)buffer);
        /****************************************************************************************/
		sprintf((char*)buffer,"ulPowerTmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerT);// ��-�ܵ���
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerJmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerJ);// ��-�ܵ���
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerFmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerF);// ��-�ܵ���
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerPmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerP);// ��-�ܵ���
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerGmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerG);// ��-�ܵ���
		strcat((char*)Para_Buff,(const char*)buffer);	
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_RDWR | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//��fd_put����ʹ�÷�����ļ�����
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
/************************************************************************************************/
		if(fp->size > 0)
		{
			char* Para_Bufftemp = rt_malloc(fp->size);//�����ڴ�
			if(Para_Bufftemp == NULL)
			{
				rt_lprintf("[storage]:Para_Bufftemp �����ڴ�ʧ��\n");
				if(close(fd) != UENOERR)
				{
					rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",file);
				}			
				return -3;
			}		
			readlen=read(fd,Para_Bufftemp,fp->size);	//����txt���������	
			if(readlen > 0) //0
			{
				rt_lprintf("[storage]:�������ļ���ȡ�ɹ� readlen=%d\n",readlen);
			}
			else
			{
				rt_lprintf("[storage]:�������ļ���ȡΪ�� readlen=%d\n",readlen);
			}
			/******�ļ���Ϊ�ղŽ��в����������������ֱ��д�������****************************/
			if(readlen > 0) //0
			{
				char ymd[] ="20180808";
				sprintf((char*)ymd,"%08X",YMD);// ��-20190801	
				char *addr = strstr(Para_Bufftemp,ymd);
				if(addr == RT_NULL)
				{
					rt_lprintf("[storage]:%sδ�ҵ�\n",ymd);		
				}
				else
				{
					rt_lprintf("[storage]:%s�ҵ�\n",ymd);
					rt_uint32_t addr_offset = 0x00;
					addr_offset = addr - Para_Bufftemp;
					rt_lprintf("[storage]:addr_offset=%d\n",addr_offset);			
					lseek(fd,addr_offset-23,SEEK_SET);//ƫ�Ƶ�ָ��λ�� 23����ʶ�����ʼ��ʱ���		
				}
				if(readlen <= RT_CONSOLEBUF_SIZE)
				{
					rt_lprintf("[storage]:�ļ�������\n%s\n",Para_Bufftemp);
				}		
				rt_free(Para_Bufftemp);Para_Bufftemp = NULL;
			}
		}		
/************************************************************************************/				
/**********����д�������**************************************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		fd= open(path_file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//��fd_put����ʹ�÷�����ļ�����
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
		if(fp->size == NULL)
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
			}
			rt_lprintf("[storage]:%s�ļ�Ϊ��,ֱ�ӷ���\n",path_file);
			return -1;
		}		
/************************************************************************************************/		
		char* Para_Bufftemp = rt_malloc(fp->size);//�����ڴ�
		if(Para_Bufftemp == NULL)
		{
			rt_lprintf("[storage]:Para_Bufftemp �����ڴ�ʧ��\n");
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",file);
			}	
			return -2;
		}		
		readlen=read(fd,Para_Bufftemp,fp->size);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s�ļ���ȡ�ɹ� readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",file);
			}
			rt_free(Para_Bufftemp);Para_Bufftemp = NULL;
			rt_lprintf("[storage]:%s�ļ���ȡΪ�� readlen=%d\n",path_file,readlen);
			return -3;
		}
		/******�ļ���Ϊ�ղŽ��в����������,����ֱ��д�������****************************/
		char ymd[] ="20180808";
		sprintf((char*)ymd,"%08X",YMD);// ��-20190801	
		char *addr = strstr(Para_Bufftemp,ymd);
		if(addr == RT_NULL)
		{
			rt_lprintf("[storage]:%sδ�ҵ�\n",ymd);	
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",file);
			}
			rt_free(Para_Bufftemp);Para_Bufftemp = NULL;
			return -4;				
		}
		else
		{
			rt_lprintf("[storage]:%s�ҵ�\n",ymd);
			rt_uint32_t addr_offset = 0x00;
			addr_offset = addr - Para_Bufftemp;
			rt_lprintf("[storage]:addr_offset=%d\n",addr_offset);			
			lseek(fd,addr_offset-23,SEEK_SET);//ƫ�Ƶ�ָ��λ�� 23����ʶ�����ʼ��ʱ���		
		}
		*(Para_Bufftemp+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:�ļ�д����\n%s\n",Para_Bufftemp);
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = addr;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //����¼32���ֽ�	
////////////////////��--�ܵ���//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerTday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerT,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerTday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerT);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerTday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 			
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--�����////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerJday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerJ,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerJday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerJ);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerJday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--�����////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerFday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerF,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerFday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerF);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerFday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--ƽ����///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerPday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerP,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerPday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerP);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerPday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////��--�ȵ���///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerGday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerG,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerGday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerG);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerGday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--�ܵ���///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerTmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerT,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerTmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerT);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerTmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--�����///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerJmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerJ,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerJmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerJ);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerJmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--�����///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerFmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerF,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerFmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerF);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerFmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--ƽ����///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerPmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerP,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerPmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerP);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerPmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////��--�ȵ���///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerGmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerG,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("ulPowerGmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerG);
			}			
			else        
			{
				rt_lprintf("�ļ��������� ulPowerGmon=%s\n",fpname); 
			}
			namelen=0;
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
		rt_free(Para_Bufftemp);Para_Bufftemp = NULL;		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:�ļ���Ч����\n");
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Charge_Record_Storage
** Descriptions:		LOG����
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Charge_Record_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	CHG_ORDER_EVENT* pCharge_Record = (CHG_ORDER_EVENT*)Storage_Para;
		
	if(cmd == WRITE)//���浽����write
	{
		// �����ļ���
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pCharge_Record->OrderNum,\
																			  pCharge_Record->StartTimestamp.Second,\
																			  pCharge_Record->StartTimestamp.Minute,\
																			  pCharge_Record->StartTimestamp.Hour,\
																			  pCharge_Record->StartTimestamp.Day,\
																			  pCharge_Record->StartTimestamp.Month,\
																			  pCharge_Record->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// ׼��д������
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pCharge_Record->OrderNum);//	�¼���¼���
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->StartTimestamp.Second,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Minute,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Hour,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Day,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Month,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->FinishTimestamp.Second,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Day,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Month,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼�����ԭ��
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pCharge_Record->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼��ϱ�״̬ = ͨ���ϱ�״̬
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pCharge_Record->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// ������뵥�ţ�SIZE(16)��
		sprintf((char*)buffer,"RequestNO=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pCharge_Record->RequestNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pCharge_Record->RequestNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		
		// �û�id  visible-string��SIZE(64)��
		sprintf((char*)buffer,"cUserID=");
		for(int i=0;i<sizeof(pCharge_Record->cUserID);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pCharge_Record->cUserID[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
	
		//	·�����ʲ���� visible-string��SIZE(22)��
		sprintf((char*)buffer,"AssetNO=");
		for(int i=0;i<sizeof(pCharge_Record->AssetNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pCharge_Record->AssetNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	
	
		//	��������������λ��kWh�����㣺-2��
		sprintf((char*)buffer,"ChargeReqEle=%08u\n",(rt_uint32_t)pCharge_Record->ChargeReqEle);
		strcat((char*)Para_Buff,(const char*)buffer);	
		
		sprintf((char*)buffer,"RequestTimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->RequestTimeStamp.Second,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Minute,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Hour,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Day,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Month,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Year);//	�������ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);	
	
		sprintf((char*)buffer,"PlanUnChg_TimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Second,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Minute,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Hour,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Day,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Month,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Year);//	�ƻ��ó�ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);		

		//	���ģʽ {������0��������1��}
		sprintf((char*)buffer,"ChargeMode=%08u\n",(rt_uint32_t)pCharge_Record->ChargeMode);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//	����ʱ�����ֵ
		sprintf((char*)buffer,"StartMeterValue=%08u\n",(rt_uint32_t)pCharge_Record->StartMeterValue);
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		//	ֹͣʱ�����ֵ
		sprintf((char*)buffer,"StopMeterValue=%08u\n",(rt_uint32_t)pCharge_Record->StopMeterValue);
		strcat((char*)Para_Buff,(const char*)buffer);
		
	
		sprintf((char*)buffer,"ChgStartTime=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->ChgStartTime.Second,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Minute,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Hour,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Day,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Month,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Year);//	�������ʱ��	
	
		sprintf((char*)buffer,"ChgStopTime=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->ChgStopTime.Second,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Minute,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Hour,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Day,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Month,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Year);//	���ֹͣʱ��	

		//	�ѳ��������λ��kWh�����㣺-2��
		sprintf((char*)buffer,"ucChargeEle=%08u\n",(rt_uint32_t)pCharge_Record->ucChargeEle);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//	�ѳ�ʱ�䣨��λ��s��	
		sprintf((char*)buffer,"ucChargeTime=%08u\n",(rt_uint32_t)pCharge_Record->ucChargeTime);
		strcat((char*)Para_Buff,(const char*)buffer);		
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********����д�������************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s�ļ���ȡ�ɹ� readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
			}
			rt_lprintf("[storage]:%s�ļ���ȡΪ�� readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s�ļ�������\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //����¼32���ֽ�
		rt_uint8_t fpnameRd[32] = {0,};      //����¼32���ֽ�	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼���¼���/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->OrderNum,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->OrderNum);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->StartTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->StartTimestamp.Year,\
							pCharge_Record->StartTimestamp.Month,\
							pCharge_Record->StartTimestamp.Day,\
							pCharge_Record->StartTimestamp.Hour,\
							pCharge_Record->StartTimestamp.Minute,\
							pCharge_Record->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->FinishTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->FinishTimestamp.Year,\
							pCharge_Record->FinishTimestamp.Month,\
							pCharge_Record->FinishTimestamp.Day,\
							pCharge_Record->FinishTimestamp.Hour,\
							pCharge_Record->FinishTimestamp.Minute,\
							pCharge_Record->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////�¼�����ԭ��/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->Reason,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->Reason);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////�¼��ϱ�״̬ = ͨ���ϱ�״̬/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChannelState,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ChannelState);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////������뵥�� SIZE(16)/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"RequestNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->RequestNO,sizeof(pCharge_Record->RequestNO));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pCharge_Record->RequestNO);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////�û�id  visible-string��SIZE(64)��/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"cUserID"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->cUserID,sizeof(pCharge_Record->cUserID));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pCharge_Record->cUserID);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////·�����ʲ���� visible-string��SIZE(22)��////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AssetNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->AssetNO,sizeof(pCharge_Record->AssetNO));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pCharge_Record->AssetNO);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}	
/////////��������������λ��kWh�����㣺-2��/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeReqEle"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChargeReqEle,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ChargeReqEle);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////�������ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"RequestTimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->RequestTimeStamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->RequestTimeStamp.Year,\
							pCharge_Record->RequestTimeStamp.Month,\
							pCharge_Record->RequestTimeStamp.Day,\
							pCharge_Record->RequestTimeStamp.Hour,\
							pCharge_Record->RequestTimeStamp.Minute,\
							pCharge_Record->RequestTimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////�ƻ��ó�ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"PlanUnChg_TimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->PlanUnChg_TimeStamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->PlanUnChg_TimeStamp.Year,\
							pCharge_Record->PlanUnChg_TimeStamp.Month,\
							pCharge_Record->PlanUnChg_TimeStamp.Day,\
							pCharge_Record->PlanUnChg_TimeStamp.Hour,\
							pCharge_Record->PlanUnChg_TimeStamp.Minute,\
							pCharge_Record->PlanUnChg_TimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////���ģʽ {������0��������1��}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeMode"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChargeMode,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ChargeMode);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////����ʱ�����ֵ////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"StartMeterValue"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->StartMeterValue,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->StartMeterValue);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////ֹͣʱ�����ֵ////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"StopMeterValue"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->StopMeterValue,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->StopMeterValue);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}	
////////////////////�������ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"ChgStartTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChgStartTime.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->ChgStartTime.Year,\
							pCharge_Record->ChgStartTime.Month,\
							pCharge_Record->ChgStartTime.Day,\
							pCharge_Record->ChgStartTime.Hour,\
							pCharge_Record->ChgStartTime.Minute,\
							pCharge_Record->ChgStartTime.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////�������ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"ChgStopTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChgStopTime.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->ChgStopTime.Year,\
							pCharge_Record->ChgStopTime.Month,\
							pCharge_Record->ChgStopTime.Day,\
							pCharge_Record->ChgStopTime.Hour,\
							pCharge_Record->ChgStopTime.Minute,\
							pCharge_Record->ChgStopTime.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////�ѳ��������λ��kWh�����㣺-2��////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ucChargeEle"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ucChargeEle,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ucChargeEle);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////�ѳ�ʱ�䣨��λ��s��	///////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ucChargeTime"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ucChargeTime,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ucChargeTime);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
		rt_lprintf("[storage]:%s�ļ�д�ɹ�\n",path_file);
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s�ļ���Ч����\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Order_Charge_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Order_Charge_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	ORDER_CHG_EVENT* pOrder_Charge = (ORDER_CHG_EVENT*)Storage_Para;
	
	if(cmd == WRITE)//���浽����write
	{
		// �����ļ���
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pOrder_Charge->OrderNum,\
																			  pOrder_Charge->StartTimestamp.Second,\
																			  pOrder_Charge->StartTimestamp.Minute,\
																			  pOrder_Charge->StartTimestamp.Hour,\
																			  pOrder_Charge->StartTimestamp.Day,\
																			  pOrder_Charge->StartTimestamp.Month,\
																			  pOrder_Charge->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// ׼��д������
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pOrder_Charge->OrderNum);//	�¼���¼���
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOrder_Charge->StartTimestamp.Second,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Minute,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Hour,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Day,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Month,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOrder_Charge->FinishTimestamp.Second,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Day,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Month,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼�����ԭ��
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pOrder_Charge->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼��ϱ�״̬ = ͨ���ϱ�״̬
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pOrder_Charge->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);

		// �ܹ���
		sprintf((char*)buffer,"TotalFault=%08u\n",(rt_uint32_t)pOrder_Charge->TotalFault);
		strcat((char*)Para_Buff,(const char*)buffer);	
		
		// �������״̬
		sprintf((char*)buffer,"Fau=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pOrder_Charge->Fau);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pOrder_Charge->Fau[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********����д�������************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s�ļ���ȡ�ɹ� readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
			}
			rt_lprintf("[storage]:%s�ļ���ȡΪ�� readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s�ļ�������\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //����¼32���ֽ�
		rt_uint8_t fpnameRd[32] = {0,};      //����¼32���ֽ�	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼���¼���/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->OrderNum,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->OrderNum);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->StartTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOrder_Charge->StartTimestamp.Year,\
							pOrder_Charge->StartTimestamp.Month,\
							pOrder_Charge->StartTimestamp.Day,\
							pOrder_Charge->StartTimestamp.Hour,\
							pOrder_Charge->StartTimestamp.Minute,\
							pOrder_Charge->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->FinishTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOrder_Charge->FinishTimestamp.Year,\
							pOrder_Charge->FinishTimestamp.Month,\
							pOrder_Charge->FinishTimestamp.Day,\
							pOrder_Charge->FinishTimestamp.Hour,\
							pOrder_Charge->FinishTimestamp.Minute,\
							pOrder_Charge->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////�¼�����ԭ��/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->Reason,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->Reason);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////�¼��ϱ�״̬ = ͨ���ϱ�״̬/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->ChannelState,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->ChannelState);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////�ܹ���///////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"TotalFault"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->TotalFault,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->TotalFault);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
		rt_lprintf("[storage]:%s�ļ�д�ɹ�\n",path_file);		
/////////�������״̬/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Fau"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->Fau,sizeof(pOrder_Charge->Fau));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pOrder_Charge->Fau);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s�ļ���Ч����\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Plan_Offer_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Plan_Offer_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	PLAN_OFFER_EVENT* pPlan_Offer = (PLAN_OFFER_EVENT*)Storage_Para;
	
	if(cmd == WRITE)//���浽����write
	{
		// �����ļ���
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pPlan_Offer->OrderNum,\
																			  pPlan_Offer->StartTimestamp.Second,\
																			  pPlan_Offer->StartTimestamp.Minute,\
																			  pPlan_Offer->StartTimestamp.Hour,\
																			  pPlan_Offer->StartTimestamp.Day,\
																			  pPlan_Offer->StartTimestamp.Month,\
																			  pPlan_Offer->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// ׼��д������
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pPlan_Offer->OrderNum);//	�¼���¼���
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->StartTimestamp.Second,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Minute,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Hour,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Day,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Month,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->FinishTimestamp.Second,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Day,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Month,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼�����ԭ��
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pPlan_Offer->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼��ϱ�״̬ = ͨ���ϱ�״̬
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pPlan_Offer->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);

		// ������뵥�ţ�SIZE(16)��
		sprintf((char*)buffer,"RequestNO=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pPlan_Offer->RequestNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->RequestNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		
		// �û�id  visible-string��SIZE(64)��
		sprintf((char*)buffer,"cUserID=");
		for(int i=0;i<sizeof(pPlan_Offer->cUserID);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->cUserID[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
	
		//	·�����ʲ���� visible-string��SIZE(22)��
		sprintf((char*)buffer,"AssetNO=");
		for(int i=0;i<sizeof(pPlan_Offer->AssetNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->AssetNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	
	
		//	��������������λ��kWh�����㣺-2��
		sprintf((char*)buffer,"ChargeReqEle=%08u\n",(rt_uint32_t)pPlan_Offer->ChargeReqEle);
		strcat((char*)Para_Buff,(const char*)buffer);	
		
		sprintf((char*)buffer,"RequestTimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->RequestTimeStamp.Second,\
							   										        (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Minute,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Hour,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Day,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Month,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Year);//	�������ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);	
	
		sprintf((char*)buffer,"PlanUnChg_TimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Second,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Minute,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Hour,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Day,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Month,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Year);//	�ƻ��ó�ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);		

		//	���ģʽ {������0��������1��}
		sprintf((char*)buffer,"ChargeMode=%08u\n",(rt_uint32_t)pPlan_Offer->ChargeMode);
		strcat((char*)Para_Buff,(const char*)buffer);
	
		//	�û���¼����  visible-string��SIZE(38)��
		sprintf((char*)buffer,"Token=");
		for(int i=0;i<sizeof(pPlan_Offer->Token);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->Token[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");		
		//	����û��˺�  visible-string��SIZE(9)��
		sprintf((char*)buffer,"UserAccount=");
		for(int i=0;i<sizeof(pPlan_Offer->UserAccount);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->UserAccount[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		//	��n�������������Ե�����	
		sprintf((char*)buffer,"Data=");
		for(int i=0;i<sizeof(pPlan_Offer->Data);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->Data[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********����д�������************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s�ļ���ȡ�ɹ� readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
			}
			rt_lprintf("[storage]:%s�ļ���ȡΪ�� readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s�ļ�������\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //����¼32���ֽ�
		rt_uint8_t fpnameRd[32] = {0,};      //����¼32���ֽ�	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼���¼���/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->OrderNum,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->OrderNum);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->StartTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->StartTimestamp.Year,\
							pPlan_Offer->StartTimestamp.Month,\
							pPlan_Offer->StartTimestamp.Day,\
							pPlan_Offer->StartTimestamp.Hour,\
							pPlan_Offer->StartTimestamp.Minute,\
							pPlan_Offer->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->FinishTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->FinishTimestamp.Year,\
							pPlan_Offer->FinishTimestamp.Month,\
							pPlan_Offer->FinishTimestamp.Day,\
							pPlan_Offer->FinishTimestamp.Hour,\
							pPlan_Offer->FinishTimestamp.Minute,\
							pPlan_Offer->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////�¼�����ԭ��/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->Reason,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->Reason);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////�¼��ϱ�״̬ = ͨ���ϱ�״̬/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->ChannelState,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->ChannelState);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////������뵥�� SIZE(16)/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"RequestNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->RequestNO,sizeof(pPlan_Offer->RequestNO));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->RequestNO);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////�û�id  visible-string��SIZE(64)��/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"cUserID"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->cUserID,sizeof(pPlan_Offer->cUserID));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->cUserID);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////·�����ʲ���� visible-string��SIZE(22)��////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AssetNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->AssetNO,sizeof(pPlan_Offer->AssetNO));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->AssetNO);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}	
/////////��������������λ��kWh�����㣺-2��/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeReqEle"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->ChargeReqEle,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->ChargeReqEle);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////�������ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"RequestTimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->RequestTimeStamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->RequestTimeStamp.Year,\
							pPlan_Offer->RequestTimeStamp.Month,\
							pPlan_Offer->RequestTimeStamp.Day,\
							pPlan_Offer->RequestTimeStamp.Hour,\
							pPlan_Offer->RequestTimeStamp.Minute,\
							pPlan_Offer->RequestTimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////�ƻ��ó�ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"PlanUnChg_TimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->PlanUnChg_TimeStamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->PlanUnChg_TimeStamp.Year,\
							pPlan_Offer->PlanUnChg_TimeStamp.Month,\
							pPlan_Offer->PlanUnChg_TimeStamp.Day,\
							pPlan_Offer->PlanUnChg_TimeStamp.Hour,\
							pPlan_Offer->PlanUnChg_TimeStamp.Minute,\
							pPlan_Offer->PlanUnChg_TimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////���ģʽ {������0��������1��}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeMode"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->ChargeMode,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->ChargeMode);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////�û���¼����  visible-string��SIZE(38)��////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Token"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->Token,sizeof(pPlan_Offer->Token));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->Token);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////����û��˺�  visible-string��SIZE(9)��/////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"UserAccount"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->UserAccount,sizeof(pPlan_Offer->UserAccount));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->UserAccount);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////��n�������������Ե�����	///////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Data"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->Data,sizeof(pPlan_Offer->Data));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->Data);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s�ļ���Ч����\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Plan_Fail_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Plan_Fail_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	PLAN_FAIL_EVENT* pPlan_Fail = (PLAN_FAIL_EVENT*)Storage_Para;
	
	if(cmd == WRITE)//���浽����write
	{
		// �����ļ���
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pPlan_Fail->OrderNum,\
																			  pPlan_Fail->StartTimestamp.Second,\
																			  pPlan_Fail->StartTimestamp.Minute,\
																			  pPlan_Fail->StartTimestamp.Hour,\
																			  pPlan_Fail->StartTimestamp.Day,\
																			  pPlan_Fail->StartTimestamp.Month,\
																			  pPlan_Fail->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// ׼��д������
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pPlan_Fail->OrderNum);//	�¼���¼���
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Fail->StartTimestamp.Second,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Minute,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Hour,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Day,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Month,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Fail->FinishTimestamp.Second,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Day,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Month,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Year);//  �¼�����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼�����ԭ��
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pPlan_Fail->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼��ϱ�״̬ = ͨ���ϱ�״̬
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pPlan_Fail->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);

		// ������뵥�ţ�SIZE(16)��
		sprintf((char*)buffer,"RequestNO=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pPlan_Fail->RequestNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Fail->RequestNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
	
		//	·�����ʲ���� visible-string��SIZE(22)��
		sprintf((char*)buffer,"AssetNO=");
		for(int i=0;i<sizeof(pPlan_Fail->AssetNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Fail->AssetNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	

		//	��n�������������Ե�����	
		sprintf((char*)buffer,"Data=");
		for(int i=0;i<sizeof(pPlan_Fail->Data);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Fail->Data[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********����д�������************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s�ļ���ȡ�ɹ� readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
			}
			rt_lprintf("[storage]:%s�ļ���ȡΪ�� readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s�ļ�������\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //����¼32���ֽ�
		rt_uint8_t fpnameRd[32] = {0,};      //����¼32���ֽ�	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼���¼���/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->OrderNum,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Fail->OrderNum);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->StartTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Fail->StartTimestamp.Year,\
							pPlan_Fail->StartTimestamp.Month,\
							pPlan_Fail->StartTimestamp.Day,\
							pPlan_Fail->StartTimestamp.Hour,\
							pPlan_Fail->StartTimestamp.Minute,\
							pPlan_Fail->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->FinishTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Fail->FinishTimestamp.Year,\
							pPlan_Fail->FinishTimestamp.Month,\
							pPlan_Fail->FinishTimestamp.Day,\
							pPlan_Fail->FinishTimestamp.Hour,\
							pPlan_Fail->FinishTimestamp.Minute,\
							pPlan_Fail->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////�¼�����ԭ��/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->Reason,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Fail->Reason);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////�¼��ϱ�״̬ = ͨ���ϱ�״̬/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->ChannelState,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Fail->ChannelState);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////������뵥�� SIZE(16)/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"RequestNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->RequestNO,sizeof(pPlan_Fail->RequestNO));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Fail->RequestNO);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////·�����ʲ���� visible-string��SIZE(22)��////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AssetNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->AssetNO,sizeof(pPlan_Fail->AssetNO));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Fail->AssetNO);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////��n�������������Ե�����	///////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Data"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->Data,sizeof(pPlan_Fail->Data));//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Fail->Data);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s�ļ���Ч����\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Online_State_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Online_State_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	ONLINE_STATE* pOnline_State = (ONLINE_STATE*)Storage_Para;	
	
	if(cmd == WRITE)//���浽����write
	{
		// �����ļ���
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pOnline_State->OrderNum,\
																			  pOnline_State->OnlineTimestamp.Second,\
																			  pOnline_State->OnlineTimestamp.Minute,\
																			  pOnline_State->OnlineTimestamp.Hour,\
																			  pOnline_State->OnlineTimestamp.Day,\
																			  pOnline_State->OnlineTimestamp.Month,\
																			  pOnline_State->OnlineTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// ׼��д������
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pOnline_State->OrderNum);//	�¼���¼���
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOnline_State->OfflineTimestamp.Second,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Minute,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Hour,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Day,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Month,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Year);//  ����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOnline_State->OfflineTimestamp.Second,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Minute,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Hour,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Day,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Month,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Year);//  ����ʱ��
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// �¼��ϱ�״̬ = ͨ���ϱ�״̬
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pOnline_State->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//״̬�仯 {���ߣ�0���� ���ߣ�1��}
		sprintf((char*)buffer,"AutualState=%03u\n",(rt_uint32_t)pOnline_State->AutualState);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//����ԭ�� {δ֪��0����ͣ�磨1�����ŵ��仯��2��}
		sprintf((char*)buffer,"OfflineReason=%03u\n",(rt_uint32_t)pOnline_State->OfflineReason);
		strcat((char*)Para_Buff,(const char*)buffer);
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********����д�������************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//д���ײ�   ����ֵ0���ɹ�	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s�ļ�д��ɹ� writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ�д��ʧ�� writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //�ӱ���read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//���ļ�������ļ������ڣ����ʧ�ܡ�
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s�ļ��򿪳ɹ�\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s�ļ���ʧ�� fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//����txt���������	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s�ļ���ȡ�ɹ� readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
			}
			rt_lprintf("[storage]:%s�ļ���ȡΪ�� readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//��Ҫ׷�ӽ�����
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s�ļ�������\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//����
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //����¼32���ֽ�
		rt_uint8_t fpnameRd[32] = {0,};      //����¼32���ֽ�	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼���¼���/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OrderNum,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->OrderNum);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"OnlineTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OnlineTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOnline_State->OnlineTimestamp.Year,\
							pOnline_State->OnlineTimestamp.Month,\
							pOnline_State->OnlineTimestamp.Day,\
							pOnline_State->OnlineTimestamp.Hour,\
							pOnline_State->OnlineTimestamp.Minute,\
							pOnline_State->OnlineTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////�¼�����ʱ��//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			sprintf((char*)fpnameRd,"OfflineTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OfflineTimestamp.Second,6);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOnline_State->OfflineTimestamp.Year,\
							pOnline_State->OfflineTimestamp.Month,\
							pOnline_State->OfflineTimestamp.Day,\
							pOnline_State->OfflineTimestamp.Hour,\
							pOnline_State->OfflineTimestamp.Minute,\
							pOnline_State->OfflineTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////�¼��ϱ�״̬ = ͨ���ϱ�״̬/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->ChannelState,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->ChannelState);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////״̬�仯 {���ߣ�0���� ���ߣ�1��}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AutualState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->AutualState,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->AutualState);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////����ԭ�� {δ֪��0����ͣ�磨1�����ŵ��仯��2��}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OfflineReason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//���ص�ǰ�ļ���ָ��
		if(namelen)//��������
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OfflineReason,1);//���ص�ǰ�ļ���ָ��
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->OfflineReason);
			}
			else
			{
				rt_lprintf("%s������������fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s�ļ���Ч����\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG����
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
//��str�еõ�������
static char* get_name(char* strtemp,rt_uint8_t*fpname,rt_uint8_t *fpcnt)
{
	while(*strtemp!='\0')//û�н��� �ļ���һ��Ϊ���� �ڶ���Ϊ������ʼ
	{
		if((*strtemp=='\r')||(*strtemp=='\n'))//����¼50���ַ�
		{	
			break;
		}
		strtemp++; 
		
	}
	while(*strtemp!='\0')//û�н���
	{
		if((*strtemp!=' ')&&(*strtemp!='=')&&(*strtemp!='\r')&&(*strtemp!='\n')&&\
			(*strtemp!='/')&&(*fpcnt)<50)//����¼50���ַ�
		{	
			fpname[(*fpcnt)++]=*strtemp;//fpcnt��������ţ�++���ȼ�����*
			strtemp++; 
		}
		else if(*fpcnt) break;//�洢֮���һ�δ���˳�
		else strtemp++; 
		
	}
	if(*fpcnt)//��������
	{
		fpname[*fpcnt]='\0';//���������
	}
	
  return strtemp; 	
	
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG����
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
//��str�еõ�����ֵ   ���λ��16λ
static char* get_pvalue(char* strtemp,rt_uint32_t*fpvalue,rt_uint8_t cnt)
{
	rt_uint16_t i,j = 0;
	rt_uint8_t valueArry[48] = {0};
	rt_uint8_t arryadd[72] = {0};
	rt_uint8_t Arrycnt = 0;
	
	while(*strtemp!='\0')//û�н���
	{	
		if(((*strtemp!=' ')&&(*strtemp!='=')&&(*strtemp!=';')&& \
			(*strtemp<='9')&&(*strtemp>='0')&&(Arrycnt<48))||((*strtemp>='A')&&(*strtemp<='F')))//����¼48���ַ�
		{			
			valueArry[Arrycnt++]=*strtemp;//
			strtemp++; 
		}
		else if(Arrycnt) break;//�洢֮���һ�δ���˳�
		else strtemp++; 
	}
	if(Arrycnt)//��������
	{
		valueArry[Arrycnt++]='\0';//���������
	} 
    if(cnt == 1) //һ������ֵ
	{		
	    str2num(valueArry,fpvalue);
	}
	else  //�������ֵ
	{
		for(i=0,j=0;i<cnt*2; )  //��cnt������֮����ӷֽ�� '\0'
		{
		    arryadd[j++] = valueArry[i++];
			arryadd[j++] = valueArry[i++];
			arryadd[j++] = '\0';
		}		

		for(i=0;i<cnt;i++)
		{
		    str2nnum(arryadd+3*i,((rt_uint8_t*)fpvalue)+i);//16����+�ո�=3�ַ�
		}
		
	}
	
  return strtemp; 	
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG����
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
int storage_thread_init(void)
{
//	extern void nerase_all(void);
//	nerase_all();//��ʽ��nand�ļ�ϵͳ
	
	if(dfs_mount("nand0", "/", "uffs", 0, 0) == RT_EOK)
	{	
		rt_lprintf("[storage]:nand flash mount to / success\n");
	}
	else
	{
		rt_lprintf("[storage]:nand flash mount to / fail\n");
	}
	
	for(rt_uint8_t i = 0; i< MAX_DIR_NUM; i++)
	{
		int result = mkdir(_dir_name[i],0x777);//����_dir_name�������ļ�Ŀ¼
		if(result == UENOERR)
		{
			rt_lprintf("[storage]:make %s dir ok\n",_dir_name[i]);
		}
		else if(result == (-EEXIST))
		{
			rt_lprintf("[storage]:%s dir is already exist\n",_dir_name[i]);
		}
		else
		{
			rt_lprintf("[storage]:make %s dir fail,error num is %d\n",_dir_name[i],result);
		}
	}
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */

//		int fd = unlink(METER_GJFMode_PATH_FILE);
//		if(fd >= UENOERR)
//		{
//			rt_lprintf("%s�ļ�ɾ���ɹ� fd=%d\n",METER_GJFMode_PATH_FILE,fd);
//		}
//		else
//		{
//			rt_lprintf("%sɾ���ļ������� fd=%d\n",METER_GJFMode_PATH_FILE,fd);
//		}

//		int fd = rmdir("/LOG");//����Ϊ��Ŀ¼
//		if(fd >= UENOERR)
//		{
//			rt_lprintf("%sĿ¼ɾ���ɹ� fd=%d\n","/LOG",fd);
//		}
//		else
//		{
//			rt_lprintf("%sɾ��Ŀ¼������ fd=%d\n","/LOG",fd);
//		}	
	
//		int fd = open(NAND_LOG_PATH_FILE,O_CREAT);//�˴������ļ������� O_CREAT
//		if(fd >= 0)
//		{
//			rt_lprintf("%s�ļ������ɹ� fd=%d\n",NAND_LOG_PATH_FILE,fd);
//		}
//		else
//		{
//			rt_lprintf("%s�ļ�����ʧ�� fd=%d\n",NAND_LOG_PATH_FILE,fd);
//		}

//		
//		if(close(fd) != UENOERR)
//		{
//			rt_lprintf("[storage]:%s�ļ��ر�ʧ��\n",NAND_LOG_PATH_FILE);
//		}
//		else
//		{
//			rt_lprintf("[storage]:%s�ļ��رճɹ�\n",NAND_LOG_PATH_FILE);
//		}	
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/	
	memcpy(RouterIfo.AssetNum, "0011223344000000000011", sizeof(RouterIfo.AssetNum));// ���
    rt_lprintf("[storage]:����ʲ��ţ�%s\n",RouterIfo.AssetNum);	
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/	
	/* ���������� */
	storage_ReWr_mutex = rt_mutex_create("storage_ReWr_mutex", RT_IPC_FLAG_FIFO);
	if (storage_ReWr_mutex == RT_NULL)
	{
		rt_lprintf("[storage]:storage_ReWr_mutex����������ʧ��\n");
		return 0;
	}	
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/	
	rt_err_t res = rt_thread_init(&storage,
											"storage",
											storage_thread_entry,
											RT_NULL,
											storage_stack,
											THREAD_STORAGE_STACK_SIZE,
											THREAD_STORAGE_PRIORITY,
											THREAD_STORAGE_TIMESLICE);
	if (res == RT_EOK) 
	{
			rt_thread_startup(&storage);
	}
	return res;
}


#if defined (RT_STORAGE_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(storage_thread_init);
#endif
MSH_CMD_EXPORT(storage_thread_init, storage thread run);
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG����
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
int GetStorageData(STORAGE_CMD_ENUM cmd,void *Storage_Para,rt_uint32_t datalen)
{
	if(Para_Buff == NULL)
	{
	    Para_Buff = rt_malloc(MAX_MALLOC_NUM);//�����ڴ�
		if(Para_Buff == NULL)
		{
			rt_lprintf("[storage]:Para_Buff �����ڴ�ʧ��\n");
			return 1;
		}
		rt_lprintf("[storage]:Para_Buff rt_mallocOK\n");
	}
	rt_err_t ret = 0;	
	rt_err_t result = rt_mutex_take(storage_ReWr_mutex, RT_WAITING_FOREVER);
	
	switch(cmd)
	{
		case Cmd_MeterNumRd://
			ret = Router_Para_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,READ);
			// ���		
			*(char*)Storage_Para = ((*((char*)(RouterIfo.AssetNum)+10)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 11)-'0';//
			*(((char*)Storage_Para)+1) = ((*((char*)(RouterIfo.AssetNum)+12)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 13)-'0';//
			*(((char*)Storage_Para)+2) = ((*((char*)(RouterIfo.AssetNum)+14)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 15)-'0';//
			*(((char*)Storage_Para)+3) = ((*((char*)(RouterIfo.AssetNum)+16)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 17)-'0';//
			*(((char*)Storage_Para)+4) = ((*((char*)(RouterIfo.AssetNum)+18)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 19)-'0';//
			*(((char*)Storage_Para)+5) = ((*((char*)(RouterIfo.AssetNum)+20)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 21)-'0';//

			break;
		case Cmd_MeterPowerRd://
			ret = Meter_Power_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,READ);

			break;
		case Cmd_MeterGJFModeRd://
			ret = Meter_GJFMode_Storage(METER_GJFMode_PATH_FILE,Storage_Para,datalen,READ);	
			
			break;			
		case Cmd_MeterHalfPowerRd://
			ret = Meter_HalfPower_Storage(METER_HALF_POWER_PATH_FILE,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_MeterAnalogRd://
			ret = Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_HistoryRecordRd://
			ret = Charge_Record_Storage(HISTORY_RECORD_PATH,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_OrderChargeRd:/*�������¼���¼��Ԫ*/
			ret = Order_Charge_Storage(ORDER_CHARGE_PATH,Storage_Para,datalen,READ);	
			
			break;	
		case Cmd_PlanOfferRd:/*���ƻ��ϱ���¼��Ԫ*/
			ret = Plan_Offer_Storage(PLAN_OFFER_PATH,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_PlanFailRd:/*���ƻ�����ʧ�ܼ�¼��Ԫ*/
			ret = Plan_Fail_Storage(PLAN_FAIL_PATH,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_OnlineStateRd:/*�������״̬*/
			ret = Online_State_Storage(ONLINE_STATE_PATH,Storage_Para,datalen,READ);	
			
			break;
		default:
			rt_lprintf("[storage]:Waring��%s�յ�δ����ָ��%u\r\n",__FUNCTION__,cmd);
		    ret = 1;
			break;
	}	
	
	rt_free(Para_Buff);Para_Buff = NULL;
	
	if (result == RT_EOK)
	{
		/* �ͷŻ����� */
		rt_mutex_release(storage_ReWr_mutex);
	}
	else
	{
		rt_lprintf("[storage]:storage_ReWr_mutex �ͷŻ�����ʧ��\n");
	}
	
	return ret;
}
int SetStorageData(STORAGE_CMD_ENUM cmd,void *Storage_Para,rt_uint32_t datalen)
{
	if(Para_Buff == NULL)
	{
	  Para_Buff = rt_malloc(MAX_MALLOC_NUM);//�����ڴ�
		if(Para_Buff == NULL)
		{
			rt_lprintf("[storage]:Para_Buff �����ڴ�ʧ��\n");
			return 1;
		}
		rt_lprintf("[storage]:Para_Buff rt_mallocOK\n");
	}
	rt_err_t ret = 0;	
	rt_err_t result = rt_mutex_take(storage_ReWr_mutex, RT_WAITING_FOREVER);

	
	switch(cmd)
	{
		case Cmd_MeterNumWr://
			ret = Router_Para_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;	

		case Cmd_MeterPowerWr://
			ret = Meter_Power_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_MeterGJFModeWr://
			ret = Meter_GJFMode_Storage(METER_GJFMode_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;		
		case Cmd_MeterHalfPowerWr://
			ret = Meter_HalfPower_Storage(METER_HALF_POWER_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_MeterAnalogWr://
			ret = Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_HistoryRecordWr://
			ret = Charge_Record_Storage(HISTORY_RECORD_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_OrderChargeWr: /*�������¼���¼��Ԫ*/
			ret = Order_Charge_Storage(ORDER_CHARGE_PATH,Storage_Para,datalen,WRITE);	
			
			break;	
		case Cmd_PlanOfferWr:/*���ƻ��ϱ���¼��Ԫ*/
			ret = Plan_Offer_Storage(PLAN_OFFER_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_PlanFailWr:/*���ƻ�����ʧ�ܼ�¼��Ԫ*/
			ret = Plan_Fail_Storage(PLAN_FAIL_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_OnlineStateWr:/*�������״̬*/
			ret = Online_State_Storage(ONLINE_STATE_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		default:
			rt_lprintf("[storage]:Waring��%s�յ�δ����ָ��%u\r\n",__FUNCTION__,cmd);
		    ret = 1;
			break;	
	}
	
	rt_free(Para_Buff);Para_Buff = NULL;
	
	if (result == RT_EOK)
	{
		/* �ͷŻ����� */
		rt_mutex_release(storage_ReWr_mutex);
	}
	else
	{
		rt_lprintf("[storage]:storage_ReWr_mutex �ͷŻ�����ʧ��\n");
	}	
	
	return ret;
}
/**************************************************************
 * ��������: rt_uint8_t str2num(rt_uint8_t*src,rt_uint32_t *dest)
 * ��    ��: ��
 * �� �� ֵ: ��
 * ��    ��: ���ַ���תΪ����
 **************************************************************/	
//֧��16����ת��,��ʽΪ��0X����0x��ͷ��.
//��֧�ָ��� 
//*src:�����ַ���ָ��
//*dest:ת����Ľ����ŵ�ַ.
//����ֵ:0���ɹ�ת�����.����,�������.
//1,���ݸ�ʽ����.2,16����λ��Ϊ0.3,��ʼ��ʽ����.4,ʮ����λ��Ϊ0.
static rt_uint8_t str2num(rt_uint8_t*src,rt_uint32_t *dest)
{
	rt_uint32_t t;
	rt_uint8_t bnum=0;	//���ֵ�λ��
	rt_uint8_t *p;		  
	rt_uint8_t hexdec=10;//Ĭ��Ϊʮ��������
	p=src;
	*dest=0;//����.
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='F'&&*p>='A')||(*p=='X'&&bnum==1))//�����Ϸ�
		{
			if(*p>='A') hexdec=16;	//�ַ����д�����ĸ,Ϊ16���Ƹ�ʽ.
			bnum++;					//λ������.
		}
		else if(*p=='\0')
		    break;	                //����������,�˳�.
		else 
			return 1;				//��ȫ��ʮ���ƻ���16��������.
		p++; 
	} 
	p=src;			    //���¶�λ���ַ�����ʼ�ĵ�ַ.
	if(hexdec==16)		//16��������
	{
		if(bnum<3) return 2;			//λ��С��3��ֱ���˳�.��Ϊ0X��ռ��2��,���0X���治������,������ݷǷ�.
		if(*p=='0'&&((*(p+1)=='X')||(*(p+1)=='x')))//������'0X'��'0x'��ͷ.
		{
			p+=2;	//ƫ�Ƶ�������ʼ��ַ.
			bnum-=2;//��ȥƫ����	 
		}
		else 
			return 3;//��ʼͷ�ĸ�ʽ����
	}
	else if(bnum==0)
		return 4;//λ��Ϊ0��ֱ���˳�.	  
	while(1)
	{
		if(bnum) bnum--;
		if(*p<='9'&&*p>='0') t=*p-'0';	//�õ����ֵ�ֵ
		else t=*p-'A'+10;				//�õ�A~F��Ӧ��ֵ	    
		*dest += t*pow_df(hexdec,bnum);		   
		p++;
		if(*p=='\0')break;//���ݶ�������.	
	}
	return 0;//�ɹ�ת��
}
/**************************************************************
 * ��������: rt_uint8_t str2nnum(rt_uint8_t*src,rt_uint8_t *dest)
 * ��    ��: ��
 * �� �� ֵ: ��
 * ��    ��: ���ַ���תΪ����
 **************************************************************/	
//֧��16����ת��,����16������ĸ�����Ǵ�д��,�Ҹ�ʽΪ��0X��ͷ��.
//��֧�ָ��� 
//*src:�����ַ���ָ��
//*dest:ת����Ľ����ŵ�ַ.
//����ֵ:0���ɹ�ת�����.����,�������.
//1,���ݸ�ʽ����.2,16����λ��Ϊ0.3,��ʼ��ʽ����.4,ʮ����λ��Ϊ0.
static rt_uint8_t str2nnum(rt_uint8_t*src,rt_uint8_t *dest)
{
	rt_uint32_t t;
	rt_uint8_t bnum=0;	//���ֵ�λ��
	rt_uint8_t *p;		  
	rt_uint8_t hexdec=10;//Ĭ��Ϊʮ��������
	p=src;
	*dest=0;//����.
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='F'&&*p>='A')||(*p=='X'&&bnum==1))//�����Ϸ�
		{
			if(*p>='A') hexdec=16;	//�ַ����д�����ĸ,Ϊ16���Ƹ�ʽ.
			bnum++;					//λ������.
		}
		else if(*p=='\0')
		    break;	                //����������,�˳�.
		else 
			return 1;				//��ȫ��ʮ���ƻ���16��������.
		p++; 
	} 
	p=src;			    //���¶�λ���ַ�����ʼ�ĵ�ַ.
	hexdec=16;		    //16��������
	  
	while(1)
	{
		if(bnum) bnum--;
		if(*p<='9'&&*p>='0') t=*p-'0';	//�õ����ֵ�ֵ
		else t=*p-'A'+10;				//�õ�A~F��Ӧ��ֵ	    
		*dest += t*pow_df(hexdec,bnum);		   
		p++;
		if(*p=='\0') break;//���ݶ�������.	
	}
	return 0;//�ɹ�ת��
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG����
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
//m^n����
//����ֵ:m^n�η�
static rt_uint32_t pow_df(rt_uint8_t m,rt_uint8_t n)
{
	rt_uint32_t result=1;
	while(n--)result*=m;
	return result;
}
