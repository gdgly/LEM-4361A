#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_gpio.h"
#include "string.h"
#include "chargepile.h"
#include "stdlib.h"
#include "global.h"

#ifdef RT_USING_CAN

#ifndef FALSE
#define FALSE         0
#endif
#ifndef TRUE
#define TRUE          1
#endif

#ifndef SUCCESSFUL
#define SUCCESSFUL    0
#endif
#ifndef FAILED
#define FAILED        1
#endif

#ifndef TURN_ON
#define TURN_ON       0
#endif
#ifndef TURN_OFF
#define TURN_OFF      2
#endif

#define u8    uint8_t
#define u16   uint16_t
#define u32   uint32_t

#define delay_ms    rt_thread_mdelay

static u16 WaitStop_time = 0;
const u8 DestAdress = 0xF6;  	  //��������---Ŀ�ĵ�ַ
const u8 SrcAdress = 0x8A;        //��Դ·����---Դ��ַ
u16 Pro_Version = 0x0110;
static char CrjPileVersion[8] = {"V1.0.05"}; // �汾��
u8 software_date[8]={0};//����������� ��ʼ��Ϊ�ո�
u8 software_time[6]={0};//�������ʱ��
u32 can_heart_count = 0;
static char  USARTx_TX_BUF[256] = {0};
static char Printf_buff[8];
#define FlashBufLenMax             1024
__align(4) u8 STMFLASH_BUFF[FlashBufLenMax+2];
u32 STMFLASH_LENTH = 0;
static u32 RunTime = 0;
//////////////////////////////////////////////////////////////////////////////////
void CAN_V110_RecProtocal(void);
void Inform_Communicate_Can(uint8_t SendCmd,uint8_t Resend);
u8 CAN1_Send_Msg(struct rt_can_msg CanSendMsg,u8 len);
//////////////////////////////////////////////////////////////////////////////////	 
#define PriorityLeve0           0x00
#define PriorityLeve1           0x01
#define PriorityLeve2           0x02
#define PriorityLeve3           0x03
#define PriorityLeve4           0x04
#define PriorityLeve5           0x05
#define PriorityLeve6           0x06
#define PriorityLeve7           0x07
/////////////////////////////////////////////////////////////////////////////////////
/////////CAN----PGN�����ʽ//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
#define ChargeStartFrame           0x01
#define ChargeStartFrameAck        0x02
#define ChargeStopFrame            0x03
#define ChargeStopFrameAck         0x04
#define TimingFrame                0x05
#define TimingFrameAck             0x06
#define VertionCheckFrame          0x07
#define VertionCheckFrameAck       0x08
#define ChargeParaInfoFrame        0x09
#define ChargeParaInfoFrameAck     0x0A
#define ChargeServeOnOffFrame      0x0B
#define ChargeServeOnOffFrameAck   0x0C
#define ElecLockFrame              0x0D
#define ElecLockFrameAck           0x0E
#define PowerAdjustFrame           0x0F
#define PowerAdjustFrameAck        0x10
#define PileParaInfoFrame          0x60
#define PileParaInfoFrameAck       0x61

#define ChargeStartStateFrame      0x11
#define ChargeStartStateFrameAck   0x12
#define ChargeStopStateFrame       0x13
#define ChargeStopStateFrameAck    0x14

#define YcRecDataFrame             0x30
#define YcSendDataFrame            0x31
#define YxRecDataFrame             0x32
#define YxBackupSendDataFrame      0x33

#define HeartSendFrame             0x40
#define HeartRecFrame              0x41

#define SendErrorStateFrame        0x51
#define RecErrorStateFrame         0x52

#define FunPwmFrame                0xFE
////////////////////////////////////////////////////////////////////////////////////
#define UpdateBeginFrame           0x70
#define UpdateBeginFrameAck        0x71

#define DataSendReqFrame           0x72
#define DataSendReqFrameAck        0x73

#define DataSendFrame              0x74
#define DataSendFrameAck           0x75

#define UpdateCheckFrame           0x76
#define UpdateCheckFrameAck        0x77

#define ResetFrame                 0x78
#define ResetFrameAck              0x79
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/****************�궨��**********************************/
//�����������
typedef enum {
//	NO_FAULT            =0x00000000,
//	Connect_FAULT       =0x00000001,        //����Ӵ���״̬����		      
//	StopEct_FAULT       =0x00000002,        //��ͣ�����澯		      
//	Arrester_FAULT      =0x00000004,        //����������		
//	ACCir_FAULT         =0x00000008,        //�������߿���
//	Dooropen_FAULT      =0x00000010,        //���Ŵ򿪹���
//	CardOffLine_FAULT   =0x00000020,        //������ͨѶ�ж�
//	MeterOffLine_FAULT  =0x00000040,        //��ȱ�ͨѶ�ж�
//	BgStop_FAULT        =0x00000080,        //��̨ͨѶ�ж�
//	ViHigh_FAULT        =0x00000100,        //�����ѹ��ѹ
//	ViLow_FAULT         =0x00000200,        //�����ѹǷѹ
//	Check_FAULT         =0x00000400,        //Check״̬����
//	IoHigh_FAULT        =0x00000800,        //��������澯
//	GunTempHigh_FAULT   =0x00001000,        //���ǹ���¹���
	CanOffLine_FAULT    =0x00002000,        //CANͨѶ�ж�
				
} CHARGE_PILE_FAULT_TYPE;
//����������ͺ�������
char *err_string[] = 
{
           "                       ",               /* ERR_OK          0  */
           "����Ӵ���״̬���ϣ�   ",                 /* ERR             1  */
           "��ͣ�����澯��         ",                /* ERR             2  */
           "���������ϣ�	       ",                   /* ERR             3  */
           "�������߿��أ�         ",                /* ERR             4  */
           "���Ŵ򿪹��ϣ�         ",                /* ERR             5  */
           "������ͨѶ�жϣ�       ",                /* ERR             6  */
           "��ȱ�ͨѶ�жϣ�       ",                /* ERR             7  */
           "��̨ͨѶ�жϣ�         ",                /* ERR             8  */
           "�����ѹ��ѹ��         ",                /* ERR             9  */
           "�����ѹǷѹ��         ",                /* ERR             10 */
           "Check״̬���ϣ�        ",                /* ERR             11 */
           "��������澯��         ",                /* ERR             12 */
	       "���ǹ���¹��ϣ�       ",                /* ERR             13 */
           "GPRSͨѶ�жϣ�         ",                /* ERR             14 */	
};
///////////////////////////////////////////////////////////////////
typedef struct
{
	uint8_t  ChargIdent;                       // ���ӿڱ�ʶ ��ǹ ˫ǹ
	uint8_t  DeviceType;                       // �豸���� 0x01ֱ���������� 0x02������������ 0x03���ʿ���ģ�� 0x04���ģ�� 0x05����ģ��
    uint8_t  ChgSeviceState;                   // ϵͳ������״̬  1��ͣ��  2������
    uint8_t  ReplyState;                       // �ɹ���ʶ 0�ɹ� 1ʧ��
    uint8_t  LdSwitch;		                   // ���ɿ��ƿ���  1���� 2 �ر� 	
    uint8_t  RecStatus;                        // �������Ľ���״̬
	uint8_t  RecCount;                         // �������Ľ��ռ���
    uint8_t  SendCount;                        // �������ķ��ͼ���	
    uint8_t  reaonIdent;	                   // ʧ��ԭ��
    uint8_t  ConfIdent;                        // ȷ�ϱ�ʶ 0�ɹ� 1ʧ��
	uint8_t  StartReson;                       // ��ԭ��
	uint8_t  StopReson;                        // ͣԭ��
    uint8_t  ComUnitState;                     // ϵͳ�ϵ�ƥ��״̬  0 ����  1ƥ����  2 ƥ�����
	uint8_t  ResetAct;                         // ϵͳ�ϵ�ƥ��״̬  0xAA ��������  ������Ч
}STR_STATE_SYSTEM;                             // ϵͳ״̬֡��־λ
STR_STATE_SYSTEM StrStateSystem;   // ϵͳ״̬֡��־
///////////////////////////////////////////////////////////////////
typedef struct
{	
	u8 ChargeStartFrameReSendFlag;
	u8 ChargeStartFrameReSendCnt;	
	
	u8 ChargeStartFrameAckReSendFlag;
	u8 ChargeStartFrameAckReSendCnt;

	u8 ChargeStopFrameReSendFlag;
	u8 ChargeStopFrameReSendCnt;
	
	u8 ChargeStopFrameAckReSendFlag;
	u8 ChargeStopFrameAckReSendCnt; 	

	u8 TimingFrameReSendFlag;
	u8 TimingFrameReSendCnt;
	
	u8 TimingFrameAckReSendFlag;
	u8 TimingFrameAckReSendCnt;	

	u8 VertionCheckFrameReSendFlag; 
	u8 VertionCheckFrameReSendCnt;
	
	u8 VertionCheckFrameAckReSendFlag; 
	u8 VertionCheckFrameAckReSendCnt;	

	u8 ChargeParaInfoFrameReSendFlag;
	u8 ChargeParaInfoFrameReSendCnt;	
	
	u8 ChargeParaInfoFrameAckReSendFlag;
	u8 ChargeParaInfoFrameAckReSendCnt;	

	u8 ChargeServeOnOffFrameReSendFlag;
	u8 ChargeServeOnOffFrameReSendCnt;
	
	u8 ChargeServeOnOffFrameAckReSendFlag;
	u8 ChargeServeOnOffFrameAckReSendCnt;	

	u8 ElecLockFrameReSendFlag;
	u8 ElecLockFrameReSendCnt;
	
	u8 ElecLockFrameAckReSendFlag;
	u8 ElecLockFrameAckReSendCnt;	

	u8 PowerAdjustFrameReSendFlag; 
	u8 PowerAdjustFrameReSendCnt;
	
	u8 PowerAdjustFrameAckReSendFlag; 
	u8 PowerAdjustFrameAckReSendCnt;	


	u8 PileParaInfoFrameAckReSendFlag;
	u8 PileParaInfoFrameAckReSendCnt;	
    
	u8 ChargeStartStateFrameReSendFlag;
	u8 ChargeStartStateFrameReSendCnt;
	
	u8 ChargeStartStateFrameAckReSendFlag;
	u8 ChargeStartStateFrameAckReSendCnt;
	
	u8 ChargeStopStateFrameReSendFlag; 
    u8 ChargeStopStateFrameReSendCnt;
	
	u8 ChargeStopStateFrameAckReSendFlag; 
    u8 ChargeStopStateFrameAckReSendCnt;	

	u8 YcSendDataFrameReSendFlag; 
	u8 YcSendDataFrameReSendCnt;

	u8 YxBackupSendDataFrameReSendFlag;
	u8 YxBackupSendDataFrameReSendCnt;

	u8 SendErrorStateFrameReSendFlag;
	u8 SendErrorStateFrameReSendCnt;

	u8 UpdateBeginFrameAckReSendFlag;
	u8 UpdateBeginFrameAckReSendCnt;
	
	u8 DataSendReqFrameAckReSendFlag;
	u8 DataSendReqFrameAckReSendCnt;
	
	u8 DataSendFrameAckReSendFlag;
	u8 DataSendFrameAckReSendCnt;

	u8 UpdateCheckFrameAckReSendFlag;
	u8 UpdateCheckFrameAckReSendCnt;
	
	u8 ResetFrameAckReSendFlag;
	u8 ResetFrameAckReSendCnt;
}STR_STATE_FRAME;                       
STR_STATE_FRAME StrStateFrame;   //֡�ط�״̬
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	uint8_t   Num;				//���  16 01 10  liangbing
	uint8_t	  Elelock;          //���������� 1-���� 2-����

}STR_ELELOCK_CONTROL;
STR_ELELOCK_CONTROL strelelock;
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	uint8_t   FraNum;           //������֡��
	uint16_t  ValData;          //������Ч���ݳ���
	uint16_t  ChargVa;		    //����ѹ  1λС��
	uint16_t  ChargVb;		    //����ѹ  1λС��
	uint16_t  ChargVc;		    //����ѹ  1λС��
	uint16_t  ChargIa;          //������  2λС��
	uint16_t  ChargIb;          //������  2λС��
	uint16_t  ChargIc;          //������  2λС��
	uint16_t  V_cp1;            //������ѹ1 2λС��
	uint16_t  V_cp2;            //������ѹ2 2λС��
	uint16_t  Addition;			//�ۼӺ�
	u8  Tempt1;                 //�¶�1     1λС��
	u8  Tempt2;                 //�¶�2     1λС��	
	u8  Tempt3;                 //�¶�2     1λС��
	u8  Tempt4;                 //�¶�2     1λС��
}STR_YC; 
STR_YC strYC; // ң�����
///////////////////////////////////////////////////////////////////
//��Դ·�����·���ң������
typedef struct 				
{
	uint8_t   MesNum;           //��ǰ�������
	uint8_t   FraNum;           //������֡��
	uint16_t  ValData;          //������Ч���ݳ���
	uint16_t  Electricity;      //������  ��λ 0.1KWH
	uint16_t  ChrgTime;         //���ʱ��  ��λ 1 min
	uint16_t  Addition;			//�ۼӺ�
	uint16_t  TotalCheck;		//�ۼӺ�У����

}CHG_YC;
CHG_YC chgYC; //��Դ·�����·���ң�����
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	u8  WorkState;		   //����״̬������״̬  bit0-bit1������״̬  0����  1���� 2 ���  3��ͣ
	u8	TotalFau;          //�ܹ���
	u8	TotalAlm;          //�ܸ澯
	u8  ConnState;         //��������״̬  	    0 ���� 1δ����
	u8  StopEctFau;        //��ͣ��������        0 ���� 1�쳣
	u8	ArresterFau;       //����������	        0 ���� 1�쳣
    u8  GunOut;            //���ǹδ��λ		0 ���� 1�쳣
  	u8  ChgTemptOVFau;     //���׮���¹���		
	u8  VOVWarn;           //�����ѹ��ѹ
	u8  VLVWarn;           //�����ѹǷѹ
	u8  OutConState;       //����Ӵ���״̬      0������1�쳣
	u8  PilotFau;		   //����г������Ƶ�������        
	u8  ACConFau;		   //�����Ӵ�������	    0������1�쳣
	u8  OutCurrAla;		   //��������澯		            
	u8  OverCurrFau;	   //�����������	    0������1����
	u8  CurrentOutFlag;    //���������ʱ��ʶ	TRUE��ʼ����   FALSEֹͣ����
	u8  CurrentOutCount;   //���������ʱ��ʱ	
	u8  CCFau;	  		   //�����ǹͷ�쳣�Ͽ�			 							
	u8  ACCirFau;		   //������·������
	u8  LockState;		   //���ӿڵ�����״̬
	u8  LockFauState;	   //���ӿڵ���������״̬
	u8  GunTemptOVFau;     //���ӿڹ��¹���				
	u8  CC;				   //�������״̬CC����4    1ǹ��λ׮  0ǹ�뿪׮
	u8  CP;				   //������״̬CP����1
	u8  PEFau;			   //PE���߹���
	u8  DooropenFau;	   //���Ŵ򿪹���
	u8  ChgTemptOVAla;	   //���׮���¸澯				
	u8  GunTemptOVAla;	   //���ǹ���¸澯				
	u8  Contactoradjoin;   //�Ӵ���ճ��
	u8  GenAlaFau;		   //ͨ�ø澯�͹���
	u8  OthWarnNum;        //�����������
	u8  OthWarnVal;        //��������ֵ

}STR_YX;
STR_YX strYX; // ң�ű���
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	uint8_t  tCharParaAskTO          :1;   // �·�������Ӧ��ʱ
	uint8_t  tCharStartAskTO         :1;   // �����������Ӧ��ʱ
	uint8_t	 tWaitCharStartConTO     :1;   // �ȴ�����������״̬��ʱ
	uint8_t  tCharStopAskTO          :1;   // ���ֹͣ����Ӧ��ʱ
	uint8_t  tWaitCharStopConTO      :1;   // �ȴ����ֹͣ���״̬��ʱ
	uint8_t  tTimeAskTO              :1;   // ��ʱ����Ӧ��ʱ
	uint8_t  tChargeServeOnOffAskTO  :1;   // ��������ͣӦ��ʱ
	uint8_t  tElecLockAskTO          :1;   // ����������Ӧ��ʱ
	uint8_t  tPowerAdjustAskTO       :1;   // ��繦�ʵ���Ӧ��ʱ
	uint8_t  tPileParaInfoAskTO      :1;   // ���׮������Ϣ��ѯӦ��ʱ
	uint8_t	 tCharYXRecTO            :1;   // ң�ű��Ľ��ճ�ʱ
	uint8_t  tCharYCRecTO            :1;   // ң�ⱨ�Ľ��ճ�ʱ

}CHG_ERR_DATA;
CHG_ERR_DATA  ChgErrData; // ������������֡����
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/* ������״̬ */
typedef enum {
	state_PowerON=0,                  //0
	state_WaitVertionCheck,           //1
	state_WaitPilePara,               //2
	state_Standby,                    //3
	state_RecChargStartCmd,           //4
	state_ChargStart,                 //5
	state_ChargStartErr,              //6
	state_ActStartOK,                 //7
	state_ActStartErr,                //8
	state_ChargStartOK,               //9
	state_WaitChargeStartFrameAsk,    //10
	state_Charging,	                  //11
	state_ToCheck,                    //12
	state_AutoCheck,                  //13
	state_ActStopOK,                  //14
	state_ActStopErr,                 //15
	state_WaitStop,                   //16
	state_ErrStop,                    //17
	state_ChargEnd,                   //18
	state_Update,                     //19
	state_UpdateData,                 //20
	state_WaitChargeStopFrameAsk,
} CHG_PILE_STATE_E;
/****************���ϵͳ�ṹ��***********************/
typedef struct //
{
	CHG_PILE_STATE_E ChgState;
	unsigned char  ChgPasState;
	unsigned long  PWM_Duty;             //ռ�ձ�
	
	unsigned char  VOVWarn;              //�����ѹ��ѹ
	unsigned char  VLVWarn;              //�����ѹǷѹ
	unsigned char  OutCurrAla;			 //��������澯
	unsigned char  CheckState;           //����ȷ�Ͽ���״̬  0 δ���� 1���� 2���Գ�� 3״̬ת����
	unsigned char  GunTemptOVFau;		 //���ǹ���¸澯
	unsigned char  Contactoradjoin;	     //�Ӵ���ճ��

	unsigned long  TotalFau;             //�ܹ���
	unsigned char  ConnectState;         //����Ӵ���״̬    0�պϣ�1�Ͽ�
	
}CHGPILE_TypeDef;
static CHGPILE_TypeDef STR_ChargePile_A;
//////////////////////////////////////////////////////////////////////////////////
//��Դ·�����·�����������
typedef struct 				
{
	u8   UpdateConfIdent;          //ȷ�ϱ�ʶ 0x00���������� 0x01����ֹ���� ��������Ч
	u8   NoDownloadReason;         //��ֹ����ԭ��	0x00���� 0x01������֧�ִ˹��� 0x02�����ݺϷ���У��ʧ��	
	u8   MessageNum;               //��ǰ�������
	u8   FrameNum;                 //������֡��
	u16  ValDataLen;               //������Ч���ݳ���
	u16  AdditionUp;			   //�ۼӺ�
	u16  RevCheckUp;		       //�ۼӺ�У����
	u8   ErrResendCount;           //�����������ݰ�����ֵ

}CHG_UPDATE;
CHG_UPDATE chgUpdate; //��Դ·�����·�����������
//////////////////////////////////////////////////////////////////////////////////
/****************���������ṹ��***********************/
typedef struct //update�ṹ��
{
	u8   PileNum[8];               //׮��
	u32	 App_Mark;				   //������ʾ 1 ��������  0 δ����
	u32  App_Channel;			   //�������� 0�����ڣ�1��2G��2��4G��3��can
	u32  App_Mark_addr;            
	u8	 file_Format;              //0��Bin��1��Hex
	u8   App_UpdateNo[8];	       //����������
	u32  Update_Package_No;		   //�����ļ������к�
	u32  file_totalNo;             //�ļ��ܰ���
	u32  file_ByteNo;              //�ļ����ֽ���
	u32	 Update_Confirm;           //������־  0�ɹ�   1ʧ��
	u32  MajorVersion; 	           //�������屾��
	u32  MinorVersion;	           //�ΰ汾��
	u8   Update_Message_No;        //���ͳ�ʱ����
	u32  AdditionAll;			   //�ۼӺ�
	
}Update_TypeDef;
Update_TypeDef STR_ProgramUpdate;//���������ṹ��
/********************************************************/
/**************ģ����У׼�ṹ��***************************/
typedef struct    //ģ������������ṹ����
{
	u8 uUpDate;		        //�Ƿ��и���
	u32 uiVolAdj;           //��ѹУ׼*1000
	u32 uiCurAdj;           //����У׼*1000
	u32 uiConVolAdj;        //����ȷ�ϵ�ѹУ׼*1000
	u32 TempatatureAdj;		//�¶�У׼*1000
	
	u32 uiVolOverAlm;       //��ѹ����ֵ 1λС��
	u32 uiVolUnderAlm;		//Ƿѹ����ֵ 1λС��
	u32 uiCurAlm;			//��������ֵ 1λС��
	u32 TempatatureAlmPre;  //����Ԥ��ֵ 1λС��
	u32 TempatatureAlm;     //���±���ֵ 1λС��
	
	u32 pile_power;         //1λС����   75=7.5Kw

}YC_AnalogPara_TypeDef;
YC_AnalogPara_TypeDef STR_YC_Para_A;
/********************************************************/
/********************************************************/
/********************************************************/
#define THREAD_CHARGEPILE_PRIORITY     21
#define THREAD_CHARGEPILE_STACK_SIZE   1024
#define THREAD_CHARGEPILE_TIMESLICE    5

#define THREAD_CHARGEPILE_REV_PRIORITY    20
#define THREAD_CHARGEPILE_REV_STACK_SIZE   1024
#define THREAD_CHARGEPILE_REV_TIMESLICE    5

/* can�����¼���־*/
#define CAN_RX_EVENT 0x00000001U

#define RT_CHARGEPILE_CAN "can1"

static rt_device_t chargepile_can = RT_NULL;
static struct rt_event can_rx_event;
/* ������-�¼����ƿ� */
struct rt_event ChargePileEvent;
static struct rt_thread chargepile;
static struct rt_thread chargepileRev;
static rt_uint8_t chargepile_stack[THREAD_CHARGEPILE_STACK_SIZE];//�̶߳�ջ
static rt_uint8_t chargepileRevSend_stack[THREAD_CHARGEPILE_REV_STACK_SIZE];//�̶߳�ջ
static struct rt_can_msg g_RxMessage;



///////////////////////////////////////////////////////////////
static void timer_create_init(void);
/* ��ʱ���Ŀ��ƿ� */
static rt_timer_t CAN_Heart_Tx;
static rt_timer_t CAN_250ms_Tx;
static void CAN_Heart_Tx_callback(void* parameter);
static void CAN_250ms_Tx_callback(void* parameter);

ChargPilePara_TypeDef STR_ChargPilePara;

void *RetStructADDR[End_cmdListNum];
/**************************************************************
* ��������: ChargepileDataGetSet
* ��    ��: 
* �� �� ֵ: 
* ��    ��: 
***************************************************************/
rt_uint8_t ChargepileDataGetSet(COMM_CMD_P cmd,void *STR_SetGetPara)
{
	rt_uint8_t result = FAILED;
	
	switch(cmd)
	{
		case Cmd_ChargeStart://
			if(STR_ChargePile_A.ChgState == state_Standby)
			{
				STR_ChargePile_A.ChgState = state_ChargStart;
				result = SUCCESSFUL;
			}
			else
			{
				result = FAILED;
			}	
			
			break;
		case Cmd_ChargeStartResp://
			((ChargPilePara_TypeDef*)STR_SetGetPara)->StartReson = STR_ChargPilePara.StartReson;
			result = SUCCESSFUL;	
			
			break;
		case Cmd_ChargeStop://
			if(STR_ChargePile_A.ChgState == state_Charging)
			{
				STR_ChargePile_A.ChgState = state_ToCheck;
				result = SUCCESSFUL;
			}
			else
			{
				result = FAILED;
			}
		
			break;
		case Cmd_ChargeStopResp://
			((ChargPilePara_TypeDef*)STR_SetGetPara)->StopReson = STR_ChargPilePara.StopReson;
			result = SUCCESSFUL;	
			
			break;			
		case Cmd_SetPower://
			STR_ChargPilePara.PWM_Duty = ((ChargPilePara_TypeDef*)STR_SetGetPara)->PWM_Duty;
			result = SUCCESSFUL;
		
			break;
		case Cmd_GetPower://
			((ChargPilePara_TypeDef*)STR_SetGetPara)->PWM_Duty = STR_ChargPilePara.PWM_Duty;
			result = SUCCESSFUL;
		
			break;		
		case Cmd_RdVertion://
			result = SUCCESSFUL;
		
			break;
		case Cmd_RdVoltCurrPara://
			result = SUCCESSFUL;
		
			break;		
		default:
			rt_kprintf("Waring���յ�δ����ָ��%u\r\n",cmd);
			break;	
	}
	
	return  result;
}
/**************************************************************
* ��������: print_can_msg
* ��    ��: 
* �� �� ֵ: 
* ��    ��: 
***************************************************************/
void print_can_msg(struct rt_can_msg *data)
{
	sprintf((char*)USARTx_TX_BUF,"CAN1RecFrame:"); 
	sprintf((char*)Printf_buff,"%08X",data->id); 
	strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
	strcat((char*)USARTx_TX_BUF,(const char*)"--");

	for(int i=0;i<data->len;i++)   //ѭ����������
	{
		sprintf((char*)Printf_buff,"%02X",data->data[i]); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
	}
	rt_lprintf("%s\n",USARTx_TX_BUF);
}
/**************************************************************
* ��������: can_rx_ind
* ��    ��: 
* �� �� ֵ: 
* ��    ��: 
***************************************************************/
static rt_err_t can_rx_ind(rt_device_t device, rt_size_t len)
{
	rt_event_send(&can_rx_event, CAN_RX_EVENT);
	return RT_EOK;
}
/**************************************************************
* ��������: chargepile_thread_entry
* ��    ��: 
* �� �� ֵ: 
* ��    ��: 
***************************************************************/
static void chargepile_thread_entry(void *parameter)
{
	strYC.ChargVa = 2200;		    //����ѹ  1λС��
	strYC.ChargVb = 2200;		    //����ѹ  1λС��
	strYC.ChargVc = 2200;		    //����ѹ  1λС��
	STR_ChargePile_A.ChgState = state_WaitVertionCheck;
	StrStateSystem.LdSwitch = 0x01;	      // ���ɿ��ƿ���
	rt_thread_mdelay(100);
	while (1)
	{

		if((STR_ChargePile_A.ChgState == state_Standby)&&(RunTime%5 == 0x00))
		{
			ChargPilePara_TypeDef *STR_CHG_test = NULL;
			ChargepileDataGetSet(Cmd_ChargeStart,STR_CHG_test);
			rt_lprintf("[chargepile]:�·������������\n");
		}
		else if((STR_ChargePile_A.ChgState == state_Charging)&&(RunTime%10 == 0x00))
		{
			ChargPilePara_TypeDef *STR_CHG_test = NULL;
			ChargepileDataGetSet(Cmd_ChargeStop,STR_CHG_test);
			rt_lprintf("[chargepile]:�·�ֹͣ�������\n");
		}		
		
		switch(STR_ChargePile_A.ChgState)
		{
			case state_PowerON:// �ϵ�״̬
				rt_lprintf("chargepile:State_PowerON\n");
			
				break;			
			case state_WaitVertionCheck://�ȴ��汾У��Ӧ��
				Inform_Communicate_Can(VertionCheckFrame,FALSE);
				rt_lprintf("chargepile:State_WaitVertionCheck\n");
			
				break;
			case state_WaitPilePara://�ȴ����׮����Ӧ��
				Inform_Communicate_Can(ChargeParaInfoFrame,FALSE);
				rt_lprintf("chargepile:State_WaitPilePara\n");
			
				break;
			case state_Standby://��ʼ�����->����

		
//				if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT | ChargeStopOK_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,500, &ch) == RT_EOK)
//				{
//					rt_kprintf("chargepile:���յ�ChargePileEvent 0x%02X\n", ch);	
//				}
//				else
//				{
//					rt_kprintf("chargepile:δ����ChargePileEvent 0x%02X\n", ch);
//				}				
			
				rt_lprintf("chargepile:State_Standby\n");
			
				break;
			case state_ChargStart://���յ������������ ��ʼ�·��������
				Inform_Communicate_Can(ChargeStartFrame,FALSE);
				rt_lprintf("chargepile:ChargeStartFrame\n");
				STR_ChargePile_A.ChgState = state_WaitChargeStartFrameAsk;
			
				break;
			case state_WaitChargeStartFrameAsk://���յ������������ ��ʼ�·��������
				rt_lprintf("chargepile:State_WaitCharging\n");
			
				break;
			case state_Charging://�����ɹ��������
				rt_lprintf("chargepile:state_Charging\n");
			
				break;
			case state_ToCheck://���յ�ͣ������ ��ʼ�·�ͣ������
				Inform_Communicate_Can(ChargeStopFrame,FALSE);
				rt_lprintf("chargepile:ChargeStopFrame\n");
				STR_ChargePile_A.ChgState = state_WaitChargeStartFrameAsk;
			
				break;
			case state_WaitChargeStopFrameAsk://�ȴ�ֹͣ���Ӧ��֡
				rt_lprintf("chargepile:State_WaitChargeStopFrameAsk\n");
			
				break;
			case state_WaitStop://�ȴ�ֹͣ���Ӧ��֡
				rt_lprintf("chargepile:State_WaitStop\n");
			
				break;
			case state_ChargEnd://�ȴ�ֹͣ���Ӧ��֡
				memset(&StrStateFrame,0x00,sizeof(STR_STATE_FRAME));
				StrStateFrame.YcSendDataFrameReSendFlag = TRUE; 
				StrStateFrame.YxBackupSendDataFrameReSendFlag = TRUE; 

				STR_ChargePile_A.ChgState = state_Standby;
				rt_lprintf("chargepile:State_ChargEnd->State_Standby\n");
			
				break;
			default:
				rt_lprintf("chargepile:default\n");
				break;			
		}
//        extern rt_uint8_t cpu_usage_current,cpu_usage_max;
//		rt_lprintf("cpu_usage_current=%u%%,cpu_usage_max=%u%%\n",cpu_usage_current,cpu_usage_max);
//		rt_lprintf("RunTime=%u��\n",RunTime);
//		extern long list_thread(void);
//		list_thread();
//		extern void list_mem(void);
//		list_mem();
		
		/* lock scheduler */
		rt_enter_critical();
		/* unlock scheduler */
		rt_exit_critical();
//		rt_event_send(&ChargePileEvent, ChargeStartOK_EVENT);
//		rt_event_send(&ChargePileEvent, ChargeStartER_EVENT);
//		rt_uint32_t ch = 0;
//		if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,500, &ch) == RT_EOK)
//		{
//			rt_kprintf("chargepile:���յ�ChargePileEvent 0x%02X\n", ch);	
//		}
//		else
//		{
//			rt_kprintf("chargepile:δ����ChargePileEvent 0x%02X\n", ch);
//		}	
//        rt_base_t level = rt_hw_interrupt_disable();


//		rt_hw_interrupt_enable(level);


		rt_thread_mdelay(500);
	}
}
/**************************************************************
* ��������: chargepileRev_thread_entry
* ��    ��: 
* �� �� ֵ: 
* ��    ��: 
***************************************************************/
#define can1RevCycle    200
static void chargepileRev_thread_entry(void *parameter)
{
	rt_uint32_t err;
	rt_err_t res = RT_EOK;
    rt_uint32_t can1RevCycleCount = 0;
	while (1)
	{
		res = rt_event_recv(&can_rx_event, CAN_RX_EVENT, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 0, &err);
		if (res == RT_EOK)//û��������Ҫ���� RT_ETIMEOUT
		{
			while(rt_device_read(chargepile_can, 0, &g_RxMessage, sizeof(struct rt_can_msg)))
			{
				CAN_V110_RecProtocal();
			}			
		}
		else
		{
//			rt_lprintf("[chargepileRev]:û��can����res=%d\n",res);
		}

//        can1RevCycleCount++;
//		//��ʱ�ϴ�
//		if(can1RevCycleCount >= (1000/can1RevCycle))
//		{		
//			Inform_Communicate_Can(HeartSendFrame,FALSE);
//			can1RevCycleCount = 0;
//		}
//		rt_lprintf("[chargepileRev]:û��can����res=%d\n",res);
		rt_thread_mdelay(can1RevCycle);
	}
}
/**************************************************************
 * ��������: timer_create_init 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: CAN���ڻظ��ص�����  250ms���ڷ���
 **************************************************************/
int chargepile_thread_init(void)
{
	rt_err_t res;

	rt_kprintf("HEAP_BEGIN=0x%08X,HEAP_END=0x%08X\n",HEAP_BEGIN,HEAP_END);
    /* ��ʼ��������-�¼����� */
    rt_event_init(&ChargePileEvent, "ChargePileEvent", RT_IPC_FLAG_FIFO);	
	/* ��ʼ����ʱ�� */
	timer_create_init();
	
	
	struct can_configure config = CANDEFAULTCONFIG;
	chargepile_can = rt_device_find(RT_CHARGEPILE_CAN);
		
	if(chargepile_can != RT_NULL)
	{
		rt_device_control(chargepile_can, RT_CAN_CMD_SET_MODE, &config);
		if(rt_device_open(chargepile_can, (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX)) == RT_EOK)
		{
				//��ʼ���¼�����
			rt_event_init(&can_rx_event, "can_rx_event", RT_IPC_FLAG_FIFO);			
			rt_kprintf("[ChargePile]:Open %s sucess!\n",RT_CHARGEPILE_CAN);
		}
		else
		{
			rt_kprintf("[ChargePile]:%s Device open failed!\n",RT_CHARGEPILE_CAN);
		}
	}
	else
	{
		res = RT_ERROR;
		rt_kprintf("[ChargePile]:Open %s error\n",RT_CHARGEPILE_CAN);
		
		return res;
	}
	/* ���ջص�����*/
	rt_device_set_rx_indicate(chargepile_can, can_rx_ind);	
	
#ifdef BSP_USING_CAN1
	res=rt_thread_init(&chargepile,
						"chargepile",
						chargepile_thread_entry,
						RT_NULL,
						chargepile_stack,
						THREAD_CHARGEPILE_STACK_SIZE,
						THREAD_CHARGEPILE_PRIORITY,
						THREAD_CHARGEPILE_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&chargepile);
		rt_kprintf("[ChargePile]:chargepile_thread_entry sucess\r\n");
	}
	else
	{
		res = RT_ERROR;
		rt_kprintf("[ChargePile]:chargepile_thread_entry fail\r\n");
	}
/*********************************************************************************/	
	res=rt_thread_init(&chargepileRev,
						"chargepileRev",
						chargepileRev_thread_entry,
						RT_NULL,
						chargepileRevSend_stack,
						THREAD_CHARGEPILE_REV_STACK_SIZE,
						THREAD_CHARGEPILE_REV_PRIORITY,
						THREAD_CHARGEPILE_REV_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&chargepileRev);
		rt_kprintf("[ChargePile]:chargepileRev_thread_entry sucess\r\n");
	}
	else
	{
		res = RT_ERROR;
		rt_kprintf("[ChargePile]:chargepileRev_thread_entry fail\r\n");
	}
/*********************************************************************************/	
#endif /*BSP_USING_CAN1*/
	
	return res;
}


#if defined (RT_CHARGEOILE_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(chargepile_thread_init);
#endif
MSH_CMD_EXPORT(chargepile_thread_init, chargepile thread run);


/**************************************************************
 * ��������: timer_create_init 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: CAN���ڻظ��ص�����  250ms���ڷ���
 **************************************************************/
void timer_create_init()
{
    /* ������������ʱ�� */
	CAN_Heart_Tx = rt_timer_create("CAN_Heart_Tx",  /* ��ʱ�������� CAN_Heart_Tx */
									CAN_Heart_Tx_callback, /* ��ʱʱ�ص��Ĵ����� */
									RT_NULL, /* ��ʱ��������ڲ��� */
									1000, /* ��ʱ���ȣ���OS TickΪ��λ����1000��OS Tick */
									RT_TIMER_FLAG_PERIODIC); /* �����Զ�ʱ�� */
	
    /* ������������ʱ�� */
    if (CAN_Heart_Tx != RT_NULL)
        rt_timer_start(CAN_Heart_Tx);
    else
        rt_kprintf("CAN_Heart_Tx create error\n");

	
	/* �������ڻظ���ʱ�� */
	CAN_250ms_Tx = rt_timer_create("CAN_250ms_Tx",  /* ��ʱ�������� CAN_Heart_Tx */
									CAN_250ms_Tx_callback, /* ��ʱʱ�ص��Ĵ����� */
									RT_NULL, /* ��ʱ��������ڲ��� */
									250, /* ��ʱ���ȣ���OS TickΪ��λ����250��OS Tick */
									RT_TIMER_FLAG_PERIODIC); /* �����Զ�ʱ�� */

	/* ������������ʱ�� */
	if (CAN_250ms_Tx != RT_NULL)
		rt_timer_start(CAN_250ms_Tx);
	else
		rt_kprintf("CAN_250ms_Tx create error\n");
}
//**************************************************************
/**************************************************************
 * ��������: CAN_Heart_Tx_callback 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: CAN�������ص�����  1S���ڷ��� 
 **************************************************************/
static void CAN_Heart_Tx_callback(void* parameter)
{
    RunTime++;
	WaitStop_time++;

//	Inform_Communicate_Can(HeartSendFrame,FALSE);
//	if(StrStateFrame.YcSendDataFrameReSendFlag == TRUE)//ֹͣ�������ظ���־
//	{
//		Inform_Communicate_Can(YcSendDataFrame,FALSE);//ֹͣ�������ظ�
//	}	
}
/**************************************************************
 * ��������: CAN_250ms_Tx_callback 
 * ��    ��: 
 * �� �� ֵ: 
 * ��    ��: CAN���ڻظ��ص�����  250ms���ڷ���
 **************************************************************/
static void CAN_250ms_Tx_callback(void* parameter)
{
// 	if(StrStateFrame.ChargeStartFrameReSendFlag == TRUE)//�����������ظ���־
//	{	
//		Inform_Communicate_Can(ChargeStartFrame,FALSE);//
//		StrStateFrame.ChargeStartFrameReSendCnt++;
//		if(StrStateFrame.ChargeStartFrameReSendCnt >= 8)//����2s
//		{
//			StrStateFrame.ChargeStartFrameReSendFlag = FALSE;
//			StrStateFrame.ChargeStartFrameReSendCnt = 0;
//		}
//	}
//	
//	if(StrStateFrame.ChargeStopFrameReSendFlag == TRUE)//ֹͣ�������ظ���־
//	{
//		Inform_Communicate_Can(ChargeStopFrame,FALSE);//ֹͣ�������ظ�
//		StrStateFrame.ChargeStopFrameReSendCnt++;
//		if(StrStateFrame.ChargeStopFrameReSendCnt >= 8)//����2s
//		{
//			StrStateFrame.ChargeStopFrameReSendFlag = FALSE;
//			StrStateFrame.ChargeStopFrameReSendCnt = 0;
//		}
//	}
	
//	rt_kprintf("CAN_250ms_Tx_callback\n");
}
/***************************************************************
* ��������: Inform_Communicate_Can(uint8_t SendCmd,uint8_t Resend)
* ��    ��:
* �� �� ֵ:
* ��    ��: ����������Ʒѵ�Ԫ��������
***************************************************************/
void Inform_Communicate_Can(uint8_t SendCmd,uint8_t Resend)
{
	uint8_t i;
	uint8_t PriorityLeve,DataLength;
	uint16_t temp1;
	struct rt_can_msg TxBuf; // CAN�ڷ��ͱ��Ľṹ��
	switch(SendCmd)
	{
		case ChargeStartFrame:
		{
			//��Դ·�����������������"�������"������ȼ�0X04��PF��0X01
			PriorityLeve = PriorityLeve4;

			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;      // ���ӿڱ�ʶ
			TxBuf.data[1] =  StrStateSystem.LdSwitch&0xFF;	      // ���ɿ��ƿ���	
			DataLength = 0x02;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeStartFrameReSendFlag = TRUE;
				StrStateFrame.ChargeStartFrameReSendCnt = 0;	
			}			
			
			break;
		}
		case ChargeStopFrame:
		{			
			//��Դ·�����������������"ֹͣ���"�����ȼ�0X04��PF��0X03��
			PriorityLeve = PriorityLeve4;	   //���ȼ�4		
						
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;	  //���ӿڱ�ʶ
			TxBuf.data[1] =  StrStateSystem.StopReson;            //ֹͣ���ԭ�� 01����ֹͣ 02�������ֹͣ 03���������ֹͣ
			DataLength = 0x02;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeStopFrameReSendFlag = TRUE;
				StrStateFrame.ChargeStopFrameReSendCnt = 0;
			}			

			break;
		}
		case TimingFrame:
		{			
			//��Դ·�����������������"Уʱ"�����ȼ�0X06��PF��0X05��
			PriorityLeve = PriorityLeve6;	   //���ȼ�6
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress
            temp1 = System_Time_STR.Second*1000;
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;                          // ���ӿڱ�ʶ
			TxBuf.data[1] =  temp1&0xFF;                                              // ����  ��8λ
			TxBuf.data[2] =  (temp1>>8)&0xFF;                                         // ����  ��8λ	  
			TxBuf.data[3] =  System_Time_STR.Minute&0x3F;                             // ��
			TxBuf.data[4] =  System_Time_STR.Hour&0x1F;                               // Сʱ	
			TxBuf.data[5] =  (System_Time_STR.Week<<5)+(System_Time_STR.Day&0x1F);    // 0-4���� 5-7����
			TxBuf.data[6] =  System_Time_STR.Month&0x0F;                              // ��
			TxBuf.data[7] =  System_Time_STR.Year&0x7F;                               // 0-6λ��	
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.TimingFrameReSendFlag = TRUE;
				StrStateFrame.TimingFrameReSendCnt = 0;
			}			

			break;
		}		
		case TimingFrameAck:
		{			
			//������������Դ·��������"Уʱ"����ȷ�ϣ����ȼ�0X06��PF��0X06
			PriorityLeve = PriorityLeve6;	   //���ȼ�6
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress
            temp1 = System_Time_STR.Second*1000;
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;                          // ���ӿڱ�ʶ
			TxBuf.data[1] =  temp1&0xFF;                                              // ����  ��8λ
			TxBuf.data[2] =  (temp1>>8)&0xFF;                                         // ����  ��8λ	  
			TxBuf.data[3] =  System_Time_STR.Minute&0x3F;                             // ��
			TxBuf.data[4] =  System_Time_STR.Hour&0x1F;                               // Сʱ	
			TxBuf.data[5] =  (System_Time_STR.Week<<5)+(System_Time_STR.Day&0x1F);    // 0-4���� 5-7����
			TxBuf.data[6] =  System_Time_STR.Month&0x0F;                              // ��
			TxBuf.data[7] =  System_Time_STR.Year&0x7F;                               // 0-6λ��	
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.TimingFrameAckReSendFlag = TRUE;
				StrStateFrame.TimingFrameAckReSendCnt = 0;
			}			

			break;
		}
		case VertionCheckFrame:
		{
			//��Դ·�����������������"�汾У��"�����ȼ�0X06��PF��0X07��
			PriorityLeve = PriorityLeve6;	   //���ȼ�6
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress

			TxBuf.data[0] = StrStateSystem.ChargIdent;	  //���ӿڱ�ʶ
			TxBuf.data[1] =  Pro_Version&0xFF;     // �ΰ屾��
			TxBuf.data[2] = (Pro_Version>>8)&0xFF; // ���汾��
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.VertionCheckFrameReSendFlag = TRUE;
				StrStateFrame.VertionCheckFrameReSendCnt = 0;
			}			
			
			break;
		}		
//		case VertionCheckFrameAck:
//		{
//			//������������Դ·��������"�汾У��"����ȷ�ϣ����ȼ�0X06��PF��0X08
//			PriorityLeve = PriorityLeve6;	   //���ȼ�6
//			
//			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress

//			TxBuf.data[0] = StrStateSystem.ChargIdent;	  //���ӿڱ�ʶ
//			TxBuf.data[1] =  Pro_Version&0xFF;     // �ΰ屾��
//			TxBuf.data[2] = (Pro_Version>>8)&0xFF; // ���汾��
//			TxBuf.data[3] = 0x00; // ��֧�ֵ�bmsͨ��Э��� 	  
//			TxBuf.data[4] = (((CrjPileVersion[5]-0x30)<<4)|(CrjPileVersion[6]-0x30))&0xFF;//���а汾��
//			TxBuf.data[5] = (CrjPileVersion[3]-0x30)&0xFF;//�ΰ汾��	
//			TxBuf.data[6] = (CrjPileVersion[1]-0x30)&0xFF;//���汾��
//			DataLength = 0x07;
//			CAN1_Send_Msg(TxBuf,DataLength);
//			if(Resend == TRUE)
//			{
//				StrStateFrame.VertionCheckFrameAckReSendFlag = TRUE;
//				StrStateFrame.VertionCheckFrameAckReSendCnt = 0;
//			}			
//			
//			break;
//		}
		case ChargeParaInfoFrame:
		{
			//��Դ·�����������������"������"�����ȼ�0X06��PF��0X09��
			PriorityLeve = PriorityLeve6;	   //���ȼ�6
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;

			TxBuf.data[0] = StrStateSystem.ChargIdent; //����ȷ�ϳɹ���ʶ Ĭ��Ϊ0
			
			for(i=1;i<8;i++)
			{
				TxBuf.data[i] = STR_ProgramUpdate.PileNum[i];	
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeParaInfoFrameReSendFlag = TRUE;
				StrStateFrame.ChargeParaInfoFrameReSendCnt = 0;
			}				
			
			break;
		}
		case ChargeServeOnOffFrame: 
		{		
			//��Դ·�����������������"���������"�����ȼ�0X04��PF��0X0B��
            PriorityLeve = PriorityLeve4;  

            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] = StrStateSystem.ChargIdent&0xFF;//���ӿڱ�ʶ
			TxBuf.data[1] = 0x02&0xFF;//����������ֹͣ����ָ�� strChgSevCtrl.ChargSeveiceCmd 01ֹͣ 02���� ���� ��Ч
            DataLength = 0x02;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeServeOnOffFrameReSendFlag = TRUE;
				StrStateFrame.ChargeServeOnOffFrameReSendCnt = 0;
			}			

            break;
		}		
		case ChargeServeOnOffFrameAck: 
		{		
			//������������Դ·��������"���������"����ȷ�ϣ����ȼ�0X04��PF��0X0C
            PriorityLeve = PriorityLeve4;  

            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;//���ӿڱ�ʶ
//			TxBuf.data[1] =  strChgSevCtrl.ChargSeveiceCmd&0xFF;//����ָ��
//			TxBuf.data[2] =  strChgSevCtrl.CtrlState&0xFF;//ִ�н��
//			TxBuf.data[3] =  strChgSevCtrl.Ctrreaon&0xFF;
            DataLength = 0x04;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeServeOnOffFrameAckReSendFlag = TRUE;
				StrStateFrame.ChargeServeOnOffFrameAckReSendCnt = 0;
			}			

            break;
		}
		case ElecLockFrame:
		{			
			//��Դ·�����������������"����������"�����ȼ�0X04��PF��0X0D
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;// ���ӿڱ�ʶ
            TxBuf.data[1] =  strelelock.Num&0xFF;       // ���������
            TxBuf.data[2] =  strelelock.Elelock&0xFF;   // ����ָ��
            DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ElecLockFrameReSendFlag = TRUE;
				StrStateFrame.ElecLockFrameReSendCnt = 0;
			}			

            break;
		}		
		case ElecLockFrameAck:
		{			
			//������������Դ·��������"����������"����ȷ�ϣ����ȼ�0X04��PF��0X0E
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;// ���ӿڱ�ʶ
            TxBuf.data[1] =  strelelock.Num&0xFF;       // ���������
            TxBuf.data[2] =  strelelock.Elelock&0xFF;   // ����ָ��
            TxBuf.data[3] =  StrStateSystem.ReplyState;  // ִ�н��;
            TxBuf.data[4] =  StrStateSystem.reaonIdent&0xFF; // ʧ��ԭ��
            DataLength = 0x05;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ElecLockFrameAckReSendFlag = TRUE;
				StrStateFrame.ElecLockFrameAckReSendCnt = 0;
			}			

            break;
		}
		case PowerAdjustFrame:
		{
			//��Դ·�����������������"���ʵ���"�����ȼ�0X04��PF��0X0F
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;           // ���ӿڱ�ʶ
//			TxBuf.data[1] =  strcapacity.C_capacity_style;             // ���ʵ�������
//			TxBuf.data[2] =  strcapacity.C_capacity_value&0xFF;	       // ���ʵ���ֵ
//			TxBuf.data[3] =  (strcapacity.C_capacity_value>>8)&0xFF;
            DataLength = 0x04;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.PowerAdjustFrameReSendFlag = TRUE;
				StrStateFrame.PowerAdjustFrameReSendCnt = 0;
			}				

            break;
		}		
		case PowerAdjustFrameAck:
		{
			//������������Դ·��������"���ʵ���"����ȷ�ϣ����ȼ�0X04��PF��0X10
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;           // ���ӿڱ�ʶ
//			TxBuf.data[1] =  strcapacity.C_capacity_style;             // ���ʵ�������
//			TxBuf.data[2] =  strcapacity.C_capacity_value&0xFF;	       // ���ʵ���ֵ
//			TxBuf.data[3] =  (strcapacity.C_capacity_value>>8)&0xFF;
            TxBuf.data[4] =  StrStateSystem.ReplyState;                // �ɹ���ʶ 1�ɹ� 2ʧ��
			TxBuf.data[5] =  StrStateSystem.reaonIdent;                // ʧ��ԭ��
            DataLength = 0x06;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.PowerAdjustFrameAckReSendFlag = TRUE;
				StrStateFrame.PowerAdjustFrameAckReSendCnt = 0;
			}				

            break;
		}
		case ChargeStartStateFrameAck:
		{
			//��Դ·�����������������"�������״̬"����ȷ�ϣ����ȼ�0X04��PF��0X12
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF; // ���ӿڱ�ʶ
			TxBuf.data[1] =  StrStateSystem.LdSwitch&0xFF;   // ���ɿ��ƿ���
			TxBuf.data[2] =  StrStateSystem.ReplyState;      // ����������״̬֡�ɹ���ʶ
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeStartStateFrameAckReSendFlag = TRUE;
				StrStateFrame.ChargeStartStateFrameAckReSendCnt = 0;
			}				
			
			break;
		}
		case ChargeStopStateFrameAck:
		{
			//��Դ·�����������������"���ֹͣ״̬"����ȷ�ϣ����ȼ�0X04��PF��0X14
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;	//���ӿڱ�ʶ
			TxBuf.data[1] =  StrStateSystem.StopReson&0xFF;     //ֹͣԭ��0x01-��Դ·��������ֹͣ���   0x02-���׮���ϣ�������������������ֹ���
			TxBuf.data[2] =  StrStateSystem.ReplyState;         //ֹͣ������״̬֡�ɹ���ʶ
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
			StrStateFrame.ChargeStopStateFrameAckReSendFlag = TRUE;
			StrStateFrame.ChargeStopStateFrameAckReSendCnt = 0;
			}			

			break;
		}
		case YcSendDataFrame://��Դ·������������������Է���ң�����ݣ����ȼ�0X06��PF0X31��
		{
			PriorityLeve = PriorityLeve6;  
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 
			strYC.ValData = 0x15;  // 
			TxBuf.data[0] = 0x01; // ��ǰ�������
			TxBuf.data[1] = 0x04; // ������֡��
			TxBuf.data[2] = strYC.ValData&0xFF;      // ������Ч���ݳ��ȵ��ֽ�
			TxBuf.data[3] = (strYC.ValData>>8)&0xFF; // ������Ч���ݳ��ȸ��ֽ� 
			TxBuf.data[4] = StrStateSystem.ChargIdent&0xFF; // ���ӿڱ�ʶ	  
			TxBuf.data[5] = strYC.ChargVa&0xFF;  // A���ѹ
			TxBuf.data[6] = (strYC.ChargVa>>8)&0xFF;	  
			TxBuf.data[7] = strYC.ChargVb&0xFF;  // B���ѹ
			strYC.Addition = 0x0000;
			for(i=1;i<8;i++)
			{
				strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			
			delay_ms(10);

			TxBuf.data[0] = 0x02; // ��ǰ�������
			TxBuf.data[1] = (strYC.ChargVb>>8)&0xFF;
			TxBuf.data[2] = strYC.ChargVc&0xFF; // C���ѹ
			TxBuf.data[3] = (strYC.ChargVc>>8)&0xFF;
			TxBuf.data[4] = strYC.ChargIa&0xFF; // A�����
			TxBuf.data[5] = (strYC.ChargIa>>8)&0xFF;
			TxBuf.data[6] = strYC.ChargIb&0xFF; // B�����
			TxBuf.data[7] = (strYC.ChargIb>>8)&0xFF;
			for(i=1;i<8;i++)
			{
				strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(10);
			 
			TxBuf.data[0] =  0x03;//��ǰ�������
			TxBuf.data[1] =  strYC.ChargIc&0xFF;                    //C�����
			TxBuf.data[2] =  (strYC.ChargIc>>8)&0xFF;	            //
			TxBuf.data[3] =  chgYC.Electricity&0xFF;                //������  
			TxBuf.data[4] =  (chgYC.Electricity>>8)&0xFF;
			TxBuf.data[5] =  chgYC.ChrgTime&0xFF;                   //���ʱ��
			TxBuf.data[6] =  (chgYC.ChrgTime>>8)&0xFF;	            //

			for(i=1;i<7;i++)
			{
				strYC.Addition += TxBuf.data[i];
			}
			TxBuf.data[7] =  strYC.Addition&0xff;      // ������Ч���ݳ��ȵ��ֽ�
			DataLength = 0x08;	
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(10);		
			 
			TxBuf.data[0] = 0x04;//��ǰ�������
			TxBuf.data[1] =  (strYC.Addition>>8)&0xFF; // ������Ч���ݳ��ȸ��ֽ�

			DataLength = 0x02;
			CAN1_Send_Msg(TxBuf,DataLength);				

			break;
		}													   
		case HeartSendFrame://������������Դ·���������Է����������ݣ����ȼ�0X06��PF:0X40��
		{			 
			PriorityLeve = PriorityLeve6;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;   // ���ӿڱ�ʶ
			TxBuf.data[1] =  StrStateSystem.SendCount&0xFF;    // �������ķ��ͼ���
			if((STR_ChargePile_A.TotalFau&CanOffLine_FAULT) == CanOffLine_FAULT) // ͨ�ų�ʱ
			{
                TxBuf.data[2] = 0x01; // ͨѶ��ʱ
			}
			else
			{											  
                TxBuf.data[2] = 0x00; // ͨѶ�ɹ�
			}			
			
			TxBuf.data[3] = 0x02;// 1:������״̬ ͣ��  2������
			DataLength = 0x04;
			CAN1_Send_Msg(TxBuf,DataLength);
			StrStateSystem.SendCount++;   //�ӵ�255 �Զ��������

			break;
		}
		case SendErrorStateFrame://����������������֡�����ȼ�0X04��PF:0X52��
		{
		 	PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;//���ӿڱ�ʶ
							/* �·�������Ӧ��ʱ */
							/* �����������Ӧ��ʱ*/
							/* �ȴ�����������״̬��ʱ*/
							/* ���ֹͣ����Ӧ��ʱ*/
							/* �ȴ����ֹͣ���״̬��ʱ*/
							/* ��ʱ����Ӧ��ʱ*/
							/* ��������ͣӦ��ʱ*/
			TxBuf.data[1] = (ChgErrData.tCharParaAskTO<<1|\
							 ChgErrData.tCharStartAskTO<<2|\
							 ChgErrData.tWaitCharStartConTO<<3|\
							 ChgErrData.tCharStopAskTO<<4|\
							 ChgErrData.tWaitCharStopConTO<<5|\
							 ChgErrData.tTimeAskTO<<6|\
							 ChgErrData.tChargeServeOnOffAskTO<<7)&0xFF;
							/* ����������Ӧ��ʱ*/
							/* ��繦�ʵ���Ӧ��ʱ*/		
							/* ���׮������Ϣ��ѯӦ��ʱ*/
							/* ң�ű��Ľ��ճ�ʱ*/
							/* ң�ⱨ�Ľ��ճ�ʱ*/	
			TxBuf.data[2] = (ChgErrData.tElecLockAskTO|\
							 ChgErrData.tPowerAdjustAskTO<<1|\
							 ChgErrData.tPileParaInfoAskTO<<2|\
							 ChgErrData.tCharYXRecTO<<3|\
							 ChgErrData.tCharYCRecTO<<4)&0xFF;			 
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.SendErrorStateFrameReSendFlag = TRUE;
				StrStateFrame.SendErrorStateFrameReSendCnt = 0;
			}
			break;
		}
		case PileParaInfoFrameAck:
		{
			PriorityLeve = PriorityLeve6;	   //���ȼ�6  wyg170510
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress

			TxBuf.data[0] = 0x01;//��ǰ�������
			TxBuf.data[1] = 0x06;//������֡��
			TxBuf.data[2] = 0x25;//������Ч���ݳ��ȵ��ֽ�
			TxBuf.data[3] = 0x00;//������Ч���ݳ��ȸ��ֽ�
			TxBuf.data[4] = StrStateSystem.ChargIdent;	  //���ӿڱ�ʶ
			TxBuf.data[5] = 0xEA;	 //���ұ���
			TxBuf.data[6] = 0x03;
			TxBuf.data[7] = 0x00;
			strYC.Addition = 0;
			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
				  
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);

			TxBuf.data[0] = 0x02;   //��ǰ�������
			TxBuf.data[1] = 0x00;
			TxBuf.data[2] = 0x80;	//�豸�ͺ�A
			TxBuf.data[3] = 0x52;	//
			TxBuf.data[4] = 0x02;   //BMSЭ��汾��
			TxBuf.data[5] = 0xEA;	//�����������кţ�1~4�����̱��룩
			TxBuf.data[6] = 0x03;	
			TxBuf.data[7] = 0x00;

			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);

			TxBuf.data[0] = 0x03; //��ǰ�������
			TxBuf.data[1] = 0x00; 	
			TxBuf.data[2] = 0x17;//�����������кţ�5~6��������ݣ�
			TxBuf.data[3] = 0x20;
			TxBuf.data[4] = 0x34;//�����������кţ�7~8���������Σ�
			TxBuf.data[5] = 0x12;	
			TxBuf.data[6] = 0x78;//�����������кţ�9~12��������ţ�
			TxBuf.data[7] = 0x56; 			

			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);
			
			
			TxBuf.data[0] = 0x04; //��ǰ�������
			TxBuf.data[1] = 0x34;
			TxBuf.data[2] = 0x12;
			TxBuf.data[3] = 0x00; //Ӳ���ΰ汾��
			TxBuf.data[4] = 0x01; //Ӳ�����汾��
			TxBuf.data[5] = (((CrjPileVersion[5]-0x30)<<4)|(CrjPileVersion[6]-0x30))&0xFF;//���а汾��  {"V1.0.00"}; // �汾��	 		
			TxBuf.data[6] = (CrjPileVersion[3]-0x30)&0xFF;//�ΰ汾��
			TxBuf.data[7] = (CrjPileVersion[1]-0x30)&0xFF;//���汾��
			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);
			
			TxBuf.data[0] = 0x05;//��ǰ�������
			TxBuf.data[1] = ((software_date[6]-'0')<<4)|(software_date[7]-'0');//��  LCF190307
			TxBuf.data[2] = ((software_date[4]-'0')<<4)|(software_date[5]-'0');//0x20	
			TxBuf.data[3] = ((software_date[0]-'0')<<4)|(software_date[1]-'0');//��	  
			TxBuf.data[4] = ((software_date[2]-'0')<<4)|(software_date[3]-'0');//��
			TxBuf.data[5] = STR_YC_Para_A.uiVolOverAlm&0xFF;			//��������ѹ
			TxBuf.data[6] = (STR_YC_Para_A.uiVolOverAlm>>8)&0xFF;
			TxBuf.data[7] = STR_YC_Para_A.uiVolUnderAlm&0xFF;			//��������ѹ		

			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);			
			
			TxBuf.data[0] = 0x06;//��ǰ�������
			TxBuf.data[1] = (STR_YC_Para_A.uiVolUnderAlm>>8)&0xFF; 
			TxBuf.data[2] = (STR_YC_Para_A.uiCurAlm/10)&0xFF;		//���������� 	
			TxBuf.data[3] = ((STR_YC_Para_A.uiCurAlm/10)>>8)&0xFF; 	 
			TxBuf.data[4] = 0x00;					  				//��С�������         
			TxBuf.data[5] = 0x00;	
			for(i=1;i<6;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			
			TxBuf.data[6] =  strYC.Addition&0xff;      // ������֡��
			TxBuf.data[7] = (strYC.Addition>>8)&0xFF;  // ������Ч���ݳ��ȵ��ֽ�			

			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);
			
			if(Resend == TRUE)
			{
				StrStateFrame.PileParaInfoFrameAckReSendFlag = TRUE;
				StrStateFrame.PileParaInfoFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case UpdateBeginFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;
//	        TxBuf.data[0] = CrjPileVersion[3] - '0';//����汾��  ���ֽ�
//	        TxBuf.data[1] = CrjPileVersion[6] - '0';
	        TxBuf.data[0] = StrStateSystem.ChargIdent;//���ӿڱ�ʶ
	        TxBuf.data[1] = StrStateSystem.DeviceType;//�豸����
	        TxBuf.data[2] = chgUpdate.UpdateConfIdent;//ȷ�ϱ�ʶ 0x00���������� 0x01����ֹ���� ��������Ч
	        TxBuf.data[3] = chgUpdate.NoDownloadReason;//��ֹ����ԭ��	0x00���� 0x01������֧�ִ˹��� 0x02�����ݺϷ���У��ʧ��		
			DataLength = 0x04;
			CAN1_Send_Msg(TxBuf,DataLength);
			
			if(Resend == TRUE)
			{
				StrStateFrame.UpdateBeginFrameAckReSendFlag = TRUE;
				StrStateFrame.UpdateBeginFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case DataSendReqFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//���ӿڱ�ʶ
	        TxBuf.data[1] = StrStateSystem.DeviceType;//�豸����

	        TxBuf.data[2] = 0x00;//ȷ�ϱ�ʶ 0x00�������� 0x01����ֹ���� ��������Ч	
	        TxBuf.data[3] = STR_ProgramUpdate.file_totalNo&0xFF;//�ܰ���
	        TxBuf.data[4] = (STR_ProgramUpdate.file_totalNo>>8)&0xFF;	
			
	        TxBuf.data[5] = STR_ProgramUpdate.Update_Package_No&0xFF;//��ȡ����
	        TxBuf.data[6] = (STR_ProgramUpdate.Update_Package_No>>8)&0xFF;			 	
			DataLength = 0x07;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.DataSendReqFrameAckReSendFlag = TRUE;
				StrStateFrame.DataSendReqFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case DataSendFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//���ӿڱ�ʶ
	        TxBuf.data[1] = StrStateSystem.DeviceType;//�豸����
			
	        TxBuf.data[2] = chgUpdate.UpdateConfIdent; //ȷ�ϱ�ʶ 0x00���������� 0x01����ֹ���� ��������Ч
	        TxBuf.data[3] = chgUpdate.NoDownloadReason;//��ֹ����ԭ��	0x00���� 0x01������֧�ִ˹��� 0x02�����ݺϷ���У��ʧ��			
			
	        TxBuf.data[4] = STR_ProgramUpdate.Update_Package_No&0xFF;//��ȡ����
	        TxBuf.data[5] = (STR_ProgramUpdate.Update_Package_No>>8)&0xFF;			 	
			DataLength = 0x06;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.DataSendFrameAckReSendFlag = TRUE;
				StrStateFrame.DataSendFrameAckReSendCnt = 0;
			}
			
			break;
		}		
		case UpdateCheckFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//���ӿڱ�ʶ
	        TxBuf.data[1] = StrStateSystem.DeviceType;//�豸����
	        TxBuf.data[2] = chgUpdate.UpdateConfIdent;//ȷ�ϱ�ʶ 0x00���ɹ� 0x01��ʧ�� ��������Ч	
	        TxBuf.data[3] = chgUpdate.NoDownloadReason;//ʧ��ԭ��	0x00���� 0x01�����鲻�ɹ�
			DataLength = 0x04;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.UpdateCheckFrameAckReSendFlag = TRUE;
				StrStateFrame.UpdateCheckFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case ResetFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//���ӿڱ�ʶ
	        TxBuf.data[1] = StrStateSystem.DeviceType;//�豸����

	        TxBuf.data[2] = 0xAA;//ȷ�ϱ�ʶ 0xAA���ɹ� ��������Ч	
			 	
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ResetFrameAckReSendFlag = TRUE;
				StrStateFrame.ResetFrameAckReSendCnt = 0;
			}
			
			break;
		}
		default:
		{
			break;
		}
	}	
}
/********************************************************************  
����һ֡ͨѶ����  
ԭ�ͣ�CAN_V110_RecProtocal(void)
���ܣ�����һ֡ͨѶ  
��ڲ�����  
���ڲ�����������ȷ��־��0x00Ϊ���մ�����, 0x01�ɹ� 0x02ʧ��
********************************************************************/ 
void CAN_V110_RecProtocal(void)
{
	static rt_uint32_t temp = 0,i = 0;
	static rt_uint8_t tempanalog = 0;
    struct rt_can_msg* pCan = &g_RxMessage;
	u8 pRxcmdstate;
	
	//�������ݴ���
	pRxcmdstate = (pCan->id>>16)&0xFF;
	if(pRxcmdstate == ChargeStopFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStopFrameAck:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == TimingFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecTimingFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == VertionCheckFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecVertionCheckFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeParaInfoFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeParaInfoFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeServeOnOffFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeServeOnOffFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ElecLockFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecElecLockFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == PowerAdjustFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecPowerAdjustFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == PileParaInfoFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecPileParaInfoFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeStartStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeStartStateFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeStopStateFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeStopStateFrameAck:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == YcRecDataFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecYcRecDataFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == HeartRecFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecHeartFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == RecErrorStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecErrorStateFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}	
	else if(pRxcmdstate == FunPwmFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecFunPwmFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == UpdateBeginFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecUpdateBeginFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else
	{
		sprintf((char*)USARTx_TX_BUF,"CAN1_RecFrame:");
		sprintf((char*)Printf_buff,"%08X",pCan->id);
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	
	switch(pRxcmdstate)   //�����
	{
		case ChargeStopFrameAck:	//���ֹͣ֡�����ȼ�0x04��PF��0x03��
		{
			if(StrStateFrame.ChargeStopFrameReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double ChargeStopFrame!\n");
				
				break;
			}

            StrStateFrame.ChargeStopFrameReSendFlag = TRUE;			
			rt_lprintf("CAN_V110_Rec:Rec ChargeStopFrameAck\n");
			StrStateSystem.ChargIdent = pCan->data[0];       //���ӿڱ�ʶ
//			strCharStop.StopChargReson = pCan->data[1];      //ֹͣ���ԭ��0x01-����ֹͣ0x02-������ֹ0x03�Ʒѵ�Ԫ�жϳ�����������ֹͣ
			STR_ChargePile_A.ChgState = state_WaitStop;
			WaitStop_time = 0;
			rt_lprintf("Waring:State_WaitStop,WaitStop_time\n");
			
			break;
		}
		case TimingFrame:	//��Դ·����������������Ͷ�ʱ������ȼ�0x06��PF��0x05��
		{
			if(StrStateFrame.TimingFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double TimingFrame!\n");
				
				break;
			}				
			StrStateSystem.ChargIdent = pCan->data[0]; //���ӿڱ�ʶ
			temp = ((pCan->data[1])+(pCan->data[2]<<8))/1000;
			if((temp<=60)&&((pCan->data[3]&0x3F)<60)&&((pCan->data[4]&0x1F)<24)\
				&&((pCan->data[5]&0x1F)<=31)&&((pCan->data[5]&0x1F)>=1)\
				&&((pCan->data[6]&0x0F)<=12) &&((pCan->data[6]&0x0F)>=1)\
				&&((pCan->data[7]&0x7F)<100)&&((pCan->data[3]&0x3F)!=System_Time_STR.Minute))//����1����
			{								
				System_Time_STR.Second = temp;//��
				System_Time_STR.Minute = pCan->data[3]&0x3F;	// 0-5λ -- ����
				System_Time_STR.Hour = pCan->data[4]&0x1F;   // 0-4λ -- Сʱ

				System_Time_STR.Day = pCan->data[5]&0x1F;    // 0-4λ -- ����
				System_Time_STR.Week = pCan->data[5]&0xE0;   // 5-7λ -- ����
				System_Time_STR.Month = pCan->data[6]&0x0F;  // 0-3λ -- ��
				System_Time_STR.Year = pCan->data[7]&0x7F;   // 0-6λ -- �� ����λ0-99
				
//					In_RTC_Init(&System_Time_STR);      //��ʼ��Ƭ��RTC
//					SystemTimeToRTC(&System_Time_STR);  //��ʼ��Ƭ��RTC
				
				rt_lprintf("Date:%02X-%02X-%02X Week:%x Time:%02X:%02X:%02X\n",System_Time_STR.Year,\
																				System_Time_STR.Month,\
																				System_Time_STR.Day,\
																				System_Time_STR.Week,\
																				System_Time_STR.Hour,\
																				System_Time_STR.Minute,\
																				System_Time_STR.Second); 		
				rt_lprintf("CAN_V110_Rec:timing ok!\n");//									
			}
			else
			{
				rt_lprintf("CAN_V110_Rec:rec time data error||gap<60s noneed timeing!\n");//
				rt_lprintf("Date:%02X-%02X-%02X Week:%x",pCan->data[7]&0x7F,\
														pCan->data[6]&0x0F,\
														pCan->data[5]&0x1F,\
														pCan->data[5]&0xE0); 		
				rt_lprintf("Time:%02X:%02X:%02X\n",pCan->data[4]&0x1F,\
													 pCan->data[3]&0x3F,\
													 temp);
			}
			Inform_Communicate_Can(TimingFrameAck,TRUE);//��ʱ����Ӧ��		

			break;
		}
		case VertionCheckFrameAck:	//��Դ·����������������Ͱ汾У��������ȼ�0x06��PF:0x07��
		{
			if(StrStateFrame.VertionCheckFrameReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double VertionCheckFrame!\n");
				break;
			}				
			
			StrStateSystem.ChargIdent = pCan->data[0]; //���ӿڱ�ʶ
			temp =(pCan->data[2]<<8)+pCan->data[1]; 
			if((STR_ChargePile_A.ChgState == state_WaitVertionCheck)&&(Pro_Version == temp))
			{			
				STR_ChargePile_A.ChgState = state_WaitPilePara;	
				rt_lprintf("CAN_V110_Rec:ChgState = State_WaitPilePara!\n");
				StrStateFrame.VertionCheckFrameReSendFlag = TRUE;
			}
			else
			{
				rt_lprintf("cuowu:CAN_V110_Rec:ChgState=%u!\n",STR_ChargePile_A.ChgState);
			}
			
			break;
		}			
		case ChargeParaInfoFrameAck:	//��Դ·�����·����׮(��һ�����)������Ϣ��
		{
			if(StrStateFrame.ChargeParaInfoFrameReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double ChargeParaInfoFrameAck\n");
				break;
			}
			
			StrStateSystem.ChargIdent  = pCan->data[0]; //���ӿڱ�ʶ
			if(pCan->data[1] == 0x00)
			{
				strYX.WorkState = 0; // 0���� 1����й��� 2������ 3�����ͣ
				STR_ChargePile_A.ChgState = state_Standby;
				StrStateFrame.VertionCheckFrameAckReSendFlag = FALSE;
				StrStateFrame.VertionCheckFrameAckReSendCnt = 0;
				rt_lprintf("CAN_V110_Rec:State_Standby\n");
				
				StrStateFrame.YcSendDataFrameReSendFlag = TRUE;
				StrStateFrame.YcSendDataFrameReSendCnt = 0;				
				rt_lprintf("CAN_V110_Rec:Begin YcSendDataFrameReSendFlag\n");					
			}
			else
			{
				rt_lprintf("Waring:Receive ChargeParaInfoFrameAck\n");
				
			}

			break;
		}		
		case ChargeServeOnOffFrame:		 //��������ͣ����
		{
			if(StrStateFrame.ChargeServeOnOffFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double ChargeServeOnOffFrame!\n");
				break;
			}				
			
//				if(pCan->data[1] == SEVICEOK)
//				{
//					strChgSevCtrl.ChargSeveiceCmd = SEVICEOK;
//					strChgSevCtrl.CtrlState  = CTRLOK;
//					strChgSevCtrl.Ctrreaon= 0; 
//				}
//				else if((STR_ChargePile_A.TotalFau&CanOffLine_FAULT) == CanOffLine_FAULT)	//ͨ�ų�ʱ
//				{
//					strChgSevCtrl.ChargSeveiceCmd = pCan->data[1];
//					strChgSevCtrl.CtrlState       = CTRLERR;
//					strChgSevCtrl.Ctrreaon       = 2;		
//				}
//				else if((pCan->data[1]<1)||(pCan->data[1]>2))
//				{
//					strChgSevCtrl.ChargSeveiceCmd = pCan->data[1];
//					strChgSevCtrl.CtrlState       = CTRLERR;
//					strChgSevCtrl.Ctrreaon       = 1;
//				}
//				else
//				{
//					StrStateSystem.ChgSeviceState =      SEVICESTOP;
//					strChgSevCtrl.ChargSeveiceCmd = SEVICESTOP;
//					strChgSevCtrl.CtrlState       = CTRLOK;
//					strChgSevCtrl.Ctrreaon       = 0;
//				}

			Inform_Communicate_Can(ChargeServeOnOffFrameAck,TRUE);//��������ͣӦ��

			break;
		}
		case ElecLockFrame:	   //����������	  16 01 10  liangbing
		{	
			if(StrStateFrame.ElecLockFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double ElecLockFrame!\n");
				break;
			}				
			
			StrStateSystem.ChargIdent = pCan->data[0];  //���ӿڱ�ʶ
			strelelock.Num = pCan->data[1];  //��������
			strelelock.Elelock = pCan->data[2];	 //����ָ�� 1-������2-����

			if((pCan->data[1]<1)||(pCan->data[1]>2))
			{
				StrStateSystem.ReplyState = FAILED;
				StrStateSystem.reaonIdent = 0x01; //����
			}
			else
			{
//					if(strelelock.Elelock == 1)
//					{
//						for(i=0;i<3;i++)
//						{
//							POWER_LED_ON();	
//						}
//					}
//					else 
//					{
//						for(i=0;i<3;i++)
//						{
//							POWER_LED_OFF();	
//						}
//					}
			}	
			Inform_Communicate_Can(ElecLockFrameAck,TRUE);//����������Ӧ��
			
			break;
		}
		case PowerAdjustFrame:	   //���ʵ���
		{
			if(StrStateFrame.PowerAdjustFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double PowerAdjustFrame!\n");
				break;
			}					
			
			StrStateSystem.ChargIdent = pCan->data[0];    // ���ӿڱ�ʶ
//				strcapacity.C_capacity_style = pCan->data[1]; // ���ʵ�������
//				strcapacity.C_capacity_value = (pCan->data[3]<<8)|pCan->data[2];			
//				if(strcapacity.C_capacity_style == 1) // ���ʾ���ֵ
//				{
//					if(strcapacity.C_capacity_value >=10000)
//					{
//						temp = strcapacity.C_capacity_value-10000;
//						rt_lprintf("CAN_V110_Rec:SetPower=%u.%ukW!\n",temp/10,temp%10);
//						if(temp<=200)
//						{
//							temp = temp*100*5/22/9;   //20 000/220/3 *5/3
//							if((temp < PWMDUTYRATIOMAX)&&(temp > PWMDUTYRATIOMIN))
//							{
//								STR_ChargePile_A.PWM_Duty = temp;
//								StrStateSystem.ReplyState = SUCCESSFUL;
//								StrStateSystem.reaonIdent = 0;
//							}
//							else
//							{						 	
//								STR_ChargePile_A.PWM_Duty = PWMDUTYRATIOMIN;
//								StrStateSystem.ReplyState = FAILED;
//								StrStateSystem.reaonIdent = 1;
//							}
//							rt_lprintf("CAN_V110_Rec:PWM_Duty1=%d!\n",STR_ChargePile_A.PWM_Duty);	
//						}
//						else
//						{
//							StrStateSystem.ReplyState = FAILED;
//							StrStateSystem.reaonIdent = 1;
//						}
//					}
//					else
//					{
//						StrStateSystem.ReplyState = FAILED;
//						StrStateSystem.reaonIdent = 1;
//					}
//			    }
//			    else if(strcapacity.C_capacity_style == 2) // ռ�ձ�
//			    {
//				  if(strcapacity.C_capacity_value<=100)
//				  {
//					  temp =(long) POWERMAX*strcapacity.C_capacity_value*10/132;
//				 
//					  if(temp < PWMDUTYRATIOMAX)
//					  {
//						 STR_ChargePile_A.PWM_Duty = temp;
//						 StrStateSystem.ReplyState = SUCCESSFUL;
//						 StrStateSystem.reaonIdent = 0;
//					  }
//					  else
//					  { 
//						 STR_ChargePile_A.PWM_Duty =  PWMDUTYRATIOMAX;
//						 StrStateSystem.ReplyState = FAILED;
//						 StrStateSystem.reaonIdent = 1;

//					  }
//					  rt_lprintf("CAN_V110_Rec:PWM_Duty2=%d!\n",STR_ChargePile_A.PWM_Duty);						  
//				  }
//				  else
//				  {
//					  StrStateSystem.ReplyState = FAILED;
//					  StrStateSystem.reaonIdent = 1;
//				  }				  
//			  }
//			  else
//			  {
//				  StrStateSystem.ReplyState = FAILED;
//				  StrStateSystem.reaonIdent = 1;
//			  }
		  Inform_Communicate_Can(PowerAdjustFrameAck,FALSE); // ���ʵ���Ӧ��

		  break;
		}
		case ChargeStartStateFrame:
		{
			//������������Դ·��������"�������״̬"�����ȼ�0X04��PF��0X11
			if(StrStateFrame.ChargeStartStateFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double ChargeStartStateFrame\n");
				break;
			}
			StrStateSystem.ChargIdent = pCan->data[0];         // ���ӿڱ�ʶ
			StrStateSystem.LdSwitch = pCan->data[1];           // ���ɿ��ƿ��� 
			StrStateSystem.ConfIdent = pCan->data[2];          // ����������ȷ�ϱ�ʶ
			StrStateSystem.StartReson = pCan->data[3];     // ����ʧ��ԭ��
			if(StrStateSystem.ConfIdent == SUCCESSFUL)
			{
				Inform_Communicate_Can(ChargeStartStateFrameAck,FALSE); // ����״̬Ӧ��
				STR_ChargePile_A.ChgState = state_Charging;
				rt_event_send(&ChargePileEvent, ChargeStartOK_EVENT);
				rt_lprintf("monitor_task:state_Charging&ChargeStartOK_EVENT\n");
			}

			StrStateFrame.ChargeStartStateFrameAckReSendFlag = TRUE;
			rt_lprintf("CAN_V110_Rec:Receive ChargeStartStateFrame\n");
//				rt_timer_stop(DownCount_Time);	//�رն�ʱ��	
//				rt_lprintf("CAN_V110_Rec:rt_timer_stop DownCount_Time!\n");				

			break;
		}		
		case ChargeStopStateFrame:
		{
			//������������Դ·��������"���ֹͣ״̬"�����ȼ�0X04��PF��0X13
			if(StrStateFrame.ChargeStopStateFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double ChargeStopStateFrame\n");
				break;
			}
			StrStateSystem.ChargIdent = pCan->data[0];         // ���ӿڱ�ʶ
			StrStateSystem.StopReson = pCan->data[1];          // ֹͣԭ�� 
			StrStateSystem.ConfIdent = pCan->data[2];          // ����������ȷ�ϱ�ʶ
			if(StrStateSystem.ConfIdent == SUCCESSFUL)
			{
				Inform_Communicate_Can(ChargeStopStateFrameAck,FALSE); // ����״̬Ӧ��
				STR_ChargePile_A.ChgState = state_ChargEnd;
				rt_event_send(&ChargePileEvent, ChargeStopOK_EVENT);
				rt_lprintf("monitor_task:State_ChargEnd&ChargeStopOK_EVENT\n");
			}

			StrStateFrame.ChargeStopStateFrameAckReSendFlag = TRUE;
			rt_lprintf("CAN_V110_Rec:Receive ChargeStopStateFrameAckReSendFlag\n");
//				rt_timer_stop(DownCount_Time);	//�رն�ʱ��	
//				rt_lprintf("CAN_V110_Rec:rt_timer_stop DownCount_Time!\n");				

			break;
		}
		case YcRecDataFrame://��Դ·������������������Է���ң�����ݣ����ȼ�0X06��PF0X31��
		{
//				rt_lprintf("CAN_V110_Rec:Rec YcRecDataFrame!\n");
			chgYC.MesNum = pCan->data[0]; //��ǰ�������
			if(chgYC.MesNum == 1)
			{
//					chgYC.FraNum = pCan->data[1]; //��ǰ������֡��
//					chgYC.ValData = (pCan->data[3]<<8)+pCan->data[2];
				StrStateSystem.ChargIdent = pCan->data[4];//���ӿڱ�ʶ
				strYC.ChargVa = (pCan->data[6]<<8)+pCan->data[5];
				tempanalog = pCan->data[7];	//B���ѹ���ֽ�
				chgYC.Addition = 0x00000000;
				for(i=1;i<8;i++)
				{
					chgYC.Addition += pCan->data[i];
				}
			}
			else if(chgYC.MesNum == 2)
			{
				strYC.ChargVb = (pCan->data[1]<<8) + tempanalog;
				strYC.ChargVc = (pCan->data[3]<<8) + pCan->data[2];
				strYC.ChargIa = (pCan->data[5]<<8) + pCan->data[4];
				strYC.ChargIb = (pCan->data[7]<<7) + pCan->data[6];
				rt_lprintf("ChargIa=%u(.xx��)\n",strYC.ChargIa);
				rt_lprintf("ChargIb=%u(.xx��)\n",strYC.ChargIb);				
				for(i=1;i<8;i++)
				{
					chgYC.Addition += pCan->data[i];
				}
			}
			else if(chgYC.MesNum == 3)
			{
				strYC.ChargIc = (pCan->data[2]<<8) + pCan->data[1];
				rt_lprintf("ChargIc=%u(.xx��)\n",strYC.ChargIc);
				strYC.V_cp1 = (pCan->data[4]<<8) + pCan->data[3];
				STR_ChargePile_A.PWM_Duty = ((pCan->data[6]<<8) + pCan->data[5])/10;
				strYC.Tempt1 = pCan->data[7];			   
				for(i=1;i<8;i++)
				{
					chgYC.Addition += pCan->data[i];
				}
			}
			else if(chgYC.MesNum == 4)
			{
				strYC.Tempt2 = pCan->data[1];
				strYC.Tempt3 = pCan->data[2];
				strYC.Tempt4 = pCan->data[3];
				chgYC.TotalCheck = (pCan->data[5]<<8) + pCan->data[4];//�ۼ�У��
				for(i=1;i<4;i++)
				{
					chgYC.Addition += pCan->data[i];
				}				
				
			   if(chgYC.Addition == chgYC.TotalCheck)
			   {
					rt_lprintf("CAN_V110_Rec:Rec YC,Send YcSendDataFrame!\n");
//						Inform_Communicate_Can(YcSendDataFrame,FALSE);							
			   }
			   else
			   {
					rt_lprintf("CAN_V110_Rec:YcRecDataFrame TotalCheck error!\n");// ����У��ʧ��
			   }
			}
			break;
		}
		case YxRecDataFrame://����������Ʒѵ�Ԫ�����Է���ң�����ݣ����ȼ�0X06��PF��0X32��
		{
			StrStateSystem.ChargIdent = pCan->data[0]; // ���ӿڱ�ʶ

			strYX.WorkState = pCan->data[1]&0x03;
			strYX.TotalFau = (pCan->data[1]>>2)&0x01;
			strYX.TotalAlm = (pCan->data[1]>>3)&0x01;
			strYX.ConnState = (pCan->data[1]>>4)&0x01;
			strYX.StopEctFau = (pCan->data[1]>>5)&0x01;
			strYX.ArresterFau = (pCan->data[1]>>6)&0x01;
			strYX.GunOut = (pCan->data[1]>>7)&0x01;
			
			strYX.ChgTemptOVFau = pCan->data[2]&0x01;
			strYX.VOVWarn = (pCan->data[2]>>1)&0x01;
			strYX.VLVWarn = (pCan->data[2]>>2)&0x01;
			strYX.OutConState = (pCan->data[2]>>3)&0x01;
			strYX.PilotFau = (pCan->data[2]>>4)&0x01;
			strYX.ACConFau = (pCan->data[2]>>5)&0x01;
			strYX.OutCurrAla = (pCan->data[2]>>6)&0x01;
			strYX.OverCurrFau = (pCan->data[2]>>7)&0x01;
				   
			strYX.ACCirFau = pCan->data[3]&0x01;
			strYX.LockState = (pCan->data[3]>>1)&0x01;
			strYX.LockFauState = (pCan->data[3]>>2)&0x01;
			strYX.GunTemptOVFau = (pCan->data[3]>>3)&0x01;
			strYX.CC = (pCan->data[3]>>4)&0x01;
			strYX.CP = (pCan->data[3]>>5)&0x03;
			strYX.PEFau = (pCan->data[3]>>7)&0x01;
			
			strYX.DooropenFau = pCan->data[4]&0x01;
			strYX.ChgTemptOVAla = (pCan->data[4]>>1)&0x01;
			strYX.GunTemptOVAla = (pCan->data[4]>>2)&0x01;
			strYX.Contactoradjoin = (pCan->data[4]>>3)&0x01;

			strYX.GenAlaFau = pCan->data[5];
			strYX.OthWarnNum = pCan->data[6]; // �����������
			strYX.OthWarnVal = pCan->data[7]; // ��������ֵ

			break;			             
		}			
		case HeartRecFrame://��Դ·������������������Է����������ݣ����ȼ�0X06��PF��0X31��
		{
			StrStateSystem.ChargIdent = pCan->data[0]; // ���ӿڱ�ʶ
			StrStateSystem.RecCount = pCan->data[1];   // �������ķ��ͼ���
			StrStateSystem.RecStatus = pCan->data[2];  // �������Ľ���״̬
			if(StrStateSystem.RecStatus == FAILED)
			{
				rt_lprintf("Error:���յ��쳣����֡\n");
			}
			
			break;
		}
		case RecErrorStateFrame://��Դ·������������֡�����ȼ�0X04��PF��0X51��
		{
			StrStateSystem.ChargIdent = pCan->data[0];           // ���ӿڱ�ʶ
//				StrErrData.VerChTimeOut = pCan->data[1]&0x01;        // �汾У�鳬ʱ 
//				StrErrData.ParSetTimeOut = pCan->data[1]&0x02;       // �������ó�ʱ 	
//				StrErrData.StartCharConTimeOut = pCan->data[1]&0x04; // �����������ȷ�ϳ�ʱ
//				StrErrData.WStartCharTimeOut = pCan->data[1]&0x08;   // �ȴ�����������״̬��ʱ
//				StrErrData.StopCharConTimeOut = pCan->data[1]&0x10;  // ֹͣ�������ȷ�ϳ�ʱ  
//				StrErrData.WStopCharTimeOut = pCan->data[1]&0x20;    // �ȴ�ֹͣ������״̬��ʱ 
			rt_lprintf("Error:Receive RecErrorStateFrame\n");
			
			break;
		}
		case PileParaInfoFrame:				//���׮������Ϣ��ѯ
		{
			if(StrStateFrame.PileParaInfoFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double PileParaInfoFrame!\n");
				break;
			}
			Inform_Communicate_Can(PileParaInfoFrameAck,FALSE);
			
			break;
		}
		case FunPwmFrame:				   //���׮������Ϣ��ѯ
		{
//				StrStateSystem.ChargIdent = pCan->data[0];    // ���ӿڱ�ʶ
//				STR_Charge_B.PWM_Duty = pCan->data[1]*10;     // ����BǹPWMռ�ձ�	
//				PWM_SET_B(STR_Charge_B.PWM_Duty); 			

//				rt_lprintf("CAN_V110_Rec:STR_Charge_B.PWM_Duty=%u!\n",STR_Charge_B.PWM_Duty);
			
			break;
		}
		case UpdateBeginFrame:			   //��������
		{
			if(StrStateFrame.UpdateBeginFrameAckReSendFlag == TRUE)  // �յ��ظ�����
			{
				rt_lprintf("Waring:Receive Double UpdateBeginFrame!\n");
				
				break;
			}				
			if((STR_ChargePile_A.ChgState == state_Standby)&&(STR_ProgramUpdate.App_Mark == 0x00)) //1 ��������  0 δ����
			{
				StrStateSystem.ChargIdent = pCan->data[0];
				StrStateSystem.DeviceType = pCan->data[1];
				
//					if(g_flash_txt == NULL)  
//					{
//						g_flash_txt=(FIL *)mymalloc(SRAMCCM,sizeof(FIL));//����FIL�ֽڵ��ڴ�����
//						rt_lprintf("Can_Update:g_flash_txt=%u\n",g_flash_txt);
//					}	
//					//Ŀ���ļ��д��ڸ��ļ�,��ɾ��
//					rt_lprintf("%s file delete,res=%u!\n",FLASH_FILE_PROGRAM_BIN_FILE,f_unlink((const TCHAR *)FLASH_FILE_PROGRAM_BIN_FILE));
//					
//					STR_ProgramUpdate.App_Mark = 0x01;          //1����������  0 δ����  2׼����
//					STR_ProgramUpdate.App_Channel = 0x03;       //0�����ڣ�1��2G��2��4G��3��can
//					STR_ProgramUpdate.Update_Package_No = 0x01;	//�����ļ��ΰ�������  ��1��ʼ����
//					STR_ProgramUpdate.Update_Message_No = 0x01; //����ÿ�����к�      ��1��ʼ����
//					STMFLASH_LENTH = 0;
//					chgUpdate.ErrResendCount = 0;
//					STR_ProgramUpdate.AdditionAll = 0;
//					STR_ProgramUpdate.Update_Confirm = 1;       //Ĭ������ʧ��
//	                chgUpdate.UpdateConfIdent = 0x00;           //ȷ�ϱ�ʶ 0x00���������� 0x01����ֹ���� ��������Ч
//	                chgUpdate.NoDownloadReason = 0x00;          //��ֹ����ԭ��	0x00���� 0x01������֧�ִ˹��� 0x02�����ݺϷ���У��ʧ��
//					ProgramUpdatePara(WRITE);
//					STR_ChargePile_A.ChgState = State_Update;
//					rt_lprintf("CAN_V110_Rec:ChgState:State_Update\n");

				rt_timer_stop(CAN_Heart_Tx);// �رն�ʱ��
				rt_timer_stop(CAN_250ms_Tx);// �رն�ʱ��
			}
			else
			{
//					StrStateSystem.ChargIdent = pCan->data[0];
//					StrStateSystem.DeviceType = pCan->data[1];
//	                chgUpdate.UpdateConfIdent = 0x01;//ȷ�ϱ�ʶ 0x00���������� 0x01����ֹ���� ��������Ч
//	                chgUpdate.NoDownloadReason = 0x01;//��ֹ����ԭ��	0x00���� 0x01������֧�ִ˹��� 0x02�����ݺϷ���У��ʧ��					
//					Inform_Communicate_Can(UpdateBeginFrameAck,FALSE);										
				rt_lprintf("CAN_V110_Rec:ChgState=%u,App_Mark=%u\n",STR_ChargePile_A.ChgState,STR_ProgramUpdate.App_Mark);
			}
			rt_lprintf("CAN_V110_Rec:���յ���������!!!!!!!!!!!!\n");
			
			break;
		}
		case DataSendReqFrame://�յ�����������֡����
		{
			if(STR_ChargePile_A.ChgState == state_Update) //1 ��������  0 δ����
			{
				StrStateSystem.ChargIdent = pCan->data[0];
				StrStateSystem.DeviceType = pCan->data[1];
				STR_ProgramUpdate.file_totalNo = (pCan->data[3]<<8) + pCan->data[2];
				STR_ProgramUpdate.file_ByteNo = (pCan->data[7]<<24) + (pCan->data[6]<<16) + (pCan->data[5]<<8) + pCan->data[4];									
				rt_lprintf("CAN_V110_Rec:file_totalNo=%u,file_ByteNo=%u\n",STR_ProgramUpdate.file_totalNo,STR_ProgramUpdate.file_ByteNo);
//					OSSemPost(MY_CAN_A);//�����ź��� �յ�����������֡
			}
			else
			{										
				rt_lprintf("CAN_V110_Rec:ChgState=%u\n",STR_ChargePile_A.ChgState);
			}
			
			break;
		}			
		case DataSendFrame:	//��������������
		{
			chgUpdate.MessageNum = pCan->data[0]; //��ǰ�������
			if(chgUpdate.MessageNum == 1)
			{
				chgUpdate.FrameNum = pCan->data[1]; //��ǰ������֡��
				chgUpdate.ValDataLen = (pCan->data[3]<<8) + pCan->data[2];//������Ч���ݳ���
				StrStateSystem.ChargIdent = pCan->data[4];//���ӿڱ�ʶ
				StrStateSystem.DeviceType = pCan->data[5];//�豸����
				rt_lprintf("CAN_V110_Rec:FrameNum=%u,ValDataLen=%u\n",chgUpdate.FrameNum,chgUpdate.ValDataLen);
				memcpy(STMFLASH_BUFF+STMFLASH_LENTH,&(pCan->data[6]),2);	        //����ǰ���ݰ����뻺����
				STMFLASH_LENTH += 2;															
				STR_ProgramUpdate.Update_Message_No++;					
				rt_lprintf("CAN_V110_Rec:STMFLASH_LENTH=%u,Update_Message_No=%u\n",STMFLASH_LENTH,STR_ProgramUpdate.Update_Message_No);
				chgUpdate.AdditionUp = 0x0000;
				for(i=1;i<8;i++)
				{
					chgUpdate.AdditionUp += pCan->data[i];
				}
				for(i=6;i<8;i++)
				{
					STR_ProgramUpdate.AdditionAll += pCan->data[i];
				}
			}				
			else if(chgUpdate.MessageNum == STR_ProgramUpdate.Update_Message_No)
			{
				//�ж����һ֡����
				if(chgUpdate.FrameNum == STR_ProgramUpdate.Update_Message_No)
				{
					rt_lprintf("CAN_V110_Rec:ValDataLen=%u,STMFLASH_LENTH=%u\n",chgUpdate.ValDataLen,STMFLASH_LENTH);
					memcpy(STMFLASH_BUFF+STMFLASH_LENTH,&(pCan->data[1]),chgUpdate.ValDataLen - STMFLASH_LENTH);//����ǰ���ݰ����뻺����
					STMFLASH_LENTH = chgUpdate.ValDataLen - 2;
					for(i=2;i<STMFLASH_LENTH;i++)//ǰ���������Ѿ��ڵ�һ֡��У��
					{
						chgUpdate.AdditionUp += STMFLASH_BUFF[i];
						STR_ProgramUpdate.AdditionAll += STMFLASH_BUFF[i];
					}
					
					chgUpdate.RevCheckUp = (STMFLASH_BUFF[STMFLASH_LENTH+1]<<8) + STMFLASH_BUFF[STMFLASH_LENTH];//�ۼ�У��
					if(chgUpdate.AdditionUp == chgUpdate.RevCheckUp)
					{
//							Bin_Write_File((char *)FLASH_FILE_PROGRAM_BIN_FILE,STMFLASH_BUFF,STMFLASH_LENTH);
//							rt_lprintf("CAN_V110_Rec:���յ�%u��-���%u֡��ȷ\n",STR_ProgramUpdate.Update_Package_No,STR_ProgramUpdate.Update_Message_No);
//							//�жϲ������һ������
//							if(STR_ProgramUpdate.file_totalNo == STR_ProgramUpdate.Update_Package_No)
//							{
//								STR_ProgramUpdate.Update_Package_No = 0x00;
//							}
//							else
//							{
//								STR_ProgramUpdate.Update_Package_No++;
//								chgUpdate.ErrResendCount = 0;
//							}
//							STMFLASH_LENTH = 0;
//							chgUpdate.UpdateConfIdent = 0x00;           //ȷ�ϱ�ʶ 0x00���ɹ� 0x01��ʧ�� ��������Ч
//							chgUpdate.NoDownloadReason = 0x00;          //��ֹ����ԭ��	0x00���� 0x01��У�鲻�ɹ� 0x02���߳�ʧ�� 0x03�����ղ�����					
//							Inform_Communicate_Can(DataSendFrameAck,FALSE);
//							STR_ProgramUpdate.Update_Message_No = 0x01;	
					}
					else
					{
						chgUpdate.ErrResendCount++;
						rt_lprintf("CAN_V110_Rec:CRC����-AdditionUp=%u,RevCheckUp=%u,ErrResendCount=%u\n",chgUpdate.AdditionUp,chgUpdate.RevCheckUp,chgUpdate.ErrResendCount);

						if(chgUpdate.ErrResendCount >= 3)
						{
							STMFLASH_LENTH = 0;
							STR_ProgramUpdate.Update_Message_No = 0x00;								
							chgUpdate.UpdateConfIdent = 0x01;           //ȷ�ϱ�ʶ 0x00���ɹ� 0x01��ʧ�� ��������Ч
							chgUpdate.NoDownloadReason = 0x01;          //��ֹ����ԭ��	0x00���� 0x01��У�鲻�ɹ� 0x02���߳�ʧ�� 0x03�����ղ�����
							STR_ProgramUpdate.Update_Package_No = 0x01;								
							Inform_Communicate_Can(DataSendFrameAck,FALSE);

							STR_ProgramUpdate.App_Mark = 0x00;        //1 ��������  0 δ����
							Inform_Communicate_Can(SendErrorStateFrame,FALSE);
							chgUpdate.ErrResendCount = 0;
							STR_ChargePile_A.ChgState = state_Standby;
							rt_lprintf("CAN_V110_Rec:����ʧ��,�ص�����\n");
							rt_timer_start(CAN_Heart_Tx);	// ������ʱ��
							rt_timer_start(CAN_250ms_Tx);	// ������ʱ��
						}
						else
						{
							STMFLASH_LENTH = 0;					
							chgUpdate.UpdateConfIdent = 0x01;           //ȷ�ϱ�ʶ 0x00���ɹ� 0x01��ʧ�� ��������Ч
							chgUpdate.NoDownloadReason = 0x01;          //��ֹ����ԭ��	0x00���� 0x01��У�鲻�ɹ� 0x02���߳�ʧ�� 0x03�����ղ�����							
							Inform_Communicate_Can(DataSendFrameAck,FALSE);
							STR_ProgramUpdate.Update_Message_No = 0x01;								
						}
					}	
				}					
				else
				{
					memcpy(STMFLASH_BUFF+STMFLASH_LENTH,&(pCan->data[1]),7);//����ǰ���ݰ����뻺����
					STMFLASH_LENTH += 7;
					STR_ProgramUpdate.Update_Message_No++;
				}	
			}
			else
			{
				rt_lprintf("CAN_V110_Rec:����-MessageNum=%u\n",chgUpdate.MessageNum);
			}
//				OSSemPost(MY_CAN_A);//�����ź��� �����ϴ��ɹ�
			rt_lprintf("CAN_V110_Rec:���յ�����֡���=%u\n",chgUpdate.MessageNum);
			break; 
		}
		case UpdateCheckFrame:	//���ճ���У������֡
		{
			StrStateSystem.ChargIdent = pCan->data[0];
			StrStateSystem.DeviceType = pCan->data[1];
			if(STR_ProgramUpdate.AdditionAll == ((pCan->data[5]<<24) +(pCan->data[4]<<16) +(pCan->data[3]<<8) + pCan->data[2]))
			{
				STR_ProgramUpdate.Update_Confirm = 0;  //�����ɹ�
				chgUpdate.UpdateConfIdent = 0x00;      //ȷ�ϱ�ʶ 0x00���ɹ� 0x01��ʧ�� ��������Ч
				chgUpdate.NoDownloadReason = 0x00;     //��ֹ����ԭ��	0x00���� 0x01��У�鲻�ɹ� 0x02���߳�ʧ�� 0x03�����ղ�����						
			}
			else
			{
				STR_ProgramUpdate.Update_Confirm = 0x01;    //������־  0�ɹ�   1ʧ��
				chgUpdate.UpdateConfIdent = 0x01;           //ȷ�ϱ�ʶ 0x00���ɹ� 0x01��ʧ�� ��������Ч
				chgUpdate.NoDownloadReason = 0x01;          //��ֹ����ԭ��	0x00���� 0x01��У�鲻�ɹ� 0x02���߳�ʧ�� 0x03�����ղ�����
				rt_lprintf("CAN_V110_Rec:Error AdditionAll=0x%08X\n",STR_ProgramUpdate.AdditionAll);
			}
			Inform_Communicate_Can(UpdateCheckFrameAck,FALSE);
//				OSSemPost(MY_CAN_A);//�����ź��� �����ϴ��ɹ�	
			
			break;
		}
		case ResetFrame:	//���ճ�������֡
		{
			StrStateSystem.ChargIdent = pCan->data[0];
			StrStateSystem.DeviceType = pCan->data[1];
			StrStateSystem.ResetAct = pCan->data[2];				
			if(StrStateSystem.ResetAct ==0xAA)
			{
				Inform_Communicate_Can(ResetFrameAck,FALSE);
//					if(g_flash_txt != NULL)  
//					{
//						myfree(SRAMCCM,g_flash_txt);
//						rt_lprintf("CAN_V110_Rec:g_flash_txt=%u\n",g_flash_txt);
//						g_flash_txt = NULL;
//						if(ProgramUpdatePara(WRITE))
//						{
//							//Ŀ���ļ��д��ڸ��ļ�,��ɾ��
//							rt_lprintf("%s file delete,res=%u!\n",FLASH_FILE_PROGRAM_BIN_FILE,f_unlink((const TCHAR *)FLASH_FILE_PROGRAM_BIN_FILE));//																	
//						}
//						else
//						{
//							rt_lprintf("CAN_V110_Rec:ProgramUpdatePara WRITE_OK\n");									
//						}						
//					}
//					if(usbx.hdevclass==1)//���ȴ洢��U��
//					{
//						NAND_LogWrite(LogBuffer,&LogBufferLen);
//						usbapp_mode_stop();//��ֹͣ��ǰUSB����ģʽ																
//					}
				
				__set_FAULTMASK(1); //�ر������ж�
				NVIC_SystemReset(); //��λ
			}
			else
			{
				rt_lprintf("CAN_V110_Rec:Error ResetAct=%u!\n",StrStateSystem.ResetAct);
			}
			
			break;
		}			
		default:
		{
			break;
		}
	}
	can_heart_count	= 0; //�յ����ݣ���������������
	STR_ChargePile_A.TotalFau &= ~CanOffLine_FAULT;
}
/***************************************************************
* ��������: CAN1_Send_Msg(struct rt_can_msg CanSendMsg,u8 len)
* ��    ��:
* �� �� ֵ:
* ��    ��: ����������Ʒѵ�Ԫ��������
***************************************************************/
u8 CAN1_Send_Msg(struct rt_can_msg CanSendMsg,u8 length)
{
//	uint32_t can1tx_mailbox = HAL_OK;
	u8 SendCmd = 0;
	struct rt_can_msg Tx_Message;	
	u32 i;
	memset(Tx_Message.data,0x00,sizeof(Tx_Message.data)); // ��ʼ�����ͻ���
	Tx_Message.id = CanSendMsg.id;                        // ��չ��ʾID��29 λ��
	Tx_Message.ide = RT_CAN_EXTID;                        // ��չ֡
	Tx_Message.rtr = RT_CAN_DTR;                          // ����֡
	Tx_Message.len = 0x08;
	
	for(i=0;i<length;i++)
	{
	    Tx_Message.data[i]=CanSendMsg.data[i];
	}

	SendCmd = (Tx_Message.id>>16)&0xFF;
	if(SendCmd == ChargeStartFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStartFrameAck:");
	    sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else if(SendCmd == ChargeStopFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStopFrameAck:"); 
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);	
	}
	else if(SendCmd == YcSendDataFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"YcSendDataFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else if(SendCmd == TimingFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"TimingFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == VertionCheckFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"VertionCheckFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else if(SendCmd == ChargeParaInfoFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeParaInfoFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ChargeServeOnOffFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeServeOnOffFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ElecLockFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ElecLockFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == PowerAdjustFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"PowerAdjustFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == PileParaInfoFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"PileParaInfoFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ChargeStartStateFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStartStateFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ChargeStopStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStopStateFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == HeartSendFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"HeartSendFrame:");
	    sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == SendErrorStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"ErrorStateFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else
	{
		sprintf((char*)USARTx_TX_BUF,"CAN1_Send_Msg:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //ѭ����������
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);		
	}

	rt_device_write(chargepile_can, 0, &Tx_Message, sizeof(struct rt_can_msg));

	return 0;		   	
}

#endif /*RT_USING_CAN*/
