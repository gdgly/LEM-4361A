
//#include <rtthread.h>
#include <rtdevice.h>
#include <strategy.h>

#define HPLC_UART_NAME       "uart5"  /* �����豸���� */

#define BLUE_TOOTH_UART_NAME       "uart3"  /* �����豸���� */
extern struct rt_thread hplc;
extern struct _698_STATE hplc_698_state;
extern rt_uint32_t hplc_event;
extern int hplc_lock1,hplc_lock2;
extern rt_uint32_t strategy_event;



/****************�궨��**********************************/
//�����¼�����
typedef enum {
	CTRL_NO_EVENT             =0x00000000,
	ChgPlanIssue_EVENT       	=0x00000001,        	//���ƻ��·��¼�
	ChgPlanIssueGet_EVENT     =0x00000002,      		//���ƻ��ٲ��¼�	
	ChgPlanAdjust_EVENT    		=0x00000004,        	//���ƻ������¼�
	StartChg_EVENT						=0x00000008,          //��������¼�
	StopChg_EVENT							=0x00000010,          //ֹͣ����¼�		          			
} CTRL_EVENT_TYPE;//��������·�����������¼�



typedef struct
{
	int array_size;
	PLAN_FAIL_EVENT *plan_fail_event;

}_698_PLAN_FAIL_EVENT;



typedef struct
{
	int array_size;
	CHARGE_STRATEGY *charge_strategy;

}_698_CHARGE_STRATEGY;
/**********/

typedef enum {
	Cmd_ChgPlanIssue=0, 					//���ƻ��·�
	Cmd_ChgPlanAdjust,                 		//���ƻ�����
	Cmd_ChgPlanIssueAck,                 	//���ƻ��·�Ӧ��
	Cmd_ChgPlanAdjustAck,                 //���ƻ�����Ӧ��
	Cmd_StartChg,								//�����������·�
	Cmd_StartChgAck,						//�������Ӧ��
	Cmd_StopChg,								//ֹͣ�������·�
	Cmd_StopChgAck,							//ֹͣ���Ӧ��
	Cmd_ChgRecord,													//���ͳ�綩��
	Cmd_ChgPlanExeState,                    //���ͳ��ƻ�ִ��״̬
	Cmd_DeviceFault,                      	//����·�����쳣״̬
	Cmd_PileFault,                 					//���ͳ��׮�쳣״̬
	Cmd_ChgPlanIssueGetAck,
}COMM_CMD_C;//·�������������������
#define COMM_CMD_C rt_uint32_t

//�¼�

#define  link_request	1
#define  link_request_load	0
#define  link_request_heart_beat	1
#define  link_request_unload	1


#define  link_response 0x81  //Ҫ����responseҲӦ����list��
#define  connect_request 2
#define  release_request 3

#define  get_request 5
#define  set_request 6	
#define  action_request 7
#define  report_response 8
#define  proxy_request 9
#define  connect_response 0x82
#define  release_response 0x83
#define  release_notification 0x84  //�����ô����
#define  get_response 0x85
#define  set_response 0x86
#define  action_response 0x87
#define  report_notification 0x88
#define  proxy_response 0x89
#define  security_request 16
#define  security_response 144




//GET-Request��=CHOICE
//��ȡһ�������������� [1] ;��ȡ���ɸ������������� [2] ;��ȡһ����¼�Ͷ����������� [3] 
//;��ȡ���ɸ���¼�Ͷ����������� [4] ��ȡ��֡��Ӧ����һ�����ݿ����� [5];��ȡһ���������Ե� MD5 ֵ [6]
#define  GetRequestNormal	1
#define  GetRequestNormalList	2
#define  GetRequestRecord	3
#define  GetRequestRecordList 4
#define  GetRequestNext 5
#define  GetRequestMD5 6

//�����������������
#define  ActionRequest 1//����һ�����󷽷����� [1] ��
#define  ActionRequestList 2//�������ɸ����󷽷����� [2] ��
#define  ActionThenGetRequestNormalList 3//�������ɸ����󷽷����ȡ���ɸ������������� [3] 

//�ϱ�����
#define  ReportNotificationList 1
#define  ReportNotificationRecordList 2



#define OI1_MASK 0XF0
#define OI2_MASK 0X0F

enum Data_T{
Data_NULL =0,
Data_array=1,
Data_structure=2,
Data_bool=3,
Data_bit_string=4,           //һ��bitռһ��byte
Data_double_long=5,         //32 ����λ������ Integer32��
Data_double_long_unsigned=6,//32 ����λ�������� double-long-unsigned��
Data_octet_string=9,
Data_visible_string=10,
Data_UTF8_string=12,
Data_integer=15,						//8 ����λ������ integer��
Data_long=16,
Data_unsigned=17,
Data_long_unsigned=18,			//�� Unsigned16��
Data_long64=20,
Data_long64_unsigned=21,
Data_enum=22,
Data_float32=23,
Data_float64=24,
Data_date_time=25,
Data_date=26,
Data_time=27,
Data_date_time_s=28,
Data_OI=80,
Data_OAD=81,
Data_ROAD=82,
Data_OMD=83,
Data_TI=84,
Data_TSA=85,
Data_MAC=86,
Data_RN=87,
Data_Region=88,
Data_Scaler_Unit=89,
Data_RSD=90,
Data_CSD=91,
Data_MS=92,
Data_SID=93,
Data_SID_MAC=94,
Data_COMDCB=95,
Data_RCSD=96
};
enum Object_I{
oi_electrical_energy=0,//�����������
oi_max_demand,
oi_variable,//���������
oi_event,
oi_parameter,//�α��������
oi_freeze,
oi_Acquisition_monitor,//�ɼ����
oi_gather,//����
oi_contral,//���������ı�ʶ	
oi_file_transfer=0xf//�ļ�����
//oi_esam,//ESAM�ӿں��涼��f
//oi_io_dev,//��������豸
//oi_display,//��ʾ	
//oi_define_by_user//
};
/****************************************/

//static struct CharPointDataManage _698_data_rev,_698_data_send;//��ò���
//static struct _698_ADDR *_698_addr;//ûɶ��

static unsigned char test_hplc[200]={
0xFE , 0xFE , 0xFE , 0xFE , 0x68 , 0x34 , 
0x00 , 0xC3 , 0x05 , 0x02 , 0x00 , 0x04 , 
0x12 , 0x18 , 0x00 , 0x00 , 0xB7 , 0x02 , 
0x85 , 0x01 , 0x1D , 0x00 , 0x10 , 0x02 , 
0x00 , 0x01 , 0x01 , 0x05 , 0x06 , 0x00 , 
0x00 , 0x00 , 0x66 , 0x06 , 0x00 , 0x00 , 
0x00 , 0x24 , 0x06 , 0x00 , 0x00 , 0x00 , 
0x40 , 0x06 , 0x00 , 0x00 , 0x00 , 0x00 , 
0x06 , 0x00 , 0x00 , 0x00 , 0x01 , 0x00 , 
0x00 , 0xF7 , 0x08 , 0x16,	
0xfe, 0xfe, 0xfe, 0xfe, 0xfe,	
0xFE, 0xFE, 0xFE, 0xFE, 0x68, 
0x17 , 0x00 , 0x43 , 0x45 , 0xAA , 
0xAA , 0xAA , 0xAA , 0xAA , 0xAA ,
0x00 , 0x5B , 0x4F , 0x05 , 0x01 , 
0x00 , 0x40 , 0x01 , 0x02 , 0x00 , 
0x00 , 0xED , 0x03 , 0x16	,
0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
0x68, 0x1e, 0x00, 0x81, 0x05,
0x07, 0x09, 0x19, 0x05, 0x16,
0x20, 0x00, 0x60, 0x30, 0x01, 
0x00, 0x00, 0x00, 0xb4, 0x07,
0xe0, 0x05, 0x13, 0x04, 0x08,	
0x05, 0x00, 0x00, 0xa4, 0xfc,
0x83, 0x16
};
/*, 0xfc, 0x49, 0x01, 
0x00, 0x00, 0x00, 0xb4, 0x07,
0xe0, 0x05, 0x13, 0x04, 0x08,	
0x05, 0x00, 0x00, 0xa4, 0xa8,
0xd3, 0x16	*/
/***********ֻ����698Э����صĽṹ��ͺ���*****************************/
/***********
��ʼ������ǻ�����������,�������ȵĲ���ָ�룬����ȷ���������飨ָ�����ȵ�ָ�룩
Ϊ������д�����õ�Сд����ʵ�ÿ�ͷ�Ǵ�дҲ����̫�鷳��
************/
struct _698_Scaler_Unit{
	int index;//���㡪���������ӵ�ָ��������Ϊ 10������ֵ�������ֵģ�����Ӧ���� 0��
  int type;//ö�����Ͷ�������λ
};
struct _698_comdcb{
	unsigned char baud;//300bps�� 0���� 600bps�� 1���� 1200bps�� 2����2400bps�� 3���� 4800bps�� 4���� 7200bps�� 5����
										 //9600bps�� 6���� 19200bps�� 7���� 38400bps�� 8����57600bps�� 9���� 115200bps�� 10���� ����Ӧ�� 255��	
	unsigned char parity;//��У�飨 0������У�飨 1����żУ�飨 2��
	unsigned char d_bits;//5�� 5���� 6�� 6���� 7�� 7���� 8�� 8��
	unsigned char s_bit;//1�� 1���� 2�� 2�� 
	unsigned char f_control;//	��(0)�� Ӳ��(1)�� ���(2)
};
struct _698_time{
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char data[3];//�����ǲ��Ǹ���	
};
struct _698_date{
	unsigned char year[2];//year�� milliseconds=FFFFH ��ʾ��Ч��
	unsigned char month;//FFH ��ʾ��Ч����ͬ
	unsigned char day;
	unsigned char data[4];//�����ǲ��Ǹ���	
};
struct _698_date_time_s{
	unsigned char year[2];//year�� milliseconds=FFFFH ��ʾ��Ч��
	unsigned char month;//FFH ��ʾ��Ч����ͬ
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char data[7];//�����ǲ��Ǹ���	
};
struct _698_date_time{
	unsigned char year[2];//year�� milliseconds=FFFFH ��ʾ��Ч��
	unsigned char month;//FFH ��ʾ��Ч����ͬ
	unsigned char day;
	unsigned char week;//day_of_week�� 0 ��ʾ���գ� 1~6 �ֱ��ʾ��һ��������
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char millisconds[2];//
	unsigned char data[10];//�����ǲ��Ǹ���
};
/*
���������
attribute_method_id
*/
struct _698_oad{//��������������OAD�� Object Attribute Descriptor��
	unsigned char oi[2];//�����ʶ�� OI�������ֽ���ɣ����÷������ķ�ʽΪϵͳ�ĸ��������ṩ��ʶ��.
											//�ο�P112,���ǵĶ��󣺵�����������ʶ����������ࣻ������	���¼��ࣨ����ƺ�Ҳ����Ҫ�ϱ������ݣ����α�����;
											//������;�ɼ������;������;������;�ļ�������;ESAM �ӿ���;��������豸�ӿ���;��ʾ��;�����Զ������
	unsigned char  attribute_id;//���Ա�ʶ��������,bit0��bit4 �����ʾ�������Ա�ţ�ȡֵ 0��31������ 0 ��ʾ�����������ԣ���������������ԣ�
															//bit5��bit7 �����ʾ�������������������Ƕ���ͬһ�������ڲ�ͬ���ջ�����ȡֵģʽ��ȡֵ 0��7�����������ھ���
															//��������������
	unsigned char  attribute_index;//������Ԫ������,00H ��ʾ��������ȫ�����ݡ����������һ���ṹ�����飬 01H ָ��������Եĵ�һ��Ԫ�أ���
																 //��������һ����¼�͵Ĵ洢��������ֵ n ��ʾ����� n �εļ�¼��
};
struct _698_road{ 				//��¼�Ͷ�������������ROAD,����������¼�Ͷ����е�һ�������ɸ������������ԡ�
	struct _698_oad priv_oad;
	struct _698_oad * oad_list;//��ʱ����ʱ���ٿռ�
};
struct _698_ms{//��ѡ�������
	unsigned char no_m;//�൱����Ч���á�
	unsigned char all_m;//ȫ���ɲɼ��ĵ��ܱ�
	unsigned char * usr_type;
	struct _698_tsa * usr_tsa;
	unsigned short * config_no;
	struct _698_region * usr_type_region;
	struct _698_region * usr_tsa_region;
	struct _698_region * config_no_region;	
};
struct _698_ti{
	unsigned char type;//�� �� 0������ �� 1����ʱ �� 2������ �� 3������ �� 4������ �� 5��
	unsigned short value;		
};

struct _698_selector1{//Selector1 Ϊָ������ָ��ֵ��
	struct _698_oad priv_oad;
	unsigned char * data;	
};
struct _698_selector2{//Selector2 Ϊָ�������������������ֵ��
	struct _698_oad priv_oad;
	unsigned char * start_data;	
	unsigned char * end_data;	
	unsigned char * interval_data;	
};
struct _698_selector3{//Selector3 Ϊ���ɸѡ�������ɸ�ָ����������ֵ��
	struct _698_selector2	* selector2_list;	
};
struct _698_selector4{//Selector4 Ϊָ�����ܱ��ϡ�ָ���ɼ�����ʱ�䡣
	struct _698_date_time date_time;
	struct _698_ms selector4_ms;	
};
struct _698_selector5{//Selector5 Ϊָ�����ܱ��ϡ�ָ���ɼ��洢ʱ�䡣
	struct _698_date_time date_time;
	struct _698_ms selector5_ms;		
};
struct _698_selector6{//Selector6 Ϊָ�����ܱ��ϡ�ָ���ɼ�����ʱ���������������ֵ��
	struct _698_date_time s_date_time;
	struct _698_date_time e_date_time;	
	struct _698_ti ti;	
	struct _698_ms selector6_ms;		
};
struct _698_selector7{//Selector7 Ϊָ�����ܱ��ϡ�ָ���ɼ��洢ʱ���������������ֵ��	
	struct _698_date_time s_date_time;
	struct _698_date_time e_date_time;	
	struct _698_ti ti;	
	struct _698_ms selector7_ms;			
};
struct _698_selector8{//Selector8 Ϊָ�����ܱ��ϡ�ָ���ɼ���ʱ���������������ֵ��	
	struct _698_date_time s_date_time;
	struct _698_date_time e_date_time;	
	struct _698_ti ti;	
	struct _698_ms selector8_ms;				
};
struct _698_selector9{//Selector9 Ϊָ��ѡȡ�ϵ� n �μ�¼
	unsigned char time;	
};
struct _698_selector10{//Select10 Ϊָ��ѡȡ���µ� n ����¼��
	unsigned char time;
	struct _698_ms selector10_ms;				
};
/*
RSD ����ѡ���¼�Ͷ������Եĸ�����¼������ά��¼�����ѡ����ͨ���Թ��ɼ�¼��ĳЩ����������ֵ����ָ��������ѡ��
��Χѡ�����䣺ǰ�պ󿪣���[��ʼֵ������ֵ����
���磺�¼��������¼���¼�����ԡ����������Ķ������ݼ�¼�����ԡ��ɼ������Ĳɼ����ݼ�¼��
Ӧ����ʾ��1) �����¼���¼��ͨ��ʹ���¼�����ʱ�����ѡ��2) ���ڶ������ݼ�¼��ͨ��ʹ�ö���ʱ�����ѡ��
*/
struct _698_rsd{//=CHOICE
	unsigned char no_choice; //P36 ����ĳ�Ա�ṹ��Ҳ�����ڸ�ҳ�� ��ѡ�� [0] NULL���ǲ��Ǹ���ͷ��д���ֵ�ģ�
	struct _698_selector1 selector1;
	struct _698_selector2 selector2;
	struct _698_selector3 selector3;
	struct _698_selector4 selector4;
	struct _698_selector5 selector5;
	struct _698_selector6 selector6;
	struct _698_selector7 selector7;
	struct _698_selector8 selector8;
	struct _698_selector9 selector9;
	struct _698_selector10 selector10;	
};
struct _698_csd{//CSD ����������¼�Ͷ����м�¼���й����������ԡ�
	struct _698_oad oad;
	struct _698_road road;
};
struct _698_rcsd{//CSD ����������¼�Ͷ����м�¼���й����������ԡ�
	struct _698_csd * csd;
};
struct _698_tsa{//Ŀ���������ַTSA,��Ӧ��ʾ 1��16 ���ֽڳ��ȣ�
	unsigned char * tsa;
};
struct _698_region{//Ŀ���������ַTSA,��Ӧ��ʾ 1��16 ���ֽڳ��ȣ�
	unsigned char type;//ǰ�պ� �� 0����ǰ����� �� 1����ǰ�պ�� �� 2����ǰ���� �� 3��
	unsigned char * s_data;
	unsigned char *	e_data;
};
struct _698_oi{
	unsigned short oi;
};
struct _698_omd{//���󷽷�������OMD�� Object Method Descriptor��
	unsigned char oi[2];
	unsigned char method_id; //�����󷽷���š�
	unsigned char op_mode;		
};
struct _698_scaler_unit{
	char factor; //���㡪���������ӵ�ָ��������Ϊ 10������ֵ�������ֵģ�����Ӧ���� 0��
	int unit;	//��λ����ö�����Ͷ�������λ������� ¼ B��	
};
struct _698_mac{//����mac�ǵ���ַ������Ͳ�֪����ʲô��
	unsigned char * mac;
	int size_data;	
};

struct _698_rn{
unsigned char * _698_rn;
};
struct _698_symmetrysecurity{//���� 1 Ϊ�Կͻ�����������������ܵõ������ġ�
	unsigned char * security_text;
	unsigned char * sign;	
	int size_text;
	int size_data_text;		
	int size_sign;
	int size_data_sign;	
};
struct _698_signaturesecurity{//���� 2 Ϊ�ͻ�������վ���Է��������նˣ���������վ֤������ݼ�����Ϣ���ͻ���ǩ�� 2 Ϊ�ͻ��������� 2 ��ǩ����
	unsigned char * security_text;
	unsigned char * sign;	
};
struct _698_sid{
	unsigned char id[4]; //��ʶ double-long-unsigned��
	unsigned char * data;	//�������� octet-string	
	int size;
	int size_data;	
};
struct _698_sid_mac{//SAM ������ȫ��ʶ�Լ���Ϣ�����롣
	struct _698_sid sid; //
	unsigned char * _698_mac;	
	int size;
	int size_data;	
};
#define NullS 0
#define PasswordS 1
#define symmetrys 2
#define signatures 3

struct _698_connectmechanisminfo{//SAM ������ȫ��ʶ�Լ���Ϣ�����롣
	unsigned char connect_type;//�������� [0] NullSecurity���������� [0] NullSecurity��һ������ [1] PasswordSecurity���ԳƼ��� [2] SymmetrySecurity������ǩ�� [3] SignatureSecurity
	unsigned char	NullSecurity;
	unsigned char	* PasswordSecurity;	//һ������ [1] PasswordSecurity��
	struct _698_symmetrysecurity symmetrysecurity;//�ԳƼ��� [2] SymmetrySecurity��
	struct _698_signaturesecurity signaturesecurity;			//����ǩ�� [3] SignatureSecurity		
};
enum _698_connectresult{//SAM ������ȫ��ʶ�Լ���Ϣ�����롣
	permit_link
//������Ӧ������ �� 0����������� �� 1�����Գƽ��ܴ��� �� 2�����ǶԳƽ��ܴ��� �� 3����ǩ������ �� 4����Э��汾��ƥ�� �� 5������������ �� 255��
};
struct _698_securitydata{
	unsigned char type;
	struct _698_rn securitydata_rn; //����������� RN��
	unsigned char	* s_sign_info;  //������ǩ����Ϣ octet-string
	int size;
	int datesize;
};
struct _698_connectresponseinfo{
	unsigned char	connectresult;//��֤���
	struct _698_securitydata connectresponseinfo_sd;//��֤������Ϣ 
};
//00 ���� ������Ӧ���� ������Ӧ������ �� 0��
//00 ���� ��֤������Ϣ OPTIONAL=0 ��ʾû��
//00 ���� FollowReport OPTIONAL=0��ʾû���ϱ���Ϣ
//00 ���� û��ʱ���ǩ
struct _698_PIID{//PIID �����ڿͻ��� APDU�� Client-APDU���ĸ��������������У������������£�������Ӧ��Լ��Ӧ����ʵ��ϵͳҪ�������
									//bit7�� �������ȼ��� ����0��һ��ģ� 1���߼��ģ���.response APDU�У���ֵ���Ӧ.request APDU �е���ȡ�
									//bit0~bit5�� ������ţ� ���������Ʊ����ʾ 0��63����.response APDU�У���ֵ���Ӧ.request APDU �е���ȡ�
	unsigned char piid;
};
struct _698_PIID_ACD{//PIID-ACD �����ڷ����� APDU�� Server-APDU���ĸ���������������,bit6�� ������� ACD�� ����0�������� 1����������������piid�С�
	unsigned char piid_acd;
};
enum _698_dar {//���ݷ��ʽ��DAR�� Data Access Result��,DAR ����ö�ٷ�ʽ���������ݷ��ʵĸ��ֿ��ܽ��
	success=0, //�ɹ� �� 0�����ɹ� �� 0������ʱʧЧ �� 2�����ܾ���д �� 3��������δ���� �� 4��������ӿ��಻���� �� 5����
						 //���󲻴��� �� 6�������Ͳ�ƥ�� �� 7����Խ�� �� 8�������ݿ鲻���� �� 9������֡������ȡ�� �� 10���������ڷ�֡����״̬ �� 11����
						 //��дȡ�� �� 12���������ڿ�д״̬ �� 13�������ݿ������Ч �� 14���������/δ��Ȩ �� 15����ͨ�����ʲ��ܸ��� �� 16����						 
						 //��ʱ������ �� 17������ʱ������ �� 18������������ �� 19������ȫ��֤��ƥ�� �� 20����
						 //�ظ���ֵ �� 21����ESAM ��֤ʧ�� �� 22������ȫ��֤ʧ�� �� 23�����ͻ���Ų�ƥ�� �� 24������ֵ�������� �� 25�������糬�ڻ� �� 26����
						 //��ַ�쳣 �� 27�����Գƽ��ܴ��� �� 28�����ǶԳƽ��ܴ��� �� 29����ǩ������ �� 30�������ܱ���� �� 31����ʱ���ǩ��Ч (32)��
					   //����ʱ �� 33����ESAM �� P1P2 ����ȷ �� 34����ESAM �� LC ���� �� 35�������� �� 255��
	hard_ineffect,
	temporary_ineffect	
} ;
/*****
�����ǻ�����������
******/
/*
��������ö�٣�
���������ࣺ
  1��Ԥ���ӣ�
		a)����¼
				���������𣬿ͻ�����Ӧ���ùؼ���LINK
				link(.request,.confirm) link(.indication,.response)
				����1    ��Ӧ  129
				��������link
		b):����
				��֤Ԥ���ӳ��ڻ״̬��ֻҪ������֪���Ϳ����ˣ��һ��б�Ҫ֪��ô����֪����ÿ�γ�ʱ��
				�鿴һ�£���Ϊ�������ڿ���ͨ��hplc�����ݡ�
		c):	�˳���¼
				�������������߱��ʱ����������ʱ���Ĳ����Ϳ����ˡ���������λ����˭�����Ԥ��¼�أ���
				�Ƿ�����ô��
		
	2��Ӧ�����ӣ�
		a)������Ӧ�����ӣ�connect��
				�ɿͻ�������ȷ��˫��ͨѶ��Ӧ���ﾳ����һ�ε�¼��link��ʱ������Զ����ӡ�
				connect(.request,.confirm)   connect(.indication,.response)  
				һ���ͻ���ֻ��һ��Ӧ�����ӣ�������ύ��������ʱ���������µ�������ǰһ��Ӧ�������Զ�ʧЧ��
				��������connect
			
		b):�Ͽ���������
				�Ͽ�Ӧ�����ӣ�release��,ͨ������£����������þܾ������󡣲������������������Ͽ�Ӧ�����ӵ�����
				release(.request,.confirm)  release(.indication,.response)
				
				
		c):	��ʱ����
				Ӧ�����ӽ��������У�����Э��Ӧ�����ӵľ�̬��ʱʱ�䣬��������ͨ��ʱ��ﵽ��̬��ʱʱ��󣬷���
				����ʹ��release(.notification)֪ͨ�ͻ�����Ӧ������ʧЧ�����Ͽ����˷�����Ҫ�ͻ������κ���Ӧ��
				�ٷ���ʱ��ô�������½������ӣ�
		
	3�����ݽ�����
				ͨ���߼������������ʽӿڶ�������Ի򷽷���
				ֻ���ϱ���֪ͨ/ȷ���࣬����������/��Ӧ���ͣ�
				����/��Ӧ����ͨ�����̣��ͻ�������ĳ������xx.request,������Ӧ�ò���յ��ͻ�����������������Ӧ�ý���
				��������ָʾxx.indication,Ȼ��Ӧ�ý���ͨ�����÷���xx.respnse����Ӧ�ͻ������󣬿ͻ���Ӧ�ò���յ�
				��������Ӧ����ͻ���Ӧ�ý��̷��ط���ȷ��xx.confirm���ͻ�������������ȫ�Եȣ������������λ�ÿ��Ի�����
		a):��ȡ
				�ͻ�����������������Ӧ
				get(.request,.confirm)    get(.indication,.response)
				����ͨ���������ѯ��������֧�ֵĿ�ע����ϱ��ķ��񼯣����¼���ʱ�����ϱ��ȣ�

				��������get		
			
		b):����
				set(.request,.confirm)   set(.indication,.response)

				��������set		
				
				
		c):	����
				action(.request,.confirm)    action(.indication,.response)

				��������action



		b):�ϱ�
				֪ͨ/ȷ���࣬����������/��Ӧ����
				report(.notification,.confirm) (��������������֪ͨô��)  report(.indication,.response)
				ͨ�����̣��ο�P18,�������Ϳͻ���Ҳ�ǶԵȵġ�
				��������report				
				
				
		c):	����
				
				proxy(.request,.confirm)   proxy(.indication,.response)

				��������proxy						

ϸ����ʵ�֣�  
*/


/*�����¼���Ӧ�Ľṹ��
��ʼ�ļ����������汻��������һ�����õģ�˳��ʹд�������Ȼ˳��


*/
struct _698_protocolconformance{
	unsigned char protocolconformance[8];
	//Ӧ������Э�� Application Association �� 0��������������� Get Normal �� 1����������������������� Get With List �� 2���������¼�Ͷ������� Get Record �� 3����
	//��������������� Get Proxy �� 4�������������¼�Ͷ������� Get Proxy Record �� 5���������֡����֡ Get Subsequent Frame �� 6����
	//���û����������� Set Normal �� 7�����������û����������� Set With List �� 8�������ú��ȡ Set With Get �� 9�����������ö������� Set Proxy �� 10����
	//�������ú��ȡ�������� Set Proxy With Get �� 11����ִ�ж��󷽷� Action Normal �� 12��������ִ�ж��󷽷� Action With List �� 13����ִ�з������ȡ Action With List �� 14����
	//����ִ�ж��󷽷� Action Proxy �� 15��������ִ�к��ȡ Action Proxy With Get �� 16�����¼������ϱ� Active Event Report �� 17�����¼�β���ϱ� �� 18)��
	//�¼��������λ ACD �ϱ� �� 19)����֡���ݴ��� Slicing Service �� 20����Get-request ��֡ �� 21����Get-response ��֡ �� 22����Set-request ��֡ �� 23����
	//Set-response ��֡ �� 24����Action-request ��֡ �� 25����Action-response ��֡ �� 26����Proxy-request ��֡ �� 27����Proxy-response ��֡ �� 28����
	//�¼��ϱ���֡ �� 29����DL/T645-2007 �� 30������ȫ��ʽ���� �� 31������������ ID Ϊ 0 �Ķ�ȡ���� �� 32������������ ID Ϊ 0 �����÷��� �� 33��
};

struct _698_functionconformance{
	unsigned char functionconformance[16];
	//���������� �� 0����˫���й����� �� 1�����޹����ܼ��� �� 2�������ڵ��ܼ��� �� 3�����й����� �� 4�����޹����� �� 5������������ �� 6���������� �� 7�������� �� 8����
	//���طѿ� �� 9����Զ�̷ѿ� �� 10������������ �� 11����г������ �� 12����г������ �� 13��������ʧ��� �� 14�����๦�ܶ������ �� 15�����¼���¼ �� 16����
	//�¼��ϱ� �� 17�����¶Ȳ��� �� 18����״̬����⣨�磺�����/����ť�ǣ� �� 19������̫��ͨ�� �� 20��������ͨ�� �� 21��������ͨ�� �� 22������ý��ɼ� �� 23����
	//���� �� 24����ֱ��ģ���� �� 25����������� 12V ��� �� 26�����ѱ� �� 27�������ฺ��ƽ�� �� 28�������� �� 29�����ȶ� �� 30��
};

struct _698_FactoryVersion{
unsigned char manufacturer_code[4];//���̴��� visible-string(SIZE (4))��
unsigned char soft_version[4];//����汾�� visible-string(SIZE (4))��
unsigned char soft_date[6];//����汾���� visible-string(SIZE (6))��
unsigned char hard_version[4];//Ӳ���汾�� visible-string(SIZE (4))��
unsigned char hard_date[6];//Ӳ���汾���� visible-string(SIZE (6))��
unsigned char manufacturer_ex_info[8];//������չ��Ϣ visible-string(SIZE (8))

};
/*
*/
struct _698_RELEASE_Notification{//���ӷ���������
	struct _698_PIID_ACD rel_noti_piid_acd;
	struct _698_date_time date_time_establish;
	struct _698_date_time date_time_current;		
	int size;
};
struct _698_RELEASE_Response{
	struct _698_PIID_ACD rel_req_piid_acd;
	unsigned char rusult;//��� ENUMERATED{�ɹ� �� 0��}
};
struct _698_RELEASE_Request{
	struct _698_PIID rel_req_piid;
};
/*
result
������Ӧ������ �� 0����
������� �� 1����
�Գƽ��ܴ��� �� 2����
�ǶԳƽ��ܴ��� �� 3����
ǩ������ �� 4����
Э��汾��ƥ�� �� 5����
�������� �� 255��
*/
struct _698_connect_response{
	unsigned char type;
	unsigned char piid_acd;
	//unsigned char result;
	struct _698_FactoryVersion connect_res_fv;
	unsigned char apply_version[2];
	struct _698_protocolconformance connect_res_pro;
	struct _698_functionconformance connect_res_func;	
	unsigned char max_size_send[2];
	unsigned char max_size_rev[2];
	unsigned char max_size_rev_windown;//��λ�Ǹ�����������Ҫʵ�ֶ���������
	unsigned char max_size_handle[2]; //�ͻ������ɴ���APDU�ߴ�
	unsigned char connect_overtime[4];//������Ӧ�����ӳ�ʱʱ��
	struct _698_connectresponseinfo connect_res_info;//�����в������ȵ�����Ҫ��sizeof�Ƚ����ѣ������һ�����㳤�ȵ�ô
	unsigned char FollowReport;
	unsigned char time_tag;
	int size;
};
struct _698_connect_request{//����
	unsigned char type;
	unsigned char piid;
	unsigned char version[2];
	struct _698_protocolconformance connect_req_pro;
	struct _698_functionconformance connect_req_func;
	unsigned char max_size_send[2];
	unsigned char max_size_rev[2];
	unsigned char max_size_rev_windown;//��λ�Ǹ�����������Ҫʵ�ֶ��������� ��д��һ��
	unsigned char max_size_handle[2];//�ͻ������ɴ���APDU�ߴ�
	unsigned char connect_overtime[4];//������Ӧ�����ӳ�ʱʱ��
	struct _698_connectmechanisminfo connect_req_cmi;//�����в������ȵ�����Ҫ��sizeof�Ƚ����ѣ������һ�����㳤�ȵ�ô
	unsigned char time_tag;
	int size;
};
/*


*/
struct _698_link_response{
	unsigned char type;//LINK-Response
	unsigned char piid; ////�ο�	P35.�������ȼ� bit7 0,һ��� 1:�߼���,��piid_acd���; ��piid_acd��ȣ�bit0-bit5 �Ƿ�����ţ���piid_acd���
											//�����Ʊ����ʾ0.....63 ��piid_acd���	
	unsigned char result;//bit7 0 �����ţ�1 ���š�bit0-bit2 0�ɹ�  1��ַ�ظ�   2�Ƿ��豸   3 ��������
	struct _698_date_time date_time_ask;
	struct _698_date_time date_time_rev;	
	struct _698_date_time date_time_response;	
	int position;//˽��λ�ñ�������ĩ������һλ��λ�ã�Ҳ�ǳ���
};

static struct _698_link_response hplc_698_link_response;

struct _698_link_request
{
	unsigned char type;
	unsigned char piid_acd;//�ο�	P35��bit6 �� ������� ACD�� ����0�������� 1������
	unsigned char work_type;//��¼ �� 0�������� �� 1�����˳���¼ �� 2��
	unsigned char heartbeat_time[2];
	struct _698_date_time date_time;//��(��λchar)���գ����ڼ�(0�����գ�������)��ʱ���룬���ٺ���(��λchar)
	int position;
	
};
/*��ַ��sa*/
#define ADDR_SA_TYPE_MASK 0X3F /*0��ʾ����ַ,1��ʾͨ���ַ,2��ʾ���ַ,3��ʾ�㲥��ַ*/
#define ADDR_SA_LOGIC_ADDR_MASK ~(0X30)
#define ADDR_SA_ADDR_LENGTH_MASK 0X0F  //��ַ���ȣ�û�õ�

struct _698_ADDR        //P   ��ַ��a
{
	unsigned char sa;   //��bit0-bit3����,��Ӧ1��16,������Ҫ��һ     
	unsigned char s_addr[8];//������sa����
	unsigned char ca;//�ͻ�����ַCA��1�ֽ��޷���������ʾ��ȡֵ��Χ0��255��ֵΪ0��ʾ����ע�ͻ�����ַ��
	int s_addr_len;
};
struct _698_strategy{
	unsigned char lock1;//����־��ʹ�ô����У������lock,��ס�豸�����᲻��Ӱ�������豸ʹ��
	unsigned char lock2;//ʹ�������˫���������ٽ���Դ
	unsigned char heart_beat_time0;//�������ֽ��ں󣬸��ֽ���ǰ���Ǹ����򣬵�λ����
	unsigned char heart_beat_time1;
	unsigned char dev_type;//���������Ǵ����ﵽ�������������ӿڵģ��͵���
	unsigned char cmd_type;//���������Ǵ����ﵽ�������������ӿڵģ��͵���
	unsigned char heart_beat_flag;	
};	
/*
�ṹ������
��Ա����·�û�����
ϣ���ṹ�̶�
*/
//��֡ʹ�õĺ궨�塣
#define FRAME_APART_MASK ~(0XC0) //����
#define FRAME_APART_START 0X00  //��ʼ֡
#define FRAME_APART_RESPONSE 0X80 
//ȷ��֡�������ղ���ȷ�Ͳ���ȷ��֡����ʱ��ʱ�����õĳ��ŵ㣬�Ͼ���֡����ȫ�����ס�
//ȷ��֡��ʽ�Ǻ���û����·�����ݡ�
#define FRAME_APART_END 0X40 //���֡
#define FRAME_APART_MIDDLE 0XC0 //�м�֡
/*
��֡��ʽ��
		��Ҫ��֡ʱ��ǰ��������ֽڣ����ڷ�֡�Ŀ��ơ���λ�ں󣬵�λ��ǰ
			���壺bit0-bit11 ��֡���,ȡֵ0-4095��ѭ��ʹ��
            bit12-bit13  ����
				    bit14-bit15  ֡���ͣ�ʹ�ú궨�塣	
			����ӵľ�����·������ݡ�
			ÿһ֡����ҪӦ�𣬽������������Э�鶼�����з�֡
*/
struct usrData {
	unsigned char head;
};

/*����  ,���Ӧ�ó������,ûɶ��*/
#define MAX_LENGTH 0X3FFF
/*������c �����������*/
#define CON_DIR_START_MASK ~(0X3F)
#define CON_START_MASK (0X40)  //u   ��1   s��    0
#define CON_DIR_MASK (0X80)    //UTS ��0   STU �� 1 


#define CON_UTS_S ((0<<7)|(0<<6))   /*����UTS �������Ƿ�����S ,����ͻ����Է������ϱ�����Ӧ*/
#define CON_UTS_U (0<<7)|(1<<6)     /*�����Ƿ��������������ǿͻ���*/
#define CON_STU_U (1<<7)|(1<<6)
#define CON_STU_S (1<<7)|(0<<6)

#define CON_MORE_FRAME (1<<5)   /*��֡��־λ*/

#define CON_FUNCTION_MASK 0X02

#define CON_LINK_MANAGE 0X1 /*��·���ӹ�����¼,����,�˳���¼��*/
#define CON_U_DATA 			 0X3 /*Ӧ�����ӹ������ݽ�������*/


//��󳤶�
#define  HPLC_DATA_MAX_SIZE 1024

struct _698_FRAME       //698Э��ṹ,�����ò���,�Է��õĵ�
{
	unsigned char head;  //��ʼ֡ͷ = 0x68
	unsigned char length0;   //����,�����ֽ�,��bit0-bit13��Ч,�Ǵ���֡�г���ʼ�ַ��ͽ����ַ�֮���֡�ֽ������y
	unsigned char length1;
	unsigned int length;
	unsigned char control;   //������c,bit7,���䷽��λ
	struct _698_ADDR addr;//��ַ��a
	unsigned char HCS0;//֡ͷУ��,�Ƕ�֡ͷ���ֳ���ʼ�ַ���HCS����֮��������ֽڵ�У��
	unsigned char HCS1;
	unsigned char *usrData;//��·�û�����,�����ã�����ȥ��[HPLC_DATA_MAX_SIZE]
	//struct usrData;//��·�û�����
	int usrData_len;//����Ľ���λ
	int usrData_size;//�û�������ܳ��ȣ�����ʱ����ȵ�usrDataָ��Ҫָ�ĵط�
	unsigned short FCS0;//֡У��
	unsigned short FCS1;
	unsigned char end;//�����ַ� = 0x16	
	struct _698_date_time rev_tx_frame_date_time;
//	unsigned char link_ok;	//1������˵�¼�������Ŀ��Բ������
//	unsigned char connect_ok;//������û�������
	unsigned char frame_apart;//Ĭ����0�����ǲ���֡���������0,��һ��ͻᱻ����
	unsigned char frame_no;
	unsigned int list_no;//ֻ�е�һ��������
	struct _698_strategy strategy; 
	int need_package;//���Ϊ1�Ƿ��ͺ�Ӧ���֡
	int time_flag_positon;	
};



/*

�ַ���ָ��Ĺ���ṹ��

*/
struct CharPointDataManage        //
{
	struct _698_FRAME _698_frame;
	unsigned char priveData[HPLC_DATA_MAX_SIZE];
	unsigned int size;
	unsigned int dataSize;
	struct CharPointDataManage *next;
	int list_no;//һ���Ǹ�λ�ӣ����˾�
	
};

//static struct CharPointDataManage _698_usr_data_apdu;//���������û�������

//�����в��������ݣ����Ƿ����п��ܻ��յ���Ԥ�ڵ����ݣ�������Ԥ���յ�������ʱ�ŵ�

/*�ṹ������

�ṹ����;��ĳ��ʹ��698Э��Ķ˿ڵ�״̬��

*/
struct _698_STATE        //��ַ��a
{
	unsigned char link_flag;   // ��¼����1 
	unsigned char connect_flag;//���Ӻ���1
	int try_link_type;//���ص�û�б�־�����ֵ�¼����
	unsigned char heart_beat_time0;//�������ֽ��ں󣬸��ֽ���ǰ���Ǹ����򣬵�λ����
	unsigned char heart_beat_time1;
	struct _698_date_time last_link_requset_time;
	struct _698_ADDR addr;
	unsigned char piid;//�������ȥ��
	unsigned char meter_addr_send_ok;
	unsigned char version[2];
	unsigned char protocolconformance[8];
	unsigned char functionconformance[16];
	unsigned char connect_overtime[4];
	struct _698_FactoryVersion FactoryVersion;	
	struct _698_oad oad_omd;
	unsigned char lock1;//����־��ʹ�ô����У������lock,��ס�豸�����᲻��Ӱ�������豸ʹ��
	unsigned char lock2;//ʹ�������˫���������ٽ���Դ
	int HCS_position;
	int FCS_position;
	int len_position;
	int USART_RX_STA;
	int len_left;
	int len_all;	
	int len_sa;
	int FE_no;	
};


static unsigned short fcstab[256]={
0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/*
����
*/

unsigned short pppfcs16(unsigned short fcs, unsigned char *cp, int len);
int tryfcs16(unsigned char *cp, int len);
int _698_FCS(unsigned char *data, int start_size,int size,unsigned short FCS);
int _698_analysis(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_wait_list);
int rev_698_del_affairs(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev);
int _address_unPackage(unsigned char * data,struct _698_ADDR *_698_addr,int size);
int _698_unPackage(unsigned char * data,struct  _698_FRAME  *_698_frame_rev,int size);
int _698_package(unsigned char * data,int size);
int unPackage_698_link_request(struct  _698_FRAME  *_698_frame,struct _698_link_request * request,int * size);
int unPackage_698_connect_request(struct _698_STATE  * priv_698_state,struct  _698_FRAME  * _698_frame,struct _698_connect_response * prive_struct);
int _698_HCS(unsigned char *data, int start_size,int size,unsigned short HCS);
int priveData_analysis(struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx);
int my_strcpy(unsigned char *dst,unsigned char *src,int startSize,int size);
int save_char_point_data(struct CharPointDataManage *hplc_data,int position,unsigned char *Res,int size);
int array_inflate(unsigned char *data, int size,int more_size);
int array_deflate(unsigned char *data, int size,int de_size);
int hplc_data_inflate(struct CharPointDataManage *data_rev);
int save_char_point_usrdata(unsigned char *data,int *length,unsigned char *Res,int position,int size);
int get_current_time(unsigned char * data );
int get_date_time_s(struct _698_date_time_s *date_time_s);
int hplc_tx_frame(struct _698_STATE  * priv_698_state,rt_device_t serial,struct CharPointDataManage * data_tx);
int link_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_FRAME  *_698_frame_send,struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx);
int get_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int connect_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx);
int copy_to_work_wait_list(struct CharPointDataManage *hplc_data,struct CharPointDataManage * hplc_data_wait_list);
int connect_request_package(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int link_request_package(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int copy_char_point_data(struct CharPointDataManage * des,struct CharPointDataManage * source);
int copy_698_frame(struct  _698_FRAME * des,struct  _698_FRAME  * source);
int init_CharPointDataManage(struct CharPointDataManage *des);
int free_CharPointDataManage(struct CharPointDataManage *des);
int init_698_FRAME(struct  _698_FRAME  * des);
int free_698_FRAME(struct  _698_FRAME  * des);
int iterate_wait_response_list(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_list);
int iterate_wait_request_list(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_list);
int init_698_state(struct _698_STATE  * priv_698_state);
int hplc_priveData_analysis(struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx);
int hplc_package(unsigned char * data,int size);
int clear_data(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct  _698_FRAME  *_698_frame);
int save_hplc_data(struct CharPointDataManage *data_rev,int position,unsigned char Res);
int printmy(struct  _698_FRAME  *_698_frame);
int get_single_frame_frome_hplc(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx);
int hplc_tx(struct CharPointDataManage *hplc_data);
int hplc_inition(struct CharPointDataManage *  data_wait_list,struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx);
void hplc_thread_entry(void * parameter);
int hplc_645_addr_receive(struct CharPointDataManage *data_rev);
int hplc_645_addr_response(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx);
int init_698_state(struct _698_STATE  * priv_698_state);
int my_free(unsigned char  *des);
int oad_package(struct _698_oad *priv_struct,struct  _698_FRAME  *_698_frame_rev,int position);
int get_response_package_normal(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int get_response_parameter_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int get_response_normal_oad(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int oi_parameter_get_addr(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int get_data_class(struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data,enum Data_T data_type);
int send_event(void);
extern rt_uint8_t CtrlUnit_RecResp(rt_uint32_t cmd,void *STR_SetPara,int count);
int STR_SYSTEM_TIME_to__date_time_s(STR_SYSTEM_TIME * SYSTEM_TIME,struct _698_date_time_s *date_time_s);
int my_strcpy_char(char *dst,char *src,int startSize,int size);
int action_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int Report_Cmd_ChgRecord(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int Report_Cmd_ChgPlanExeState(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int check_afair_from_botom(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx);
extern rt_uint8_t strategy_event_get(COMM_CMD_C cmd);
extern rt_uint8_t strategy_event_send(COMM_CMD_C cmd);
int package_for_test(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int hplc_tx(struct CharPointDataManage *hplc_data);
int charge_strategy_package(CHARGE_STRATEGY *priv_struct_STRATEGY,struct CharPointDataManage * hplc_data);
int charge_exe_state_package(CHARGE_EXE_STATE *priv_struct,struct CharPointDataManage * hplc_data);	
int plan_fail_event_package(PLAN_FAIL_EVENT *priv_struct,struct CharPointDataManage * hplc_data);
int Report_Cmd_DeviceFault(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int Report_Cmd_PileFault(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);









