
#include "drv_gpio.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <698.h>
#include <esam.h>
#include <storage.h>
//extern rt_uint8_t GetStorageData(STORAGE_CMD_ENUM cmd,void *STO_GetPara,rt_uint32_t datalen);
//��������
int test_dis_check=0;//�Խ������ݲ�����ͷβУ��Ŀ���

unsigned char _698_ChgPlanIssue_data[1024]={
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

//��ȡesam��Ϣ
unsigned char esam_data[1024]={0x68 , 0x2a , 0x00 , 0x43 , 0x05 , 0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x06 , 0xc2 , 0x05 , 0x02 , 0x20 , 0x03 , 
	0xf1 , 0x00 , 0x02 , 0x00 , 
	0xf1 , 0x00 , 0x04 , 0x00 , 
	0xf1 , 0x00 , 0x07 , 0x00 , 
	0x01 , 0x07 , 0xe3 , 0x07 , 0x1a , 0x12 , 0x24 , 0x2f , 0x00 , 0x00 , 0x00 , 0x91 , 0x6d , 0x16
};





//esam
ScmEsam_Comm hplc_ScmEsam_Comm;

ESAM_CMD hplc_current_ESAM_CMD;
//other


struct rt_event PowerCtrlEvent;


#define PPPINITFCS16 0xffff // initial FCS value /
#define PPPGOODFCS16 0xf0b8 // Good final FCS value 
#define my_debug 1
//struct rt_event PowerCtrlEvent;
struct rt_event HplcEvent;
#define THREAD_HPLC_PRIORITY     15
#define THREAD_HPLC_STACK_SIZE   1024*9
#define THREAD_HPLC_TIMESLICE    7
//�·�����
struct  _698_FRAME _698_ChgPlanIssue;

CHARGE_STRATEGY charge_strategy_ChgPlanIssue;//���ƻ��·�

CHARGE_STRATEGY charge_strategy_ChgRecord;   //���ͳ�綩��
CHARGE_STRATEGY_RSP ChgPlanIssue_rsp;

struct  _698_FRAME _698_ChgPlanIssueGet;
unsigned char _698_ChgPlanIssueGet_data[100];

_698_CHARGE_STRATEGY _698_charge_strategy;

_698_PLAN_FAIL_EVENT _698_router_fail_event;

_698_PLAN_FAIL_EVENT _698_pile_fail_event;


struct  _698_FRAME _698_ChgPlanAdjust;
unsigned char _698_ChgPlanAdjust_data[1024];
CHARGE_STRATEGY charge_strategy_ChgPlanAdjust;

CHARGE_STRATEGY_RSP ChgPlanAdjust_rsp;


struct  _698_FRAME _698_StartChg;
unsigned char _698_StartChg_data[50];
//START_STOP_CHARGE start_charge;




struct  _698_FRAME _698_StopChg;
unsigned char _698_StopChg_data[50];
//START_STOP_CHARGE stop_charge;



//���ͽṹ��
CHARGE_EXE_STATE Report_charge_exe_state;

struct rt_thread hplc;


int hplc_lock1=0,hplc_lock2=0;
rt_uint32_t strategy_event;
rt_uint32_t hplc_event=0;


rt_uint8_t hplc_stack[THREAD_HPLC_STACK_SIZE];//�̶߳�ջ

//int priv_698_state->USART_RX_STA=0,times=0;      					//�жϳ�ʱ�� ,��ʱ��ʱ������	
rt_device_t hplc_serial; 	// �����豸��� 

rt_device_t blue_tooth_serial; 	// �����豸��� 

struct _698_STATE hplc_698_state;

void hplc_thread_entry(void * parameter){
	int result,i=0;
	struct CharPointDataManage hplc_data_rev,hplc_data_tx;
	struct CharPointDataManage hplc_data_wait_list;//�ò��ܳ���2����ʵ��λ�ͻ�������֡��󴰿ڳߴ�
	
	rt_kprintf("[hplc]  (%s)  start !!! \n",__func__);
	
	result=rt_event_init(&HplcEvent,"HplcEvent",RT_IPC_FLAG_FIFO);
	if(result!=RT_EOK){		
			rt_kprintf("[hplc]  (%s)  rt_event Hplc faild! \n",__func__);
	}

	hplc_inition(&hplc_data_wait_list,&hplc_data_rev,&hplc_data_tx);
	init_698_state(&hplc_698_state);
	rt_memset(hplc_698_state.last_link_requset_time.data,0,10);	//��ʱ���ȥ

	
	
	while(1){
		
//	if(1){
//		rt_thread_mdelay(2000);	
//		test_dis_check=1;
		
		result=get_single_frame_frome_hplc(&hplc_698_state,&hplc_data_rev,&hplc_data_tx);
		if(result!=0){//�������������Ϊ�˷��������ӿڿ��ܵĵ��ã�������󣬽���ȥ����
			if(result==1){//��645Э���ȡ������͵���ַ��������һ֡��hplc���ᷢ��698��Ҫ���Э�飩
				hplc_645_addr_response(&hplc_698_state,&hplc_data_rev,&hplc_data_tx);				
				hplc_tx_frame(&hplc_698_state,hplc_serial,&hplc_data_tx);
				hplc_698_state.meter_addr_send_ok=1;//������ʱ��������0
				clear_data(&hplc_698_state,&hplc_data_rev,&hplc_data_rev._698_frame);//�������˺���������	,ֻ������Ƕ�̬�ĺ���������˿ռ��Ͳ�����
			}//���������Ϊ�ǳ�ʱ,ȥ���洦����������
			
			if(result==2){//�˳�������Ϊ�ܵ����¼�����ִ��clear_data
				//�������				
			}	
		}else{//�����ȷ���յ���һ֡
			if(hplc_priveData_analysis(&hplc_data_rev,&hplc_data_tx)==0){//����֡����	,���ṹ�壬����У��														
			//����һ���ɷ�����������֡��Ҫ����
				rt_kprintf("[hplc]  (%s)  	RX:\n",__func__);
				printmy(&hplc_data_rev._698_frame);	//����
				if(judge_meter_no(&hplc_698_state,&hplc_data_rev)==0){
					result=_698_analysis(&hplc_698_state,&hplc_data_tx,&hplc_data_rev,&hplc_data_wait_list);
					
					if( result!=0){//��ֱ�Ӹ�ֵ������ǿ������ת��������ֵ��Ϳ���ֱ�Ӳ���_698_frame_rev	
						//�����ǲ���Ҫ�ظ��������������
						if(result==1){//
							rt_kprintf("[hplc]  (%s)  not need to send and out\n",__func__);//���յ�����Ӧ���ʱ��Ͳ���Ҫȥ������
						}else if(result==2){
							rt_kprintf("[hplc]  (%s)  for charge\n",__func__);
						}else{
							rt_kprintf("[hplc]  (%s)  error result=%d\n",__func__,result);//						
						}						
					}else{//��������Ҫ�ظ������
						rt_kprintf("[hplc]  (%s)  TX:\n",__func__);
//					printmy(&hplc_data_tx._698_frame);
						rt_kprintf("[hplc]  (%s)  time = %0x:%0x:%0x\n",__func__,System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second);
						hplc_tx_frame(&hplc_698_state,hplc_serial,&hplc_data_tx);//��������
					}					
				}					
			}	
			clear_data(&hplc_698_state,&hplc_data_rev,&hplc_data_rev._698_frame);//�������˺���������	,ֻ������Ƕ�̬�ĺ���������˿ռ��Ͳ�����	
								
		}	
		//rt_kprintf("[hplc]  (%s) link_flag=%d hplc_698_state.connect_flag=%d\n",__func__,hplc_698_state.link_flag,hplc_698_state.connect_flag);
		
		if(0){
			if((hplc_698_state.link_flag==0)||(hplc_698_state.connect_flag==0)){//�����link�ĳ�ʱ�ط�֡������һ֡����֡�����ߴ��ڽ��ճ�ʱ������뵽������档
											
//				if(hplc_698_state.meter_addr_send_ok==2){//��ȡ�������ɿͻ�������ģ��� link_request�����ɷ����������				
					if(link_request_package(&hplc_data_tx,&hplc_698_state)==0){  //����0����������Ҫ���͵�֡,ֻ��������֡������֡�ڳ�ʱ���淢
							hplc_tx_frame(&hplc_698_state,hplc_serial,&hplc_data_tx);
							get_current_time(hplc_698_state.last_link_requset_time.data);//���ͳɹ��˸��·���ʱ��
							//���õȴ��б����
					}
//					printmy(&hplc_data_tx._698_frame);					
//				}	
								
				//rt_kprintf("[hplc]  (%s) link_flag=%d meter_addr_send_ok=%d\n",__func__,hplc_698_state.link_flag,hplc_698_state.meter_addr_send_ok);
			}else{
				//����Ӧ�ò��ύ������		
			}
		}			
	}//while��1��	
}



/*
	��hplc�ж�ȡһ��֡
*/
int get_single_frame_frome_hplc(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx){
	
	/*�������ݣ��յ����ݳ��Ⱥ󣬾Ϳ���˽�����ݿռ䣬�������ݸ�ֵ��˽�����ݿռ���
	��������������*/
	
	unsigned char Res;
	int times=0;      					//�жϳ�ʱ�� ,��ʱ��ʱ������	
	int tempSize;
	int result=-1;
	if(1){//�鿴�ײ���û���¼��ϴ�����	
//		rt_kprintf("[hplc]  (%s)  top \n",__func__);
		check_afair_from_botom(priv_698_state,data_tx);
	}	
	
	while(1){
		tempSize=rt_device_read(hplc_serial, 0, &Res, 1) ;//һֱ����������ݣ�ֱ�����գ���ʱ�˳���
		
//	for(int i=0;i<500;i++){
//		tempSize=1;
//		Res=esam_data[i];
		
		if(tempSize==1){
			times=0;    //�жϳ�ʱ�� ��������������			 
			//���յ����ݳ���
			if(priv_698_state->USART_RX_STA&0x40000000){             //�Ѿ����յ���һ֡�ĵ�һ��0x68				
				if(save_hplc_data(data_rev,data_rev->dataSize,Res)<0)//���յ�������ȫ�����չ��������㷢��		
				{
					rt_kprintf("[hplc]  (%s) get_single_frame save_hplc_data error\n",__func__);
					clear_data(priv_698_state,data_rev,&data_rev->_698_frame); 	//����ԭ���˳���Ҳ�������������           													
				}else{//
					
					if(priv_698_state->USART_RX_STA&0x20000000){    //���յ����ݳ���,����len�ǽ��յ�����+2���ȵ��ֽ�

						if(!(priv_698_state->USART_RX_STA&0x08000000)){//��δ����ͷУ��
					
							if(priv_698_state->USART_RX_STA&0x10000000){    //sa��ַ���ȣ�Ҳ��֪����ͷУ���λ�ã�
								if(data_rev->dataSize==(8+priv_698_state->len_sa)){    //��ʾ����ͷУ��λ
									priv_698_state->USART_RX_STA|=0x08000000;        //�ñ�־λ	
									////����							
									if(!test_dis_check){		
										if(_698_HCS(data_rev->priveData,1,7+priv_698_state->len_sa,0)<0){             //У�鲻��
											result=hplc_645_addr_receive(data_rev);
											if(result==1){
												rt_kprintf("[hplc]  (%s)  is 645!!!!\n",__func__);
												return result;
											}
											clear_data(priv_698_state,data_rev,&data_rev->_698_frame); 	//����ԭ���˳���Ҳ�������������    			
											//continue;//��ò������,���Բ���	
										}	
									}
								}								
							}else{
								if((data_rev->dataSize)==5){    //��ʾ�����5���������յ�sa���ݳ���
									priv_698_state->USART_RX_STA|=0x10000000;        //�ñ�־λ
									if((data_rev->priveData[1]==0xaa)&&(data_rev->priveData[2]==0xaa)){
										rt_kprintf("[hplc]  (%s) [1]==0xaa [2]==0xaa \n",__func__);
										priv_698_state->len_sa=4;//�����12����
									}else{									
										if(priv_698_state->len_all > HPLC_DATA_MAX_SIZE){
											rt_kprintf("[hplc]  (%s) len_all=%d>HPLC_DATA_MAX_SIZE \n",__func__,priv_698_state->len_all);
											clear_data(priv_698_state,data_rev,&data_rev->_698_frame);
											return -1;								
										}
										priv_698_state->len_sa=(data_rev->priveData[4]&0X0F)+1;//sa��ַ���ȣ���Ҫ���һλ
//										rt_kprintf("[hplc]  (%s) len_sa=%d\n",__func__,priv_698_state->len_sa);
									}	
								}
							}
						}
											
						priv_698_state->len_left--;		       //�ж��Ƿ��������		�����������˾Ͷ����һ�Ρ�    				
						if(priv_698_state->len_left<=0){     //����ȫ���������ˣ��ж����һλ�ǲ���19����У��								
							if(data_rev->priveData[(data_rev->dataSize-1)]!=0x16){//����λ����								
								clear_data(priv_698_state,data_rev,&data_rev->_698_frame); 	//����ԭ���˳���Ҳ��������������˴���������־											
								rt_kprintf("[hplc]  (%s)   last date!=0x16\n",__func__);
								//������645
								return -1;//��ò������			
							}else {									
								priv_698_state->USART_RX_STA|=0x04000000;      //������ɣ����ٽ�������֡����			

								rt_kprintf("[hplc]  (%s)  get right frame!!!!! first check use��s business\n",__func__);								
								if(1){//�鿴�ײ���û���¼��ϴ�����	
									check_afair_from_botom(priv_698_state,data_tx);
								}									
								return 0;														
							}											
						}												
					}else{                               //�ȴ��������ݳ���		
						if((data_rev->dataSize)==3){    //��ʾ����������������յ����ݳ���
							priv_698_state->USART_RX_STA|=0x20000000;        //���յ����ݳ��ȣ��ñ�־λ
						
							priv_698_state->len_all=data_rev->priveData[2]*256+data_rev->priveData[1]+2;//��ȡlen�ǽ��յľ���+�����һλУ��λ��һλ0x19								
							if(priv_698_state->len_all<12){//���ٵ���Щ��
								clear_data(priv_698_state,data_rev,&data_rev->_698_frame);	
								return -1;
							}else{
								priv_698_state->len_left=priv_698_state->len_all-3;						
							}														
//							rt_kprintf("[hplc]  (%s)  len_all=%d \n",__func__,priv_698_state->len_all);													
						}
					}
					
				}
			}else{//��δ�յ���һ��68
//				if(Res==0x68&&(priv_698_state->FE_no>=4)){                                    //��һ��68
//					rt_kprintf("[hplc]  (%s)    FE_no>=4 \n",__func__);
				if(Res==0x68){ 
					rt_kprintf("[hplc]  (%s)    get 0x68 \n",__func__);
					priv_698_state->FE_no=0;
					if(save_hplc_data(data_rev,data_rev->dataSize,Res)<0)//�յ�������ȫ�����չ��������㷢��		
					{
						clear_data(priv_698_state,data_rev,&data_rev->_698_frame);										
					}else{
						priv_698_state->USART_RX_STA|=0x40000000;                       //�յ���һ֡�е�һ��08�ı�־λ
						times=0;                                        //�жϳ�ʱ�� ����ʱ��ʱ������					
					}															
				}	
//				if(Res==0xfe){ //��ͷ�����ĸ�fe
//					priv_698_state->FE_no+=1;
//					times=0;		
//				}else{
//					priv_698_state->FE_no=0;				
//				}				
			}			
		}else{//�ܳ�ʱ��û�յ�����ʱ�˳�����������ҵ��
			if(1){//�����ﴦ��Ҫ���͵��¼��Ͳ��ù������豸��
						//�鿴�ײ���û���¼��ϴ�����
//				rt_kprintf("[hplc]  (%s)  end\n",__func__);			
				check_afair_from_botom(priv_698_state,data_tx);		
			}			
			rt_thread_mdelay(200);						
	
			times++;
			if(times==30){//10���жϳ�ʱ,ȫ���
				times=0;
//				rt_kprintf("[hplc]  (%s) priv_698_state->len_left=%d over time! \n",__func__,priv_698_state->len_left);				
				clear_data(priv_698_state,data_rev,&data_rev->_698_frame);//�������˺���������	,ֻ������Ƕ�̬�ĺ���������˿ռ��Ͳ�����
				return -1;
			}
		}
	}	
}

/*
�ظ�ȫ����

*/
int check_afair_from_botom(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_tx){
	
	int result=-1;	
	while(hplc_698_state.lock1==1){
		rt_kprintf("[hplc]  (%s)   lock1==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_698_state.lock1=1;
	while(hplc_698_state.lock2==1){
		rt_kprintf("[hplc]  (%s)   lock2==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_698_state.lock2=1;
	
//		if(hplc_event&(0x1<<)){	//ת���������	
//		hplc_event&=(~(0x1<<Cmd_ChgPlanIssueAck));	
//		rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanIssueAck  \n",__func__);
//		data_tx->priveData=;
//		data_tx->dataSize=;
//		data_tx->_698_frame.usrData_len=data_tx->dataSize-10-data_tx->priveData[4];
//	
//	if(data_tx->_698_frame.usrData_len>0){
	

//		security_get_package(priv_698_state,data_tx);

//		hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
//	}		
//	}		
	
	if(hplc_event&(0x1<<Cmd_ChgPlanIssueAck)){	//���ƻ��·�Ӧ��	
		hplc_event&=(~(0x1<<Cmd_ChgPlanIssueAck));	
		rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanIssueAck  \n",__func__);
		_698_ChgPlanIssue.need_package=1;
		//��С�ܵ�������ҵ�_698_ChgPlanIssue��Ȼ�������Ӧ��֡
		
		result=action_response_package(&_698_ChgPlanIssue,priv_698_state,data_tx);//����

		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
//			if(security_flag==1){//�ǰ�ȫ����,��Ҫ&&�Ѿ���ԿЭ�̹��ˣ��ƺ�ֻ����Կ������
				//���û����������ؼ��ܣ�Ȼ�����´��

				data_tx->dataSize=9+data_tx->_698_frame.usrData[4];
				result=security_get_package(priv_698_state,data_tx);
//			}	
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
		}		
	}		

	if(hplc_event&(0x1<<Cmd_ChgPlanAdjustAck)){	//���ƻ�����Ӧ��	
		rt_kprintf("[hplc]  (%s)  Cmd_ChgPlanAdjustAck  \n",__func__);	
		_698_ChgPlanAdjust.need_package=1;
		result=action_response_package(&_698_ChgPlanAdjust,priv_698_state,data_tx);//����	
		_698_ChgPlanAdjust.need_package=0;
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			
			data_tx->dataSize=9+data_tx->_698_frame.usrData[4];
			result=security_get_package(priv_698_state,data_tx);
			
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
		}		
	}	
	if(hplc_event&(0x1<<Cmd_ChgPlanIssueGetAck)){		
		rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanIssueGetAck  \n",__func__);	
		_698_ChgPlanIssueGet.need_package=1;
		result=get_response_package(&_698_ChgPlanIssueGet,priv_698_state,data_tx);//����
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			rt_kprintf("[hplc]  (%s)  print data_tx:\n",__func__);
			
			data_tx->dataSize=9+data_tx->_698_frame.usrData[4];
			result=security_get_package(priv_698_state,data_tx);
			
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������
			//printmy(&data_tx->_698_frame);
		}
	}	
	
	
//	rt_kprintf("[hplc]  (%s)   hplc_event=%4x  \n",__func__,hplc_event);
	if(hplc_event&(0x1<<Cmd_StartChgAck)){	//�������Ӧ��
		hplc_event&=(~(0x1<<Cmd_StartChgAck));	
		rt_kprintf("[hplc]  (%s)   Cmd_StartChgAck  \n",__func__);

		_698_StartChg.need_package=1;
		result=action_response_package(&_698_StartChg,priv_698_state,data_tx);//����
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			
			data_tx->dataSize=9+data_tx->_698_frame.usrData[4];
			result=security_get_package(priv_698_state,data_tx);
			
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
		}		
	}

	if(hplc_event&(0x1<<Cmd_StopChgAck)){	//�������Ӧ��
		hplc_event&=(~(0x1<<Cmd_StopChgAck));	
		rt_kprintf("[hplc]  (%s)   Cmd_StopChgAck  \n",__func__);	
		_698_StopChg.need_package=1;
		result=action_response_package(&_698_StopChg,priv_698_state,data_tx);//����
		_698_StopChg.need_package=0;
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			
			data_tx->dataSize=9+data_tx->_698_frame.usrData[4];
			result=security_get_package(priv_698_state,data_tx);
			
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
		}				
	}		
	

	
	
	
	
	
	
	
	
	if(hplc_event&(0x1<<Cmd_ChgPlanExeState)){	//						
		rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanExeState  \n",__func__);	
		result=Report_Cmd_ChgPlanExeState(data_tx,priv_698_state);
		//printmy(&data_tx->_698_frame);			
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			//rt_kprintf("[hplc]  (%s)  print data_tx:\n",__func__);	
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
		}													
	}
	
	
	if(hplc_event&(0x1<<Cmd_ChgRecord)){	//���ͳ�綩�� REPORT  ֻ���ϱ����ɸ��������ԣ����ϱ����ɸ���¼�Ͷ�����������
		rt_kprintf("[hplc]  (%s)   Cmd_ChgRecord  \n",__func__);
//		result=Report_Cmd_ChgRecord(data_tx,priv_698_state);//Cmd_ChgPlanExeState();//
		
			report_notification_package(Cmd_ChgRecord,data_tx,data_tx,priv_698_state);
			
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			//rt_kprintf("[hplc]  (%s)  print data_tx:\n",__func__);	
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
		}				
	}	

	if(hplc_event&(0x1<<Cmd_DeviceFault)){	//����·�����쳣״̬
		rt_kprintf("[hplc]  (%s)   Cmd_DeviceFault  \n",__func__);	
//		result=Report_Cmd_DeviceFault(data_tx,priv_698_state);
		report_notification_package(Cmd_DeviceFault,data_tx,data_tx,priv_698_state);		
		
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			//rt_kprintf("[hplc]  (%s)  print data_tx:\n",__func__);	
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
		}					
	}	
	if(hplc_event&(0x1<<Cmd_PileFault)){	//����·�����쳣״̬
		rt_kprintf("[hplc]  (%s)   Cmd_DeviceFault  \n",__func__);	
//		result=Report_Cmd_PileFault(data_tx,priv_698_state);
		if( result!=0){
				rt_kprintf("[hplc]  (%s)    error \n",__func__);//												
		}else{//��������Ҫ�ظ������
			//rt_kprintf("[hplc]  (%s)  print data_tx:\n",__func__);	
			hplc_tx_frame(priv_698_state,hplc_serial,data_tx);//��������	
			report_notification_package(Cmd_DeviceFault,data_tx,data_tx,priv_698_state);				
		}				
	}				
	hplc_698_state.lock2=0;
	hplc_698_state.lock1=0;

	return result;		
}



/*
 ���ͣ����÷��͵���֡��Ӧ����֡ʱ�ǲ�����Ҫ�ȴ�hplc�ظ���������Ҫ���ǲ��Ǿ�û����֡����Ҫ����֡����Ҫһ��һȷ��ô��


*/
int hplc_tx(struct CharPointDataManage *hplc_data){
	int result=1;
	//unsigned char rel_time[10];//9λʱ�䣬��һλ��Ӧ����ʱ���
	
	//    �����ַ��� 
  //rt_device_write(serial, 0, str, (sizeof(str) - 1));
	//��ȡʵʱʱ��,������ʱ���
	//�������͵��̣߳���Ӱ����������? ������������? �������ͱȽϺ�ʵ�֣�������û����������£��յ�Ҳû�ã�������ڻ�������ǳ�֡��ô��
	//��һ��Ҫ�ж��ǲ��ǹ����ţ���Ϊ�п���Ҫת�������û���֡�����ָ���������û���hplc�ڷ������ݣ����������û����ýӿں�������֡��
	//������ݣ������ͺ�����������󣬽�����һ�������߼���
	return result;	
}


int hplc_645_addr_response(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx){
//68 32 00 00 00 
//00 00 68 93 06 
//65 33 33 33 33 
//33 FF 16 //18��
	int i;
	data_tx->priveData[0]=0x68;
	my_strcpy(data_tx->priveData+1,priv_698_state->addr.s_addr,0,priv_698_state->addr.s_addr_len);
	data_tx->priveData[7]=0x68;
	data_tx->priveData[8]=0x93;
	data_tx->priveData[9]=0x06;		
	my_strcpy(data_tx->priveData+10,priv_698_state->addr.s_addr,0,priv_698_state->addr.s_addr_len);
	for(i=0;i<6;i++){
		data_tx->priveData[10+i]+=0x33;	
	}
	data_tx->priveData[16]=0;
	for(i=0;i<16;i++){
		//rt_kprintf("\n[hplc]  (%s)  hplc_645_addr_response  priveData[%d]=%02x\n",__func__,i,data_tx->priveData[i]);
		data_tx->priveData[16]+=data_tx->priveData[i];	
	}


	data_tx->priveData[17]=0x16;
	data_tx->dataSize=18;
	return 0;
}

void hplc_PWR_ON(void)	
{					
	rt_pin_mode(GET_PIN(G,4),PIN_MODE_OUTPUT);//power-on,
	rt_pin_write(GET_PIN(G,4),PIN_HIGH);

	rt_pin_mode(GET_PIN(G,3),PIN_MODE_OUTPUT);//	
	rt_pin_write(GET_PIN(G,3),PIN_HIGH);//RESET,�͵�ƽ��Ч,ֻҪ�ǵ͵�ƽ����Ч,�����ø�

	rt_pin_mode(GET_PIN(G,2),PIN_MODE_OUTPUT);//		
	rt_pin_write(GET_PIN(G,2),PIN_HIGH);//SET,�͵�ƽ��Ч,ûɶ��Ӧ��֪����ɶ�õ�		
}
void hplc_PWR_OFF(void)	
{		
	rt_pin_write(GET_PIN(G,4),PIN_LOW);
	rt_pin_write(GET_PIN(G,3),PIN_LOW);//RESET,�͵�ƽ��Ч			
	rt_pin_write(GET_PIN(G,2),PIN_HIGH);//SET,�͵�ƽ��Ч		
}//ģ�����

int hplc_inition(struct CharPointDataManage *  data_wait_list,struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx){

	struct serial_configure hplc_config = RT_SERIAL_CONFIG_DEFAULT; /* ���ò��� */
	
	hplc_serial = rt_device_find(HPLC_UART_NAME);//������Ӧ�Ĵ���
 
	if(hplc_serial!=RT_NULL){
		hplc_config.baud_rate = BAUD_RATE_9600;
		hplc_config.data_bits = DATA_BITS_9;
		hplc_config.stop_bits = STOP_BITS_1;
		hplc_config.parity = PARITY_EVEN  ;
		hplc_config.bufsz=1124;
		rt_device_control(hplc_serial, RT_DEVICE_CTRL_CONFIG, &hplc_config);//���ô��ڲ���
		if(rt_device_open(hplc_serial, RT_DEVICE_FLAG_DMA_RX)==0){//��dam����ģʽ�򿪴����豸

				rt_kprintf("[hplc]  (%s)   SET SERIAL AS 9600 E 8 1 \n",__func__);//
		}	
	}else{
		rt_kprintf("[hplc]  (%s)   rt_device_find is error \n",__func__);//	
	}	
		
	if(1){
		hplc_PWR_ON();
	}else{
		hplc_PWR_OFF();			
	}
	//���巢�����ݵĳ�ʼ��	
	init_CharPointDataManage(data_rev);	
	init_CharPointDataManage(data_tx);
	init_CharPointDataManage(data_wait_list);
	init_698_FRAME(&_698_ChgPlanIssue);	
	_698_ChgPlanIssue.usrData=_698_ChgPlanIssue_data;
	_698_ChgPlanIssue.usrData_size=1024;

	

	
	init_698_FRAME(&_698_ChgPlanIssueGet);
	_698_ChgPlanIssueGet.usrData=_698_ChgPlanIssueGet_data;
	
	
	
	_698_ChgPlanIssueGet.usrData_size=100;
	
	init_698_FRAME(&_698_ChgPlanAdjust);
	_698_ChgPlanAdjust.usrData=_698_ChgPlanAdjust_data;
	_698_ChgPlanAdjust.usrData_size=1024;
	
	init_698_FRAME(&_698_StartChg);
	_698_StartChg.usrData=_698_StartChg_data;
	_698_StartChg.usrData_size=50;

	init_698_FRAME(&_698_StopChg);
	_698_StopChg.usrData=_698_StopChg_data;
	_698_StopChg.usrData_size=50;	
	return 0;	
}


int hplc_645_addr_receive(struct CharPointDataManage *data_rev){
//68 AA AA AA AA AA AA 68 13 00 DF 16 //ʵ���յ���18��,��Ϊ�㲥����AA

	if(data_rev->dataSize>=12){
		if(data_rev->priveData[0]==0x68&&(data_rev->priveData[7]==0x68)
			&&(data_rev->priveData[11]==0x16)){
			rt_kprintf("\n[hplc]  (%s)   good struct\n",__func__);//		
		}else{
			return -1;
		}
		if(data_rev->priveData[1]==0xAA&&(data_rev->priveData[8]==0x13)
			&&(data_rev->priveData[10]==0xDF)){
			rt_kprintf("\n[hplc]  (%s)   good orde check\n",__func__);//		
		}else{
			return -1;
		}	
		return 1;
	
	}
	return 0;		


}

/*
��������
����������
����ֵ��

�������ã�hplc�ӿڲ�������Ҫ����֡У��,���ṹ�帳��static struct  _698_FRAME  _698_frame_rev
					������Ӧ����,

*/
//extern int priveData_analysis(unsigned char * data,struct  _698_FRAME  *_698_frame_rev,struct  _698_FRAME  *_698_frame_send,int size);
//int hplc_priveData_analysis(unsigned char * data,struct  _698_FRAME  *_698_frame_rev,struct  _698_FRAME  *_698_frame_send,int size,struct CharPointDataManage * data_tx){
int hplc_priveData_analysis(struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx){	
	//_698_frame_rev->strategy.cmd_type=0;
	//�յ���֡����¼�յ���ʱ��
	//rt_kprintf("[hplc]  (%s)   hplc_priveData_analysis data_tx.size= %d \n",__func__,data_tx->size);
	get_current_time(data_rev->_698_frame.rev_tx_frame_date_time.data);
	return priveData_analysis(data_rev,data_tx);//������������
	
	
}

/*
�����������Ҫ33λ�Ŀ��м��
*/
int hplc_package(unsigned char * data,int size){
	return 0;
}


/*

˵����������,������������,Ĭ������1024
*/

int clear_data(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct  _698_FRAME  *_698_frame){

	//rt_kprintf("[hplc]  (%s)    \n",__func__);
	priv_698_state->USART_RX_STA=0;                       //�յ���һ֡�е�һ��08�ı�־λ
//	times=0;                                        //�жϳ�ʱ�� ,��ʱ��ʱ������	
	data_rev->dataSize=0;//��������ж�0
	rt_memset(data_rev->priveData,0,data_rev->size);
	priv_698_state->len_left=0;
	priv_698_state->len_sa=0;
	priv_698_state->len_all=0;
	priv_698_state->FE_no=0;
	//if((data_rev->size!=0)&&(data_rev->priveData!=RT_NULL)){
	//	rt_memset(data_rev->priveData,0,data_rev->size);		
	//}

	/*if(array_deflate(data_rev->priveData, data_rev->size,0)==0){
		data_rev->size=1024;
		rt_memset(data_rev->priveData,0,data_rev->size);//���		
	}else{
		rt_memset(data_rev->priveData,0,data_rev->size);
		return -1;
	}*/	
	return 0;	
}	


/*
//int save_char_point_data
˵����������,������������,Ĭ������1024
*/

int save_hplc_data(struct CharPointDataManage *hplc_data,int position,unsigned char Res){
	save_char_point_data(hplc_data,position,&Res,1);
	return 0;		
}





int printmy(struct  _698_FRAME  *_698_frame){
	int i;
//	unsigned char * p;
	rt_kprintf("[hplc]  (%s)  _698_frame->head=    %0x\n",__func__,_698_frame->head);	
	rt_kprintf("[hplc]  (%s)  _698_frame->length0= %0x\n",__func__,_698_frame->length0);
	rt_kprintf("[hplc] (%s)  _698_frame->length1= %0x\n",__func__,_698_frame->length1);
	rt_kprintf("[hplc] (%s)  _698_frame->control= %0x\n",__func__,_698_frame->control);		
	rt_kprintf("[hplc] (%s)  _698_frame->addr.sa=  %0x\n",__func__,_698_frame->addr.sa);	
	//rt_kprintf("[hplc]  (%s)  _698_frame->addr.s_addr_len= %0x\n",__func__,_698_frame->addr.s_addr_len);	
	
	for(i=0;i<_698_frame->addr.s_addr_len;i++){
	 rt_kprintf("[hplc] (%s)  _698_frame->addr->s_addr[%d]= %0x\n",__func__,i,_698_frame->addr.s_addr[i]);	
	}	
	rt_kprintf("[hplc] (%s)  _698_frame->addr.ca=%0x \n",__func__,_698_frame->addr.ca);
	rt_kprintf("[hplc] (%s)  _698_frame->HCS0= %0x\n",__func__,_698_frame->HCS0);
	rt_kprintf("[hplc] (%s)  _698_frame->HCS1= %0x\n",__func__,_698_frame->HCS1);
	
//	rt_kprintf("[hplc]  (%s)  _698_frame->usrData_len= %d\n",__func__,_698_frame->usrData_len);	
	for(i=0;i<_698_frame->usrData_len;i++){
//		rt_kprintf("[hplc] (%s)  __698_frame->usrData[%d]= %0x\n",__func__,i,_698_frame->usrData[i]);
		rt_kprintf("%02x ",_698_frame->usrData[i]);	
		
	}
	rt_kprintf("\n");	
	
	
	rt_kprintf("[hplc]  (%s)  _698_frame->FCS0= %0x\n",__func__,_698_frame->FCS0);
	rt_kprintf("[hplc]  (%s)  _698_frame->FCS1= %0x\n",__func__,_698_frame->FCS1);
	rt_kprintf("[hplc]  (%s)  _698_frame->end=  %0x\n",__func__,_698_frame->end);

//	for(i=0;i<data_tx->dataSize;i++){
//		rt_kprintf("%02x ",data_tx->priveData[i]);	
//	}
//	rt_kprintf("\n");	
	
	
	
	
	
	

/*
		for(i=0;i<;i++){
		rt_kprintf("[hplc]  (%s)  i %d  data[i]= %0x\n",i,test_hplc[i]);
	}
	
	
	unsigned char head;      //��ʼ֡ͷ = 0x68
	unsigned short length;   //����,�����ֽ�,��bit0-bit13��Ч,�Ǵ���֡�г���ʼ�ַ��ͽ����ַ�֮���֡�ֽ������y
	unsigned char control;   //������c,bit7,���䷽��λ
	struct _698_ADDR addr;//��ַ��a
		unsigned char sa;   //��bit0-bit3����,��Ӧ1��16,������Ҫ��һ     
		unsigned char *s_addr;//������sa����
		unsigned char ca;
		int s_addr_len;
	
	
	unsigned char HCS0;//֡ͷУ��,�Ƕ�֡ͷ���ֳ���ʼ�ַ���HCS����֮��������ֽڵ�У��
	unsigned char HCS1;
	unsigned char *usrData;//��·�û�����
	int usrData_len;
	unsigned short FCS0;//֡У��
	unsigned short FCS1;
	unsigned char end;//�����ַ� = 0x16
	
	unsigned char lock1;//����־��ʹ�ô����У������lock,��ס�豸�����᲻��Ӱ�������豸ʹ��
	unsigned char lock2;//ʹ�������˫���������ٽ���Դ
	unsigned char cmd_type;//���������Ǵ����ﵽ�������������ӿڵģ��͵���
*/
	return 0;
}









/*
��������
����������
����ֵ��

�������ã��������õ�ͳһ�ӿ�

*/
int priveData_analysis(struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx){

//������	
	if(!test_dis_check){
		if(_698_FCS(data_rev->priveData, 1,data_rev->dataSize-2,0)!=0	){//����ʱ��Ҫ�ȼ��� ,У��ɹ�������һ��
			rt_kprintf("[hplc]  (%s)    _698_FCS if failed \n",__func__);	
			return -1;		
		} 
	}		
	if( _698_unPackage(data_rev->priveData,&data_rev->_698_frame,data_rev->dataSize)!=0){//��ֱ�Ӹ�ֵ������ǿ������ת��������ֵ��Ϳ���ֱ�Ӳ���_698_frame_rev	
		rt_kprintf("[hplc]  (%s)   _698_unPackage(data,_698_frame_rev,size)!=0\n",__func__);
		return -1;
	}
	

	return 0;	
}

/*
���ܣ��Է�����д���
������size,������֮֡���֡���ȡ�
�ܾ�һЩ����Ҫ�ķ���

��֡Ӧ����Ҫ����������
	//1:����������    �����Ƿ�֡      ��Ҫ������Ӧ��ÿ����֡������֡     �ǲ�����Ҫ��ʱ����
	//2:����������    ���͵��Ƿ�֡    ������һ֡����Ҫ�ȴ�Ӧ��          iterate_wait_response_list����
	//3:�ͻ��������  ���͵��Ƿ�֡    ��Ҫ�ȴ�Ӧ���յ�Ӧ������һ֡�� iterate_wait_response_list����
	//4:�ͻ��������  �����Ƿ�֡      ��Ҫ������Ӧ��ÿ����֡������֡					

//�����������֡��
������ɷ������������ͨ֡���ظ����� return 0


������ɷ���������ķ���   ���ǲ��ǵ�һ֡�������һ֡������ǵ�һ֡��ֱ�ӱ��棨�ɹ��󷵻�Ӧ�� return 0����
									        ������ǵ�һ֡��ȥ�鿴��û���Ѿ������֡��û�з��� -1����ʱ��������ô�죩���б������ݣ��������ȣ�����Ӧ�� return 0


��������û��������ͨ֡    �жϴ���һ�£����ûظ� return 1

��������û������        ������ʱ���루���͵�һ֡�ʹ浽�������յ�Ӧ��󣬷���һ�루���ȶ��Ǽ���İ�����һ֡��֡�ó������֡���ȣ�  
											   ���յ����Ƿ�֡�ǣ��� �ɷ���������ķ��� һ������
												 
												 
�յ���֡�ͷ�֡��Ӧ��												 

*/


int iterate_wait_response_list(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_list){
/*
	new2019-07-01
	
	ֻ�����֡�ģ���֡�Ļ��ߴ�����˵ģ���ֱ��������
	
*/
	int i;
	struct CharPointDataManage *above,*node,*want=RT_NULL;
//	unsigned char *want_affair;
	unsigned char want_affair;
	above=hplc_data_list;//ͷ
	node=hplc_data_list;

	
	if(0x81<=data_rev->_698_frame.usrData[0]<=0x89){
		want_affair=data_rev->_698_frame.usrData[0]-0x80;//��ԭ�����͵�֡		
	}else if(data_rev->_698_frame.usrData[0]==security_response){
		want_affair=security_request;				
	}else{
		rt_kprintf("[hplc]  (%s)  no this kind of affair or not support! \n",__func__);
		return -3;				
	}

	if((hplc_data_list->list_no==0)||(node->_698_frame.usrData==RT_NULL)){//���������һ����,��ָ����������������������пյ�
		rt_kprintf("[hplc]  (%s)   there is no hplc_data_list \n",__func__);
		return -3;
	}
	
	if(!(data_rev->_698_frame.control&CON_MORE_FRAME)){//�������֡���ͼ򵥵�ɾȥ��һ֡(����֡��������)���ú������ߣ�������Ĳ���������		
		for(i=0;i<hplc_data_list->list_no;i++){//��������
			if(want_affair==node->_698_frame.usrData[0]&&(!node->_698_frame.control&CON_START_MASK)){//�ж����ͺ�oad�����ǲ���Ӧ��֡������Ǿͷ�����һ֡���Ҳ�ɾ����ֱ�������ꡣ
				//�ж��ǲ�����Ҫ��֡�����б��еıȽϾɵ��ҵ����˳����ظ�����Ҫ��ʱ�ĳ���ȥ��������ͬ����Ҫÿ������ȥɾ�����ӵ������У���			
				want=node;
				break;
			}else{
				above=node;//������һ��			
			}
			if(node->next==RT_NULL){//������ǿվ��˳�	,��Ϊ�������
				rt_kprintf("[hplc]  (%s)   node->next==RT_NULL \n",__func__);
				break;			
			}else{
				node=node->next;
			}
		}
		
		if((hplc_data_list->list_no==1)&&(want!=RT_NULL)){//���ֻ��һ���������������Ҫ��
			//head->list_no-=1;//�����ﾳ����
			init_CharPointDataManage(want);			
			return 0;
		}
		
		if(want!=RT_NULL&&(want->next==RT_NULL)){//����Ҫ�ģ����ǵ�һ�����������һ��
			above->next=RT_NULL;//ǰһ��ָ��		
			hplc_data_list->list_no-=1;
			free_CharPointDataManage(want);
			return 0;
		
		}
		
		if(want!=RT_NULL){//����Ҫ�ģ����м��
			above->next=want->next;//ָ����һ��
			hplc_data_list->list_no-=1;
			free_CharPointDataManage(want);
			return 0;	
		}
		return -3;//û���ҵ����ʵ�֡��
	}else{	//����Ƿ�֡�ģ���Ҫ�����ݼӳ����������֡������֮�󣬰����ݸ�data_rev,������
		//if(finish){
		//���ݸ�data_rev
		//������ȥ��������,
		//return 0;
		//}else if ��û�꣩{
		//����Ӧ��֡ 
		//return -3;//Ӧ��֡���Ǵ����ˣ�����Ĳ������ˣ�
		//}	
	}
	return 0;
}	
int iterate_wait_request_list(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_list){
/*
	new2019-07-01
	ֻ�����֡�ģ���Ϊ����ֱ֡�Ӿʹ�����
	ֻ�����ͻ��������
	
*/
	int i;
	struct CharPointDataManage *above,*node,*want=RT_NULL;
	above=hplc_data_list;//ͷ
	node=hplc_data_list;
//  unsigned char * want_affair;
	unsigned char want_affair;
	want_affair=data_rev->_698_frame.usrData[0];	
	if((hplc_data_list->list_no==0)||(node->_698_frame.usrData==RT_NULL)){//���������һ����,��ָ����������������������пյ�
		rt_kprintf("[hplc]  (%s)   there is no hplc_data_list \n",__func__);
		return -3;
	}	
	if((data_rev->_698_frame.control&CON_MORE_FRAME)){//�����֡��* want_affair��hplc_data_list->_698_frame.usrData[0]�Ƚ�		
		for(i=0;i<hplc_data_list->list_no;i++){//��������
			if(want_affair==node->_698_frame.usrData[0]&&(node->_698_frame.control&CON_START_MASK)){//�ж����ͺ�oad�����ǲ���Ӧ��֡������Ǿͷ�����һ֡���Ҳ�ɾ����ֱ�������ꡣ
				//�ж��ǲ�����Ҫ��֡�����б��еıȽϾɵ��ҵ����˳����ظ�����Ҫ��ʱ�ĳ���ȥ��������ͬ����Ҫÿ������ȥɾ�����ӵ������У���			
				want=node;
				break;
			}else{
				above=node;//������һ��			
			}
			if(node->next==RT_NULL){//������ǿվ��˳�	,��Ϊ�������
				rt_kprintf("[hplc]  (%s)   node->next==RT_NULL \n",__func__);
				break;			
			}else{
				node=node->next;
			}
		}		
		if((hplc_data_list->list_no==1)&&(want!=RT_NULL)){//���ֻ��һ���������������Ҫ�ģ��ж�������ô�������ˣ���ʼ�� ����0��û���꣬���ط�֡Ӧ��֡������ 1���յ�Ӧ���֡��������һ�룬������2����ͬ
			init_CharPointDataManage(want);			
			return 0;
		}
		
		if(want!=RT_NULL&&(want->next==RT_NULL)){//����Ҫ�ģ����ǵ�һ�����������һ�����ж�������ô�������ˣ���ʼ�� ����0��û���꣬���ط�֡Ӧ��֡data_tx������ 1
			above->next=RT_NULL;//ǰһ��ָ��		
			hplc_data_list->list_no-=1;
			free_CharPointDataManage(want);
			return 0;				
		}
		
		if(want!=RT_NULL){//����Ҫ�ģ����м�ģ��ж�������ô�������ˣ���ʼ�� ����0��û���꣬���ط�֡Ӧ��֡������ 1
			above->next=want->next;//ָ����һ��
			hplc_data_list->list_no-=1;
			free_CharPointDataManage(want);
			return 0;		
		}
		//û���ҵ����ʵ�֡���ͽ���һ֡���뵽��֡�С�		
		return 1;		
	}else{	//����ǲ���֡�ģ�������ֻ�Ǽ�һ���ж�
			
	}
	return 0;
}	

/*

���ܣ����ش����֡
result==1������Ҫ����
*/


int _698_analysis(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_wait_list){
//	unsigned char *p,current_meter_serial=0;
//	unsigned char want_affair;
	int result=1;//�����ͷ���1
// current_meter_serial=get_meter_serial();set_meter_serial();
//���ж��Ǵӿͻ��������������Ǵӷ��������ͻ��������ǶԿ������ͶԵ��ʱ�ǲ�ͬ�ģ�����Ҫ���������
		
	switch ( data_rev->_698_frame.control&CON_DIR_START_MASK){//�ȴ�ӡΪ�˵��ԣ����ڿ��ܻ��õ�
		case(CON_UTS_S)://�ɷ���������ģ���Ȼ�ǻ�Ӧ������Ӧ���͵ķ�֡����������Ӧ�𣬵��Ƿ�֡���͵ģ���Ӧ�𣩣����Ϻ�Ϳ��Դ����ˣ�����֡Ӧ�����Ϻ���һ�����̣������ߣ�
//			rt_kprintf("[hplc]  (%s)  UTS_S \n",__func__);
						
			//���want_affair�漰��oad���ҵ�������oad�������õ��������ʱ��ʵ�֣��Ͼ��������������͵����ޣ����ң����Ӻ���Ҳ����ʵ����
			//ֻ�����֡	���ͷ����Ӧ��ֻ�з�֡����Ž�ȥ���������ú���Ĳ���������	
			result=iterate_wait_response_list(priv_698_state,data_tx,data_rev,hplc_data_wait_list);	//�����֡�ɹ��ˣ��͸�ֵ��data_rev��Ȼ����	
			//�ҵ��Ǹ�֡����ɾ��,��������Ƿ�֡��ƴ֡��������Ӧ��
			if(result!=0){//Ŀǰ������������յ��Ĳ�����Ҫ��֡������֡�Ƿ�֡���һ�û�з�������֡����Ӧ��֡��Ӧ��֡�Ƿ�֡�����һ�û�����������
				return result;				
			}//������������������_698_del_affairs����
			break;
		case(CON_UTS_U):
			//�ɿ���������,���ͻ������������Ƿ�֡�����ݣ���hplc_data_wait_list�������ط�֡Ӧ��������˸��ƻ�data_rev��ɾ��list����Ӧ�𣬷���0��
			//��֡��Ӧ��Ҳ��iterate_wait_request_list��������һ��֡����
//			rt_kprintf("[hplc]  (%s)  UTS_U \n",__func__);
			if((data_rev->_698_frame.control&CON_MORE_FRAME)){
				
				rt_kprintf("[hplc]  (%s)   CON_MORE_FRAME  so go on ! \n",__func__);				
				result=iterate_wait_request_list(priv_698_state,data_tx,data_rev,hplc_data_wait_list);
				//��֡��Ӧ��Ҳ�����ﴦ������һ��֡				
				if(result!=0){//Ŀǰ������������յ�����û�е�֡���Ҳ��ǵ�һ֡��Ӧ��֡�Ƿ�֡���Ҳ������һ֡������֡�Ƿ�֡�����һ�û�����������
					return result;				
				}//������������������_698_del_affairs����		
				
			}else{//������Ĳ��Ƿ�֡��ʲôҲ�����������洦������
//				rt_kprintf("[hplc]  (%s) UTS_U is not  CON_MORE_FRAME   ! \n",__func__);							
			}			
			break;				
		default:
			rt_kprintf("[hplc]  (%s)   not real UTS \n",__func__);		
			break;
	}
	result=rev_698_del_affairs(priv_698_state,data_tx,data_rev);//ʵ�ʴ����û����Ե��������������յ�	
	return result;
}



/*
*
*
*/
int copy_698_FactoryVersion(struct _698_FactoryVersion *des,struct _698_FactoryVersion *src){
	my_strcpy(des->manufacturer_code,src->manufacturer_code,0,4);

	my_strcpy(des->soft_version,src->soft_version,0,4);
	my_strcpy(des->soft_date,src->soft_date,0,6);
	my_strcpy(des->hard_version,src->hard_version,0,4);
	my_strcpy(des->hard_date,src->hard_date,0,6);
	my_strcpy(des->manufacturer_ex_info,src->manufacturer_ex_info,0,8);	
	return 0;
}
int unPackage_698_connect_request(struct _698_STATE  * priv_698_state,struct  _698_FRAME  * _698_frame,struct _698_connect_response * prive_struct){

//��Ϣ�ɿͻ�������	
	rt_kprintf("[hplc]  (%s)    \n",__func__);
	prive_struct->type=link_response;
	prive_struct->piid_acd=_698_frame->usrData[1];
	copy_698_FactoryVersion(&prive_struct->connect_res_fv,&priv_698_state->FactoryVersion);
	my_strcpy(prive_struct->apply_version,_698_frame->usrData,2,2);
	my_strcpy(priv_698_state->version,_698_frame->usrData,2,2);
	
	my_strcpy(prive_struct->connect_res_pro.protocolconformance,_698_frame->usrData,4,8);
	my_strcpy(priv_698_state->protocolconformance,prive_struct->connect_res_pro.protocolconformance,0,8);
		
	my_strcpy(prive_struct->connect_res_func.functionconformance,_698_frame->usrData,12,16);
	my_strcpy(priv_698_state->functionconformance,prive_struct->connect_res_func.functionconformance,0,16);
	
	//������Ҳû�£�ÿ����Ӧ���յ��Ǳߵ�ֵȻ��Ӧ
	my_strcpy(prive_struct->max_size_send,_698_frame->usrData,28,2);	
	my_strcpy(prive_struct->max_size_rev,_698_frame->usrData,30,2);
	my_strcpy(&prive_struct->max_size_rev_windown,_698_frame->usrData,32,1);
	my_strcpy(prive_struct->max_size_handle,_698_frame->usrData,33,2);
	my_strcpy(prive_struct->connect_overtime,_698_frame->usrData,35,4);
	prive_struct->connect_res_info.connectresult=0x00;//������Ӧ������
	prive_struct->connect_res_info.connectresponseinfo_sd.type=_698_frame->usrData[39];//��֤������Ϣ=	��֤�������
	if(prive_struct->connect_res_info.connectresponseinfo_sd.type!=NullS){
		prive_struct->connect_res_info.connectresponseinfo_sd.type=0;//�����ʱ������
			rt_kprintf("[hplc]  (%s)  not control  type!=NullS \n",__func__);
	}
	prive_struct->FollowReport=0;//OPTIONAL=0��ʾû���ϱ���Ϣ
	if(prive_struct->FollowReport!=0){
					rt_kprintf("[hplc]  (%s)  not control  FollowReport!=0 \n",__func__);
	}else{
	
	}	
	if(_698_frame->usrData[39]==0){
		prive_struct->time_tag=_698_frame->usrData[39];	
	}else{//������
			rt_kprintf("[hplc]  (%s)  not control  time_tag!=0 \n",__func__);		
	}		
	return 0;
}
/*
��������
����������
����ֵ��

�������ã�
���ṹ�帳��static struct _698_link_request hplc_698_link_request;
					������Ӧ����,


*/
int unPackage_698_link_request(struct  _698_FRAME  *_698_frame,struct _698_link_request * request,int * size){
	request->type=_698_frame->usrData[0];
	request->piid_acd=_698_frame->usrData[1];
	request->work_type=_698_frame->usrData[2];
	_698_frame->strategy.heart_beat_time0=request->heartbeat_time[0]=_698_frame->usrData[3];
	_698_frame->strategy.heart_beat_time1=request->heartbeat_time[1]=_698_frame->usrData[4];
	request->date_time.year[0]=_698_frame->usrData[5];
	request->date_time.year[1]=_698_frame->usrData[6];
	request->date_time.month=_698_frame->usrData[7];
	request->date_time.day=_698_frame->usrData[8];
	request->date_time.week=_698_frame->usrData[9];
	request->date_time.hour=_698_frame->usrData[10];
	request->date_time.minute=_698_frame->usrData[11];
	request->date_time.second=_698_frame->usrData[12];
	request->date_time.millisconds[0]=_698_frame->usrData[13];
	request->date_time.millisconds[1]=_698_frame->usrData[14];
	
	request->position=15;
	my_strcpy(request->date_time.data,_698_frame->usrData,5,10);	
	return 0;	
}

/*
���������ýṹ�帳ֵ��* des=* source

*/

int copy_698_frame(struct  _698_FRAME * des,struct  _698_FRAME  * source){
	
	des->head=source->head;//��ʼ֡ͷ = 0x68	
	
	des->length0=source->length0;//hplc_data->size<1024ʱ
	des->length1=source->length1;		
	des->control=source->control;   //������c,bit7,���䷽��λ
	des->addr.sa=source->addr.sa;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����

	//����������ַ
	des->addr.s_addr_len=source->addr.s_addr_len;
	my_strcpy(des->addr.s_addr,source->addr.s_addr,0,source->addr.s_addr_len);//��������

	des->addr.ca=source->addr.ca;

//У��ͷ
	des->HCS0=source->HCS0;	
	des->HCS1=source->HCS1;

	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	des->usrData_len=source->usrData_len;//�û����ݳ��ȹ���

	//���ƻ�������̫��
	
	save_char_point_usrdata(des->usrData,&des->usrData_size,source->usrData,0,source->usrData_len);	
	
	//_698_frame_rev->usrData=data+(8+_698_frame_rev->addr.s_addr_len);

	des->FCS0=source->FCS0;
	des->FCS1=source->FCS1;
		
	des->end=source->end;/**/
	return 0;
}
/*

*/
int copy_char_point_data(struct CharPointDataManage * des,struct CharPointDataManage * source){

	des->_698_frame.head=source->_698_frame.head;//��ʼ֡ͷ = 0x68		
	des->_698_frame.length0=source->_698_frame.length0;//hplc_data->size<1024ʱ
	des->_698_frame.length1=source->_698_frame.length1;		
	des->_698_frame.control=source->_698_frame.control;   //������c,bit7,���䷽��λ
	des->_698_frame.addr.sa=source->_698_frame.addr.sa;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����

	//����������ַ
	des->_698_frame.addr.s_addr_len=source->_698_frame.addr.s_addr_len;
	//if(des->addr.s_addr!=RT_NULL){//���ͷ�
		
	//	rt_free(des->addr.s_addr);//���жϴ��󣬶����Ե��ڴ�
		
	//}
	//des->addr.s_addr=(unsigned char *)rt_malloc(sizeof(unsigned char)*(source->addr.s_addr_len));//����ռ�		
	my_strcpy(des->_698_frame.addr.s_addr,source->_698_frame.addr.s_addr,0,source->_698_frame.addr.s_addr_len);//��������

	des->_698_frame.addr.ca=source->_698_frame.addr.ca;
//У��ͷ
	des->_698_frame.HCS0=source->_698_frame.HCS0;	
	des->_698_frame.HCS1=source->_698_frame.HCS1;
	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	des->_698_frame.usrData_len=source->_698_frame.usrData_len;//�û����ݳ��ȹ���
	                             	
//	if(des->usrData!=RT_NULL){//���ͷ�
//		rt_kprintf("[hplc]  (%s)   des->usrData!=RT_NULL\n",__func__);
	//	rt_free(des->usrData);
//	}else{
//		rt_kprintf("[hplc]  (%s)   des->usrData==RT_NULL\n",__func__);	
	
//	}
	//des->usrData=(unsigned char *)rt_malloc(source->usrData_len);//���û�����ռ�	
	//des->usrData_size=source->usrData_len;//�ռ��С	
	des->_698_frame.usrData=des->priveData+(8+des->_698_frame.usrData_len);
	save_char_point_usrdata(des->_698_frame.usrData,&des->_698_frame.usrData_size,source->_698_frame.usrData,0,source->_698_frame.usrData_len);		
	des->_698_frame.FCS0=source->_698_frame.FCS0;
	des->_698_frame.FCS1=source->_698_frame.FCS1;
		
	des->_698_frame.end=source->_698_frame.end;/**/
	return 0;
}

/*

*/

int copy_to_work_wait_list(struct CharPointDataManage *hplc_data,struct CharPointDataManage * hplc_data_wait_list){
	int i;
	struct CharPointDataManage * new_struct,*head,*node,*end;
	head=hplc_data_wait_list;//ͷ	
	if(hplc_data_wait_list->list_no!=0){//�����һ��û��
		new_struct=(struct CharPointDataManage *)rt_malloc(sizeof(struct CharPointDataManage));//�����¿ռ�
		if(new_struct==RT_NULL){//���ͷ�
			rt_kprintf("[hplc]  (%s)  new_struct==RT_NULL \n",__func__);
		}else{	
			init_CharPointDataManage(new_struct);//ָ�븳��ֵ
		}
		//������һ��
		node=hplc_data_wait_list;//ָ���һ��
		
		for(i=0;i<hplc_data_wait_list->list_no;i++){//���������һ��
			if(node->next==RT_NULL){//����Ҳ��������Ϊ�ҵ������һ��������������������󣬿�ָ�����̫���أ������˳�				
				if(hplc_data_wait_list->list_no!=i+1){//û�е����һ�����Ǹ���ָ�룬
					rt_kprintf("[hplc]  (%s)   opy_to_work_wait_list_no is not right\n",__func__);
					hplc_data_wait_list->list_no=i+1;//����������ԣ���������������������Ĳ�Ҫ��,�����һ��Ҳ�Ǹ��յġ���������������ɾȥ				
				}
				break;			
			}else{//�ж��ǲ����ظ����͵�֡������Ǿ͸��ǣ���Ҫ����ʱ��,Ҳ�������ط���ʱ���ظ�֡�Ĺ�����ʵ�֡�
				//return 0;			
			}
			node=node->next;//
		}		
		node->next=new_struct;
		end=new_struct;//endָ�����һ��				
	}else{//�����һ����û�и�ֵ���ͽ�new_structָ���һ����
		rt_kprintf("[hplc]  (%s)   new_struct=hplc_data_wait_list\n",__func__);
		end=hplc_data_wait_list;//��һ��Ҳ�����һ��
	}
	end->next=RT_NULL;//ָ���һ��	
	my_strcpy(end->priveData,hplc_data->priveData,0,hplc_data->dataSize);//��������	
	copy_char_point_data(end,hplc_data);
	head->list_no+=1;//�����ﾳ����
	return 0;
}

/*

�������ã���¼֡������ؿ��õ�data_tx

������size,������֮֡���֡���ȡ�
*/


int Report_Cmd_ChgPlanExeState(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state){
	int result=1;
	unsigned char temp_char;
	//�ṹ�帳ֵ����ͬ����

	hplc_data->dataSize=0;	
	temp_char=hplc_data->_698_frame.head = 0x68;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	int len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵĳ���	
	
	temp_char=hplc_data->_698_frame.control=CON_STU_S|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa ;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//������������ַ
	hplc_data->_698_frame.addr=priv_698_state->addr;
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);

	temp_char=hplc_data->_698_frame.addr.ca=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ	
	
	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);	                              
	
	
	temp_char=report_notification;//�ϱ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=ReportNotificationList;//�ϱ�����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x09;//�Լ�����PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	

	/**�û����ݵĽṹ�岿�֣��ο���ȡһ����¼�Ͷ�������**/

	//SEQUENCE OF A-ResultNormal
	temp_char=0x01;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	//�������������� OAD		
	temp_char=0x34;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x04;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x06;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//Get-Result
	
	temp_char=0x01;//���� [1] Data
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	//	//	
	charge_exe_state_package(&Report_charge_exe_state,hplc_data);
	

	temp_char=0x00;// û��ʱ���ǩ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		


	hplc_data->_698_frame.usrData_len=hplc_data->dataSize-HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
	//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

	
	
	
	int FCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��
		
	temp_char=hplc_data->_698_frame.end=0x16;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	if(hplc_data->dataSize>HPLC_DATA_MAX_SIZE){
			rt_kprintf("[hplc]  (%s)  >HPLC_DATA_MAX_SIZE too long   \n",__func__);
			return -1;	
	}
	
	hplc_data->priveData[len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

	hplc_data->priveData[len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

//У��ͷ
	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(hplc_data->priveData, HCS_position);
	hplc_data->_698_frame.HCS0=hplc_data->priveData[HCS_position];	
	hplc_data->_698_frame.HCS1=hplc_data->priveData[HCS_position+1];

	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(hplc_data->priveData, FCS_position);
	
	hplc_data->_698_frame.FCS0=hplc_data->priveData[FCS_position];
	hplc_data->_698_frame.FCS1=hplc_data->priveData[FCS_position+1];		


  //���账���



	return result;//������
}//_698_frame_rev->��

/*

�������ã���¼֡������ؿ��õ�data_tx

������size,������֮֡���֡���ȡ�
*/

int link_request_package(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state){
	int result=1;
	
	struct _698_link_request user_date_struct;
	struct _698_date_time current_time;
	short past_time;
	unsigned char temp_char;
	//�ṹ�帳ֵ����ͬ����

	get_current_time(current_time.data);
	past_time=(current_time.data[5]-priv_698_state->last_link_requset_time.data[5])*60*60  
						+(current_time.data[6]-priv_698_state->last_link_requset_time.data[6])*60
						+(current_time.data[7]-priv_698_state->last_link_requset_time.data[7]);
	
	if(past_time >= (priv_698_state->heart_beat_time0*256+priv_698_state->heart_beat_time1-5)){//����ӽ�������ʱ֡����������֡,�����Сλ��˳���
		
		rt_kprintf("[hplc]  (%s)   past_time >= overtime sent link_request= %d   \n",__func__,priv_698_state->try_link_type);
		result=0;
	
	}else{
		rt_kprintf("[hplc]  (%s)   past_time <= overtime sent link_request= %d   \n",__func__,priv_698_state->try_link_type);
		return result;
	}


	
	hplc_data->dataSize=0;	
	temp_char=hplc_data->_698_frame.head = 0x68;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	int len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵĳ���	
	
	temp_char=hplc_data->_698_frame.control=CON_STU_S|CON_LINK_MANAGE;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa ;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//������������ַ
	hplc_data->_698_frame.addr.s_addr_len=priv_698_state->addr.s_addr_len;
//	if(hplc_data->_698_frame.addr.s_addr!=RT_NULL){//���ͷ�
//		rt_free(hplc_data->_698_frame.addr.s_addr);//���жϴ��󣬶����Ե��ڴ�
//	}
//	hplc_data->_698_frame.addr.s_addr=(unsigned char *)rt_malloc(sizeof(unsigned char)*(hplc_data->_698_frame.addr.s_addr_len));//����ռ�	
	


	my_strcpy(hplc_data->_698_frame.addr.s_addr,priv_698_state->addr.s_addr,0,hplc_data->_698_frame.addr.s_addr_len);//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);


	temp_char=hplc_data->_698_frame.addr.ca=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ	
	

	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=(hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len));	                              
	
//	if(hplc_data->_698_frame.usrData!=RT_NULL){//���ͷ�	,���ǿյľͲ�����
//		rt_kprintf("[hplc]  (%s)   hplc_data->_698_frame.usrData!=RT_NULL \n",__func__,__func__);
		//rt_free(hplc_data->_698_frame.usrData);

//	}
	//hplc_data->_698_frame.usrData=(unsigned char *)rt_malloc(sizeof(unsigned char)*(1024));//���û�����ռ�	
	//hplc_data->_698_frame.usrData_size=1024;//�ռ��С		
	
	temp_char=user_date_struct.type=link_request;//���û��ṹ�壬ֻ��Ϊ�˲���©���ͷ����ȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	

		
	
	if(priv_698_state->link_flag==0){//��û�е�¼�������¼֡
		temp_char=user_date_struct.piid_acd=0x00;//���ӵ�Ĭ�����ȼ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
		temp_char=user_date_struct.work_type=link_request;
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
	  priv_698_state->try_link_type=link_request_load;
		get_current_time(priv_698_state->last_link_requset_time.data);//������֡����ʼʱ�丳ֵ	
		
	}else if (priv_698_state->connect_flag==1){//�Ѿ���¼������ɹ����ж��ǲ�����Ҫ��������	
		temp_char=user_date_struct.piid_acd=0x01;//���ӵ�Ĭ�����ȼ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
		
		temp_char=user_date_struct.work_type=link_request_heart_beat;
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
		priv_698_state->try_link_type=link_request_heart_beat;	
		get_current_time(priv_698_state->last_link_requset_time.data);//������֡����ʼʱ�丳ֵ							
		
	}	else{
		return -1;
	}	
	temp_char=user_date_struct.heartbeat_time[0]=priv_698_state->heart_beat_time0;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=user_date_struct.heartbeat_time[1]=priv_698_state->heart_beat_time1;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	
	get_current_time(user_date_struct.date_time.data);//������֡����ʼʱ�丳ֵ		
	save_char_point_data(hplc_data,hplc_data->dataSize,user_date_struct.date_time.data,10);	
	
	hplc_data->_698_frame.usrData_len=(hplc_data->dataSize-HCS_position-2);//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
	//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

	
	
	
	int FCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��
		
	temp_char=hplc_data->_698_frame.end=0x16;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	hplc_data->priveData[len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

	hplc_data->priveData[len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

//У��ͷ
	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(hplc_data->priveData, HCS_position);
	hplc_data->_698_frame.HCS0=hplc_data->priveData[HCS_position];	
	hplc_data->_698_frame.HCS1=hplc_data->priveData[HCS_position+1];

	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(hplc_data->priveData, FCS_position);
	
	hplc_data->_698_frame.FCS0=hplc_data->priveData[FCS_position];
	hplc_data->_698_frame.FCS1=hplc_data->priveData[FCS_position+1];		


  //���账���



	return result;//������
}//_698_frame_rev->��

/*

�������ã����ؿ��õ�data_tx

������size,������֮֡���֡���ȡ�
*/

int connect_request_package(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state){
	int result;
	struct _698_connect_request user_date_struct;
	unsigned char temp_char;
	//�ṹ�帳ֵ����ͬ����
	rt_kprintf("[hplc]  (%s) \n",__func__);

	hplc_data->dataSize=0;	
	temp_char=hplc_data->_698_frame.head = 0x68;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�


	
	int len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵĳ���
	
	temp_char=hplc_data->_698_frame.control=CON_UTS_U|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

	
	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa ;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);


	//����������ַ
	hplc_data->_698_frame.addr.s_addr_len=priv_698_state->addr.s_addr_len;
	//if(hplc_data->_698_frame.addr.s_addr==RT_NULL){//���ͷ�
	//	rt_free(hplc_data->_698_frame.addr.s_addr);//���жϴ��󣬶����Ե��ڴ�
	//	hplc_data->_698_frame.addr.s_addr=(unsigned char *)rt_malloc(sizeof(unsigned char)*(hplc_data->_698_frame.addr.s_addr_len));//����ռ�	
	//}	
	my_strcpy(hplc_data->_698_frame.addr.s_addr,priv_698_state->addr.s_addr,0,hplc_data->_698_frame.addr.s_addr_len);//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);


	temp_char=hplc_data->_698_frame.addr.ca=priv_698_state->addr.ca;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ



	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);		                               
	
//	if(hplc_data->_698_frame.usrData==RT_NULL){//���ͷ�	,���ǿյľͲ�����
//		rt_kprintf("[hplc]  (%s)     hplc_data->_698_frame.usrData==RT_NULL \n",__func__);
		//rt_free(hplc_data->_698_frame.usrData);
	//	hplc_data->_698_frame.usrData=(unsigned char *)rt_malloc(sizeof(unsigned char)*(1024));//���û�����ռ�	
	//	hplc_data->_698_frame.usrData_size=1024;//�ռ��С	
//	}

	
	temp_char=user_date_struct.type=connect_request;//���û��ṹ�壬ֻ��Ϊ�˲���©���ͷ����ȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	
	temp_char=user_date_struct.piid=priv_698_state->piid;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//��ȡЭ��汾�ţ���hpcl��״̬�ṹ���У��ƺ����Զ����

	//my_strcpy(user_date_struct.version,priv_698_state->version,0,2);//10����������ʱ��
	save_char_point_data(hplc_data,hplc_data->dataSize,priv_698_state->version,2);//���������ô

	//my_strcpy(user_date_struct.connect_req_pro.protocolconformance,priv_698_state->protocolconformance,0,8);//
	save_char_point_data(hplc_data,hplc_data->dataSize,priv_698_state->protocolconformance,8);
	
	//my_strcpy(user_date_struct.connect_req_func.functionconformance,priv_698_state->functionconformance,0,16);//
	save_char_point_data(hplc_data,hplc_data->dataSize,priv_698_state->functionconformance,16);
	
	
	temp_char=user_date_struct.max_size_send[0]=0x04;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=user_date_struct.max_size_send[1]=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=user_date_struct.max_size_rev[0]=0x04;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=user_date_struct.max_size_rev[1]=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=user_date_struct.max_size_rev_windown=0x01;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			
	
	temp_char=user_date_struct.max_size_handle[0]=0x04;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=user_date_struct.max_size_handle[1]=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	//my_strcpy(user_date_struct.connect_overtime,priv_698_state->connect_overtime,0,4);//
	save_char_point_data(hplc_data,hplc_data->dataSize,priv_698_state->connect_overtime,4);
	
	temp_char=user_date_struct.connect_req_cmi.NullSecurity=NullS;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//	
	

	temp_char=0x00;//û��ʱ���ǩ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//	



	hplc_data->_698_frame.usrData_len=hplc_data->dataSize-HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
	//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		





	int FCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��

		
	temp_char=hplc_data->_698_frame.end=0x16;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô



//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	hplc_data->priveData[len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

	hplc_data->priveData[len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	


//У��ͷ
	//rt_kprintf("[hplc]  (%s)  calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(hplc_data->priveData, HCS_position);
	hplc_data->_698_frame.HCS0=hplc_data->priveData[HCS_position];	
	hplc_data->_698_frame.HCS1=hplc_data->priveData[HCS_position+1];


	//rt_kprintf("[hplc]  (%s)  calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(hplc_data->priveData, FCS_position);
	
	hplc_data->_698_frame.FCS0=hplc_data->priveData[FCS_position];
	hplc_data->_698_frame.FCS1=hplc_data->priveData[FCS_position+1];
	
	return result;
	

}//_698_frame_rev->��


int connect_response_del(struct  _698_FRAME  *_698_frame_rev ,struct _698_FRAME  * _698_frame_send,struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx){
	//struct _698_connect_response connect_response;
  //save_FactoryVersion(priv_698_state->FactoryVersion,_698_frame_rev->usrData,2)
  //���泧�Ұ汾��Ϣ 
	priv_698_state->connect_flag=1;//���ӳɹ�
	return 0 ;



}	
int omd_package(struct _698_omd *priv_struct,struct  _698_FRAME  *_698_frame_rev,int position){
	int result=1;//�����ͷ���1,������ͣ�result=0;

	rt_kprintf("[hplc]  (%s)   \n",__func__);
	priv_struct->oi[0]=_698_frame_rev->usrData[position];	
	priv_struct->oi[1]=_698_frame_rev->usrData[position+1];
	priv_struct->method_id=_698_frame_rev->usrData[position+2];	
	priv_struct->op_mode=_698_frame_rev->usrData[position+3];
	result=0;

	return result;	
	
}
int oad_package(struct _698_oad *priv_struct,struct  _698_FRAME  *_698_frame_rev,int position){
	int result=1;//�����ͷ���1,������ͣ�result=0;

	rt_kprintf("[hplc]  (%s)   \n",__func__);
	priv_struct->oi[0]=_698_frame_rev->usrData[position];	
	priv_struct->oi[1]=_698_frame_rev->usrData[position+1];
	priv_struct->attribute_id=_698_frame_rev->usrData[position+2];	
	priv_struct->attribute_index=_698_frame_rev->usrData[position+3];
	result=0;

	return result;	
	
}
int get_data_class(struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data,enum Data_T data_type){

	unsigned char temp_char;
	temp_char=0x01;//get_responseʱ�����ص���������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=data_type;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	return 0;

}

int get_date_time_s(struct _698_date_time_s *date_time_s){
	//�ӽӿڻ�ȡ����ȡ���ٸ�ֵ��date_time_s

//STR_SYSTEM_TIME_to__date_time_s(&priv_struct->StartTimestamp,&date_time_s);
		date_time_s->data[0]=20;//20��
		date_time_s->data[1]=(System_Time_STR.Year>>4)*10+System_Time_STR.Year&0x0f;//��	
		date_time_s->data[2]=(System_Time_STR.Month>>4)*10+System_Time_STR.Year&0x0f;//��
		date_time_s->data[3]=(System_Time_STR.Day>>4)*10+System_Time_STR.Day&0x0f;//��	
		date_time_s->hour=date_time_s->data[4]=(System_Time_STR.Hour>>4)*10+System_Time_STR.Hour&0x0f;//ʱ
		date_time_s->minute=date_time_s->data[5]=(System_Time_STR.Minute>>4)*10+System_Time_STR.Minute&0x0f;//��	
		date_time_s->second=date_time_s->data[6]=(System_Time_STR.Second>>4)*10+System_Time_STR.Second&0x0f;//��	


return 0;
}


int oi_parameter_get_time(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	struct _698_date_time_s date_time_s;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
//	temp_char=priv_698_state->addr.s_addr_len;//û�ӳ���
//	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	

	get_date_time_s(&date_time_s);
	save_char_point_data(hplc_data,hplc_data->dataSize,date_time_s.data,7);			
	return 0;
}


int oi_parameter_get_addr(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){

	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
	temp_char=priv_698_state->addr.s_addr_len;//��ַ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);//�����ַ

	return 0;
}



int action_response_charge_StartStop(CHARGE_STRATEGY * charge_strategy,struct  _698_FRAME  *_698_frame_rev){
	int i=0,j=0,count,len=0,position;
	rt_kprintf("[hplc]  (%s) \n",__func__);
	i=_698_frame_rev->time_flag_positon;//ָ���������,ע�������
	i++;//���� �ṹ��λ��
	if(_698_frame_rev->usrData[i++]!=2){//���� ��Ա����λ��
		rt_kprintf("[hplc]  (%s)  struct no. is not right i=%d!! \n",__func__,i);				
	}
	//·�����ʲ����  visible-string��SIZE(22)��
	i+=2;//���������һ����Ա������һ������
	len=_698_frame_rev->usrData[i]+1;//
	my_strcpy_char(charge_strategy->cRequestNO,(char *)_698_frame_rev->usrData,i,len);	
	
	//�û�ID visible-string��SIZE(64)��
	i+=len+1;//���������16λ��һ������
	len=1;//
	my_strcpy_char(charge_strategy->cUserID,(char *)_698_frame_rev->usrData,i,len);
	
	
	
	
	return 0;
}




int action_response_charge_strategy(CHARGE_STRATEGY * charge_strategy,struct  _698_FRAME  *_698_frame_rev){
	int i=0,j=0,count,len=0,position;
	rt_kprintf("[hplc]  (%s) \n",__func__);
	i=_698_frame_rev->time_flag_positon;//ָ���������,ע�������
	i++;//���� �ṹ��λ��
	if(_698_frame_rev->usrData[i++]!=10){//���� ��Ա����λ��
		rt_kprintf("[hplc]  (%s)  struct no. is not right i=%d!! \n",__func__,i);				
	}
	//������뵥�� octet-string��SIZE(16)��
	i+=2;//���������һ����Ա������һ������
	len=_698_frame_rev->usrData[i]+1;//
	my_strcpy_char(charge_strategy->cRequestNO,(char *)_698_frame_rev->usrData,i,len);	
	
	//�û�ID visible-string��SIZE(64)��
	i+=len+1;//���������16λ��һ������
	len=_698_frame_rev->usrData[i]+1;//
	my_strcpy_char(charge_strategy->cUserID,(char *)_698_frame_rev->usrData,i,len);
	
	//������  {��վ��1������������2��}
	i+=len+1;//���������λ��һ������
	//my_strcpy(&charge_strategy->ucDecMaker,_698_frame_rev->usrData,i,1);
	charge_strategy->ucDecMaker=_698_frame_rev->usrData[i];
	
	//��������{���ɣ�1�� ��������2��}
	i+=2;//���������λ��һ������
	charge_strategy->ucDecType=_698_frame_rev->usrData[i];
	
	//����ʱ��
	i+=3;//���������λ��һ������,����ĵ�һλ
	charge_strategy->strDecTime.Year=_698_frame_rev->usrData[i];//��
	charge_strategy->strDecTime.Month=_698_frame_rev->usrData[i++];//��
	charge_strategy->strDecTime.Day=_698_frame_rev->usrData[i++];//��	
	charge_strategy->strDecTime.Hour=_698_frame_rev->usrData[i++];//ʱ
	charge_strategy->strDecTime.Minute=_698_frame_rev->usrData[i++];//��	
	charge_strategy->strDecTime.Second=_698_frame_rev->usrData[i++];//��
	
	//·�����ʲ����  visible-string��SIZE(22)��	
	i+=2;//���������λ��һ������,
	len=_698_frame_rev->usrData[i]+1;//	
	
	my_strcpy_char(charge_strategy->cAssetNO,(char *)_698_frame_rev->usrData,i,len);
	
	//��������������λ��kWh�����㣺-2��double-long-unsigned
	i+=len+1;//���������λ��һ������,
	charge_strategy->ulChargeReqEle=_698_frame_rev->usrData[i]<<24;
	charge_strategy->ulChargeReqEle+=_698_frame_rev->usrData[i++]<<16;
	charge_strategy->ulChargeReqEle+=_698_frame_rev->usrData[i++]<<8;
	charge_strategy->ulChargeReqEle+=_698_frame_rev->usrData[i++];
	//�������  double-long����λ��kW�����㣺-4����//�����жϸ���ԭ��ת���Ϳ�����
	i+=2;//���������λ��һ������,
	charge_strategy->ulChargeRatePow=_698_frame_rev->usrData[i]<<24;
	charge_strategy->ulChargeRatePow+=_698_frame_rev->usrData[i++]<<16;
	charge_strategy->ulChargeRatePow+=_698_frame_rev->usrData[i++]<<8;
	charge_strategy->ulChargeRatePow+=_698_frame_rev->usrData[i++];	
	//���ʱ��  arrayʱ�γ�繦��		
	i+=2;//���������λ��һ������,
	count=_698_frame_rev->usrData[i];//����
	i++;//���������λ
	for(j=0;j<count;j++){
		//��ʼʱ��    date_time_s
		i+=3;//����һ���ṹ������,һ����������һ������
		charge_strategy->strChargeTimeSolts[j].strDecStartTime.Year=(_698_frame_rev->usrData[i]*256+_698_frame_rev->usrData[i+1])%2000;//��
		i++;
		charge_strategy->strChargeTimeSolts[j].strDecStartTime.Month=_698_frame_rev->usrData[i++];//��
		charge_strategy->strChargeTimeSolts[j].strDecStartTime.Day=_698_frame_rev->usrData[i++];//��	
		charge_strategy->strChargeTimeSolts[j].strDecStartTime.Hour=_698_frame_rev->usrData[i++];//ʱ
		charge_strategy->strChargeTimeSolts[j].strDecStartTime.Minute=_698_frame_rev->usrData[i++];//��	
		charge_strategy->strChargeTimeSolts[j].strDecStartTime.Second=_698_frame_rev->usrData[i++];//��		
		//����ʱ��    date_time_s��
		i+=8;//���������λ��һ������,
		charge_strategy->strChargeTimeSolts[j].strDecStopTime.Year=(_698_frame_rev->usrData[i]*256+_698_frame_rev->usrData[i+1])%2000;//��
		i++;
		charge_strategy->strChargeTimeSolts[j].strDecStopTime.Month=_698_frame_rev->usrData[i++];//��
		charge_strategy->strChargeTimeSolts[j].strDecStopTime.Day=_698_frame_rev->usrData[i++];//��	
		charge_strategy->strChargeTimeSolts[j].strDecStopTime.Hour=_698_frame_rev->usrData[i++];//ʱ
		charge_strategy->strChargeTimeSolts[j].strDecStopTime.Minute=_698_frame_rev->usrData[i++];//��	
		charge_strategy->strChargeTimeSolts[j].strDecStopTime.Second=_698_frame_rev->usrData[i++];//��		
		//��繦��    double-long����λ��kW�����㣺-4��
		i+=8;//���������λ��һ������,
		charge_strategy->strChargeTimeSolts[j].ulChargePow=_698_frame_rev->usrData[i]<<24;
		charge_strategy->strChargeTimeSolts[j].ulChargePow+=_698_frame_rev->usrData[i++]<<16;
		charge_strategy->strChargeTimeSolts[j].ulChargePow+=_698_frame_rev->usrData[i++]<<8;
		charge_strategy->strChargeTimeSolts[j].ulChargePow+=_698_frame_rev->usrData[i++];			
	}
	
	
	return 0;
}


int plan_fail_event_package(PLAN_FAIL_EVENT *priv_struct,struct CharPointDataManage * hplc_data){

//	int result=1,i=0,j=0,Value;
	unsigned char temp_char;//,*temp_array;
//	CHARGE_TIMESOLT *priv_struct_TIMESOLT;
	struct _698_date_time_s date_time_s;
	
	temp_char=Data_structure;//�ṹ��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=7;//�ṹ���Ա��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

//	Value=priv_struct->OrderNum;
	//�¼���¼��� 
	temp_char=Data_double_long_unsigned;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=((priv_struct->OrderNum&0xff000000)>>24);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->OrderNum&0x00ff0000)>>16);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->OrderNum&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=((priv_struct->OrderNum&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	//  �¼�����ʱ�� 
	temp_char=Data_date_time_s;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	STR_SYSTEM_TIME_to__date_time_s(&priv_struct->StartTimestamp,&date_time_s);
	
	save_char_point_data(hplc_data,hplc_data->dataSize,date_time_s.data,7);	

	// �¼�����ʱ��
	temp_char=Data_date_time_s;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	STR_SYSTEM_TIME_to__date_time_s(&priv_struct->FinishTimestamp,&date_time_s);
	
	save_char_point_data(hplc_data,hplc_data->dataSize,date_time_s.data,7);	

	//�¼�����Դ    enum��
	temp_char=Data_enum;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=priv_struct->Reason;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);


	//����û�ο����޷�д��
	
	return -1;
}
//

int charge_exe_state_package(CHARGE_EXE_STATE *priv_struct,struct CharPointDataManage * hplc_data){

//	int result=1,i=0,j=0,Value;
	int i=0;	
	unsigned char temp_char,*temp_array;
	
	temp_char=Data_structure;//�ṹ��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=12;//�ṹ���Ա��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//������뵥�� octet-string��SIZE(16)��
	temp_char=Data_octet_string;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=16;//����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_array=(unsigned char *)priv_struct->cRequestNO;
	save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,16);
	
	//·�����ʲ����  visible-string��SIZE(22)��
	temp_char=Data_visible_string;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=22;//�������������ϴ��߾���Ĭ����һ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_array=(unsigned char *)priv_struct->cAssetNO;
	save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,22);
	
	//ִ��״̬ {1������ִ�� 2��ִ�н��� 3��ִ��ʧ��}
	temp_char=Data_enum;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=priv_struct->exeState;//�������������ϴ��߾���Ĭ����һ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	//����ʾֵ��ֵ���� ���ƽ�ȣ�
	
	temp_char=Data_array;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=5;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//�� ���ƽ��
	for(i=0;i<5;i++){
//		Value=priv_struct->ulEleActualValue[i];	
		temp_char=Data_double_long;//
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		

		temp_char=((priv_struct->ulEleActualValue[i]&0xff000000)>>24);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct->ulEleActualValue[i]&0x00ff0000)>>16);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct->ulEleActualValue[i]&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

		temp_char=((priv_struct->ulEleActualValue[i]&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	}

	//��ǰ����ʾֵ���� ���ƽ�ȣ�
	
	temp_char=Data_array;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=5;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//�� ���ƽ��
	for(i=0;i<5;i++){
//		Value=priv_struct->ulEleActualValue[i];	
		temp_char=Data_double_long;//
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		

		
		temp_char=((priv_struct->ulEleActualValue[i]&0xff000000)>>24);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct->ulEleActualValue[i]&0x00ff0000)>>16);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct->ulEleActualValue[i]&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

		temp_char=((priv_struct->ulEleActualValue[i]&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	}

	//�ѳ�������� ���ƽ�ȣ�
	
	temp_char=Data_array;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=5;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//�� ���ƽ��
	for(i=0;i<5;i++){
//		Value=priv_struct->ucChargeEle[i];	
		temp_char=Data_double_long;//
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
			
		temp_char=((priv_struct->ucChargeEle[i]&0xff000000)>>24);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct->ucChargeEle[i]&0x00ff0000)>>16);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct->ucChargeEle[i]&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

		temp_char=((priv_struct->ucChargeEle[i]&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	}

	//��������������λ��kWh�����㣺-2��double-long-unsigned
//	Value=priv_struct->ucChargeTime;
	temp_char=Data_double_long_unsigned;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=((priv_struct->ucChargeTime&0xff000000)>>24);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucChargeTime&0x00ff0000)>>16);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucChargeTime&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=((priv_struct->ucChargeTime&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	



//	Value=priv_struct->ucPlanPower;
	temp_char=Data_double_long;//�ƻ���繦�ʣ���λ��W�����㣺-1��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		
	temp_char=((priv_struct->ucPlanPower&0xff000000)>>24);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucPlanPower&0x00ff0000)>>16);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucPlanPower&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=((priv_struct->ucPlanPower&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		

//	Value=priv_struct->ucActualPower;
	temp_char=Data_double_long;//��ǰ��繦�ʣ���λ��W�����㣺-1��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		
	temp_char=((priv_struct->ucActualPower&0xff000000)>>24);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucActualPower&0x00ff0000)>>16);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucActualPower&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=((priv_struct->ucActualPower&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		


	//  ��ǰ����ѹ����λ��V�����㣺-1,a b c���ࣩ
	
	temp_char=Data_array;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=1;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);


	//����
	temp_char=Data_long_unsigned;//��ǰ��繦�ʣ���λ��W�����㣺-1��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		
	temp_char=((priv_struct->ucVoltage&0xff00)>>8);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=(priv_struct->ucVoltage&0x00ff);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			


	//��ǰ����������λ��A�����㣺-3  a b c���ࣩ

	temp_char=Data_array;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=1;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	//����
//	Value=priv_struct->ucCurrent;
	temp_char=Data_double_long;//��ǰ��繦�ʣ���λ��W�����㣺-1��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		
	temp_char=((priv_struct->ucCurrent&0xff000000)>>24);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucCurrent&0x00ff0000)>>16);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct->ucCurrent&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=((priv_struct->ucCurrent&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//���׮״̬��1������2������3�����ϣ�
	temp_char=Data_enum;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=priv_struct->ChgPileState;//�������������ϴ��߾���Ĭ����һ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	return 0;
}




int charge_strategy_package(CHARGE_STRATEGY *priv_struct_STRATEGY,struct CharPointDataManage * hplc_data){
	int i=0,j=0;
	unsigned char temp_char,*temp_array;
	CHARGE_TIMESOLT *priv_struct_TIMESOLT;
	struct _698_date_time_s date_time_s;
	
	temp_char=Data_structure;//�ṹ��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=10;//�ṹ���Ա��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//������뵥�� octet-string��SIZE(16)��
	temp_char=Data_octet_string;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=16;//����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_array=(unsigned char *)priv_struct_STRATEGY->cRequestNO;
	save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,16);
	//�û�ID visible-string��SIZE(64)��
	temp_char=Data_visible_string;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=64;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_array=(unsigned char *)priv_struct_STRATEGY->cUserID;
	save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,64);
	//������  {��վ��1������������2��}
	temp_char=Data_enum;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=priv_struct_STRATEGY->ucDecMaker;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	//��������{���ɣ�1�� ��������2��}
	temp_char=Data_enum;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=priv_struct_STRATEGY->ucDecType;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	//����ʱ��
	temp_char=Data_date_time_s;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	STR_SYSTEM_TIME_to__date_time_s(&priv_struct_STRATEGY->strDecTime,&date_time_s);
	
	save_char_point_data(hplc_data,hplc_data->dataSize,date_time_s.data,7);			

	//·�����ʲ����  visible-string��SIZE(22)��
	temp_char=Data_visible_string;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=22;//�������������ϴ��߾���Ĭ����һ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_array=(unsigned char *)priv_struct_STRATEGY->cAssetNO;
	save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,22);		
	//��������������λ��kWh�����㣺-2��double-long-unsigned
//	Value=priv_struct_STRATEGY->ulChargeReqEle;
	temp_char=Data_double_long_unsigned;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=((priv_struct_STRATEGY->ulChargeReqEle&0xff000000)>>24);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct_STRATEGY->ulChargeReqEle&0x00ff0000)>>16);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct_STRATEGY->ulChargeReqEle&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=((priv_struct_STRATEGY->ulChargeReqEle&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	
	//�������  double-long����λ��kW�����㣺-4����//�����жϸ���ԭ��ת���Ϳ�����
//	Value=priv_struct_STRATEGY->ulChargeRatePow;
	
	temp_char=Data_double_long;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		
	temp_char=((priv_struct_STRATEGY->ulChargeRatePow&0xff000000)>>24);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct_STRATEGY->ulChargeRatePow&0x00ff0000)>>16);//�����ǲ������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

	temp_char=((priv_struct_STRATEGY->ulChargeRatePow&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=((priv_struct_STRATEGY->ulChargeRatePow&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			
	
	//���ģʽ      enum{������0��������1��}
	temp_char=Data_enum;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	temp_char=priv_struct_STRATEGY->ucDecType;//�������������ϴ��߾���Ĭ����һ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
			
	//���ʱ��  arrayʱ�γ�繦��
	temp_char=Data_array;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=priv_struct_STRATEGY->ucTimeSlotNum;//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	if(priv_struct_STRATEGY->ucTimeSlotNum==0){
		return 0;
	}
	
	for(j=0;j<priv_struct_STRATEGY->ucTimeSlotNum;i++){
		priv_struct_TIMESOLT=(CHARGE_TIMESOLT *)priv_struct_STRATEGY->strChargeTimeSolts+i;	
		temp_char=Data_structure;//�ṹ��
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

		temp_char=3;//�ṹ���Ա��
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
		//��ʼʱ��    date_time_s
		temp_char=Data_date_time_s;//
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		STR_SYSTEM_TIME_to__date_time_s(&priv_struct_TIMESOLT->strDecStartTime,&date_time_s);
		
		save_char_point_data(hplc_data,hplc_data->dataSize,date_time_s.data,7);					
		//����ʱ��    date_time_s��
		temp_char=Data_date_time_s;//
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
		STR_SYSTEM_TIME_to__date_time_s(&priv_struct_TIMESOLT->strDecStopTime,&date_time_s);
		
		save_char_point_data(hplc_data,hplc_data->dataSize,date_time_s.data,7);		

		//��繦��    double-long����λ��kW�����㣺-4��
//		Value=priv_struct_TIMESOLT->ulChargePow;
		temp_char=Data_double_long;//
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
			
		temp_char=((priv_struct_TIMESOLT->ulChargePow&0xff000000)>>24);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct_TIMESOLT->ulChargePow&0x00ff0000)>>16);//�����ǲ������
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			

		temp_char=((priv_struct_TIMESOLT->ulChargePow&0x0000ff00)>>8);//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

		temp_char=((priv_struct_TIMESOLT->ulChargePow&0x000000ff));//�����ǲ��������������Ҳ���ԣ�
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	}	
	return 0;
}




/**///

int oi_esam_info_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,i=0;
	unsigned char temp_char;
	CHARGE_STRATEGY *priv_struct_STRATEGY;
	
	hplc_ScmEsam_Comm.DataTx_len=0;
	hplc_current_ESAM_CMD=RD_INFO_FF;
	ESAM_Communicattion(hplc_current_ESAM_CMD,&hplc_ScmEsam_Comm);
	

	temp_char=(hplc_ScmEsam_Comm.DataRx_len-1);//�������������ϴ��߾���Ĭ����һ
//	rt_kprintf("[hplc]  (%s)   result=%d hplc_ScmEsam_Comm.DataRx_len=%d\n",__func__,result,hplc_ScmEsam_Comm.DataRx_len);
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	if(hplc_ScmEsam_Comm.DataRx_len<1){

		return -1;//û��������
	}else{
		result=0;		
		save_char_point_data(hplc_data,hplc_data->dataSize,(hplc_ScmEsam_Comm.Rx_data+4),(hplc_ScmEsam_Comm.DataRx_len-1));
	}


		return result;		
}

/**/

int oi_charge_strategy_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,i=0;
	unsigned char temp_char;
	CHARGE_STRATEGY *priv_struct_STRATEGY;
	
	temp_char=_698_charge_strategy.array_size;//�������������ϴ��߾���Ĭ����һ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	if(_698_charge_strategy.array_size==0){

		return 0;
	}
	
	for(i=0;i<_698_charge_strategy.array_size;i++){
		priv_struct_STRATEGY=(CHARGE_STRATEGY *)_698_charge_strategy.charge_strategy+i;
		charge_strategy_package(priv_struct_STRATEGY,hplc_data);
	
	}
	if(result == 0)	{//�������
		
	}	
	return result;		
}

int oi_electrical_pap(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,i=0;
	unsigned char temp_char,temp_array[20]={0,0,8,4,0,0,2,1,0,0,1,1,0,0,3,1,0,0,2,1};
	temp_char=(priv_698_state->oad_omd.oi[1]&OI2_MASK)>>4;
	switch (temp_char){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		case(0)://����
			result=-1;
			rt_kprintf("[hplc]  (%s)  conjunction    \n",__func__);
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){//

				get_data_class(priv_698_state,hplc_data,Data_array);

				//result=get_rap(temp_array);
				temp_char=5;//5������
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
				for(i=0;i<20;i++){
					if(i%4==0){//�ܼ��ƽ�ȣ������ѹ�м��ƽ��ô��
						temp_char=Data_double_long_unsigned;//��������
						save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
					}					
					temp_char=temp_array[i];//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);				
				}
				result =0;				
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==1  \n",__func__);
				return -1;
			}			
			break;
		case(1)://A ��
			result=-1;
			rt_kprintf("[hplc]  (%s)  a phase   \n",__func__);
			break;
		case(2)://B ��
			result=-1;
			rt_kprintf("[hplc]  (%s)  b phase  \n",__func__);		
			break;			
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  only supply communication addr \n",__func__);
			break;		
	}	
	if(result == 0)	{//�������
		
	}	
	return result;		
}

//int action_response_charge_strategy_package(CHARGE_STRATEGY_RSP  *ChgPlanIssue_rsp,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
//}

int oi_electrical_rap(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,i=0;
	unsigned char temp_char,temp_array[20]={8,0,0,4,2,0,0,1,2,0,0,1,2,0,0,1,2,0,0,1};
	temp_char=(priv_698_state->oad_omd.oi[1]&OI2_MASK)>>4;
	switch (temp_char){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		case(0)://����
			result=-1;
			rt_kprintf("[hplc]  (%s)  conjunction    \n",__func__);
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){//

				get_data_class(priv_698_state,hplc_data,Data_array);

				//result=get_rap(temp_array);
				temp_char=5;//5������
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
				for(i=0;i<20;i++){
					if(i%4==0){//�ܼ��ƽ�ȣ������ѹ�м��ƽ��ô��
						temp_char=Data_double_long_unsigned;//��������
						save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
					}					
					temp_char=temp_array[i];//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);				
				}
				result =0;				
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==1  \n",__func__);
				return -1;
			}			
			break;
		case(1)://A ��
			result=-1;
			rt_kprintf("[hplc]  (%s)  a phase   \n",__func__);
			break;
		case(2)://B ��
			result=-1;
			rt_kprintf("[hplc]  (%s)  b phase  \n",__func__);		
			break;			
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  only supply communication addr \n",__func__);
			break;		
	}	
	if(result == 0)	{//�������
		
	}	
	return result;		
}


/*

oi_variable


*/
int oi_variable_oib_meterage(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,i=0,j;
	unsigned char temp_char;
	ScmMeter_Analog prv_ScmMeter_Analog;
//	temp_char=priv_698_state->oad_omd.oi[1];
	switch (priv_698_state->oad_omd.oi[1]){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		case(0x00)://��ѹ
			result=-1;
			rt_kprintf("[hplc]  (%s)  voltage    \n",__func__);
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){//

				get_data_class(priv_698_state,hplc_data,Data_array);


				temp_char=1;//a���ѹ
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
				
				cmMeter_get_data(EMMETER_ANALOG,&prv_ScmMeter_Analog);	
				
				temp_char=Data_long_unsigned;//��������
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);					

				rt_kprintf("[hplc]  (%s)  prv_ScmMeter_Analog.ulVol=%d    \n",__func__,prv_ScmMeter_Analog.ulVol);

				temp_char=((prv_ScmMeter_Analog.ulCur&(0x0000ff00))>>8);//
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);				

				temp_char=(prv_ScmMeter_Analog.ulCur&(0x0000ff));//
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
				result =0;
				
	
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}			
			break;
		case(0x01)://����
			result=-1;
			rt_kprintf("[hplc]  (%s)  current    \n",__func__);
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){//

				get_data_class(priv_698_state,hplc_data,Data_array);


				temp_char=1;//a�����
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
				
				cmMeter_get_data( EMMETER_ANALOG,&prv_ScmMeter_Analog);	

				
				temp_char=Data_double_long;//��������
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	


				for(i=3;i>=0;i--){
					
					temp_char=((prv_ScmMeter_Analog.ulCur&(0xff000000>>((3-(i%4))*8)))>>((i%4)*8));//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);				
				}				
	
				result =0;
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}			
			break;
		case(0x04)://�й�����
			result=-1;
			rt_kprintf("[hplc]  (%s)  active  power    \n",__func__);
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){//

				get_data_class(priv_698_state,hplc_data,Data_array);


				temp_char=2;//�� �� a���й�����
				save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
				
				cmMeter_get_data( EMMETER_ANALOG,&prv_ScmMeter_Analog);	
				
				for(i=7;i>=0;i--){
					if((i+1)%4==0){//�ܼ��ƽ�ȣ������ѹ�м��ƽ��ô��
						temp_char=Data_double_long;//��������
						save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
					}	
							
					temp_char=((prv_ScmMeter_Analog.ulAcPwr&(0xff000000>>((3-(i%4))*8)))>>((i%4)*8));//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);				
				}
				result =0;
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				result=-1;
			}			
			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  only supply  \n",__func__);
			break;		
	}	
	if(result == 0)	{//�������
		
	}	
	return result;		

}
/*

oi_parameter,//�α��������


*/

int oi_electrical_oib_sum(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
	temp_char=(priv_698_state->oad_omd.oi[1]&OI1_MASK)>>4;
	switch (temp_char){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		case(0)://����й�
			result=-1;
			rt_kprintf("[hplc]  (%s)   Combined  Active Power \n",__func__);			
			break;
		case(1)://�����й�
			result=-1;
			rt_kprintf("[hplc]  (%s)   pasitive  Active Power \n",__func__);
			result=oi_electrical_pap(_698_frame_rev,priv_698_state,hplc_data);		
			break;
		case(2)://�����й�
			result=-1;
			rt_kprintf("[hplc]  (%s)  Reverse Active Power    \n",__func__);
			result=oi_electrical_rap(_698_frame_rev,priv_698_state,hplc_data);			
			break;			
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  only supply communication addr \n",__func__);
			break;		
	}	
	if(result == 0)	{//�������		
	}	
	return result;		
}


int oi_action_response_charge_oib(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,len;
	unsigned char temp_char,*temp_array;

//	rt_kprintf("[hplc]  (%s)   priv_698_state->oad_omd.oi[1]=%0x   \n",__func__,priv_698_state->oad_omd.oi[1]);
	switch (priv_698_state->oad_omd.oi[1]){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		/*case(0)://����ʱ��

			rt_kprintf("[hplc]  (%s)  not the operation     \n",__func__);
			result=-1;		
			break;*/
		
		
		
		case(0x01):
			result=-1;
			//�ж�����,������
			if(priv_698_state->oad_omd.attribute_id==0x7f){//�·����ƻ�
				if(_698_ChgPlanIssue.need_package==1){
					_698_ChgPlanIssue.need_package=0;
					
					temp_char=ChgPlanIssue_rsp.cSucIdle;//DAR�� �ɹ� �� 0����Ӳ��ʧЧ �� 1�������� ��255��
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		
		
					temp_char=Data_structure;//00 ���� Data OPTIONAL=0 ��ʾû������
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
					
					temp_char=2;//��Ա����
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);							
								
					//������뵥�� octet-string��SIZE(16)��
					temp_char=Data_octet_string;//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
					
					len=temp_char=(unsigned char )ChgPlanIssue_rsp.cRequestNO[0];//����
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
					
					temp_array=(unsigned char *)(ChgPlanIssue_rsp.cRequestNO+1);
					save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,len);	

					//·�����ʲ����  visible-string��SIZE(22)��
					temp_char=Data_visible_string;//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
					
					len=temp_char=(unsigned char )ChgPlanIssue_rsp.cAssetNO[0];//��������
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
					
					temp_array=(unsigned char *)(ChgPlanIssue_rsp.cAssetNO+1);
					save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,len);
					

					result=0;
				
				}else{
					//�ж�һ���û���û�������һ�γ��ƻ���û�о��˳�������ã����ڴ���һ���û��ϴ���ҵ��
					if((my_strategy_event_get()&ChgPlanIssue_EVENT)!=0){//�û���û�������һ�γ��ƻ�
						rt_kprintf("[hplc]  (%s) ()&ChgPlanIssue_EVENT)!=0     \n",__func__);
//						return 2;
					};
					if(1){//�ٴ���һ���û��ϴ���ҵ��
						check_afair_from_botom(priv_698_state,hplc_data);
					}
					
					copy_698_frame(&_698_ChgPlanIssue,_698_frame_rev);//����698֡,��ֱ�Ӹ�ֵ����ֵ�Ὣָ�븲��
					_698_ChgPlanIssue.time_flag_positon=_698_frame_rev->usrData_len;//���һλ��ֻ��������ʱ��Ч
		
					//������������
					action_response_charge_strategy(&charge_strategy_ChgPlanIssue,_698_frame_rev);//�浽�ܵ�����
					strategy_event_send(ChgPlanIssue_EVENT);
//					rt_event_send(&PowerCtrlEvent,ChgPlanIssue_EVENT);
					return 2;//�����¼�	
				}

				//�����ź�������ѳ��ƻ���׼����
			}else if(priv_698_state->oad_omd.attribute_id==128){//������ƻ�
				if(_698_ChgPlanAdjust.need_package==1){
					_698_ChgPlanAdjust.need_package=0;
					
					temp_char=ChgPlanAdjust_rsp.cSucIdle;//DAR�� �ɹ� �� 0����Ӳ��ʧЧ �� 1�������� ��255��
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		
		
					temp_char=Data_structure;//00 ���� Data OPTIONAL=0 ��ʾû������
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
					
					temp_char=2;//��Ա����
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);							
								
					//������뵥�� octet-string��SIZE(16)��
					temp_char=Data_octet_string;//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
					
					len=temp_char=(unsigned char )ChgPlanAdjust_rsp.cRequestNO[0];//����
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
					
					temp_array=(unsigned char *)(ChgPlanAdjust_rsp.cRequestNO+1);
					save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,len);	

					//·�����ʲ����  visible-string��SIZE(22)��
					temp_char=Data_visible_string;//
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
					
					len=temp_char=(unsigned char )ChgPlanAdjust_rsp.cAssetNO[0];//����
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
					
					temp_array=(unsigned char *)(ChgPlanAdjust_rsp.cAssetNO+1);
					save_char_point_data(hplc_data,hplc_data->dataSize,temp_array,len);	
					
					result=0;
				
				}else{
					if((my_strategy_event_get()&ChgPlanAdjust_EVENT)!=0){//�û���û�������һ�γ��ƻ�
						rt_kprintf("[hplc]  (%s) ()&ChgPlanAdjust_EVENT)!=0     \n",__func__);
//						return 2;
					};
					if(1){//�ٴ���һ���û��ϴ���ҵ��
						check_afair_from_botom(priv_698_state,hplc_data);
					}
					
					
					
					copy_698_frame(&_698_ChgPlanAdjust,_698_frame_rev);//����698֡,��ֱ�Ӹ�ֵ����ֵ�Ὣָ�븲��
					_698_ChgPlanAdjust.time_flag_positon=_698_ChgPlanAdjust.usrData_len;//���һλ��ֻ��������ʱ��Ч
					//������ƻ���
					action_response_charge_strategy(&charge_strategy_ChgPlanAdjust,_698_frame_rev);
					strategy_event_send(ChgPlanAdjust_EVENT);
					return 2;//�����¼�	
				}				
			
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}				
			break;		
			
		case(0x04)://������
			result=-1;
			//�ж�����,������
			if(priv_698_state->oad_omd.attribute_id==127){//������������
				if(_698_StartChg.need_package==1){
					_698_StartChg.need_package=0;
					
					temp_char=0;//DAR�� �ɹ� �� 0����Ӳ��ʧЧ �� 1�������� ��255��
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
					temp_char=0;//Data OPTIONAL=0 ��ʾû������
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);							
					result=0;
				
				}else{
					if((my_strategy_event_get()&StartChg_EVENT)!=0){//�û���û�������һ��
						rt_kprintf("[hplc]  (%s) ()&StartChg_EVENT)!=0     \n",__func__);
//						return 2;
					};
					if(1){//�ٴ���һ���û��ϴ���ҵ��
						check_afair_from_botom(priv_698_state,hplc_data);
					}
					
					
					
					copy_698_frame(&_698_StartChg,_698_frame_rev);//����698֡,��ֱ�Ӹ�ֵ����ֵ�Ὣָ�븲��
					_698_StartChg.time_flag_positon=_698_StartChg.usrData_len;//���һλ��ֻ��������ʱ��Ч

					rt_kprintf("[hplc]  (%s) start  rt_event_send \n",__func__);
					
//					action_response_charge_StartStop(&StartStopChg_ZHOU,_698_frame_rev);//�浽�ܵ�����
					
					
					strategy_event_send(StartChg_EVENT);
					return 2;//�����¼�	
				}

				//�����ź�������ѳ��ƻ���׼����
			}else if(priv_698_state->oad_omd.attribute_id==128){//ֹͣ��������
				if(_698_StopChg.need_package==1){
					_698_StopChg.need_package=0;
					
					temp_char=0;//DAR�� �ɹ� �� 0����Ӳ��ʧЧ �� 1�������� ��255��
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

					temp_char=0;//Data OPTIONAL=0 ��ʾû������
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			
				
					result=0;
				
				}else{
					if((my_strategy_event_get()&StopChg_EVENT)!=0){//�û���û�������һ�γ��ƻ�
						rt_kprintf("[hplc]  (%s) ()&StopChg_EVENT)!=0     \n",__func__);
//						return 2;
					};
					if(1){//�ٴ���һ���û��ϴ���ҵ��
						check_afair_from_botom(priv_698_state,hplc_data);
					}					
									
					copy_698_frame(&_698_StopChg,_698_frame_rev);//����698֡,��ֱ�Ӹ�ֵ����ֵ�Ὣָ�븲��
					_698_StopChg.time_flag_positon=_698_StopChg.usrData_len;//���һλ��ֻ��������ʱ��Ч
					rt_kprintf("[hplc]  (%s) stop  rt_event_send \n",__func__);
					//rt_event_send(&PowerCtrlEvent,StopChg_EVENT);
					strategy_event_send(StopChg_EVENT);
//						event=0x0000001<<Cmd_StopChgAck;
//						hplc_event=hplc_event|event;
					//rt_kprintf("[hplc]  (%s) stop  rt_event_send is ok \n",__func__);
					return 2;//�����¼�	
				}
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}				
			break;					
					
		default:

			rt_kprintf("[hplc]  (%s)  only supply operation=%0x \n",__func__,priv_698_state->oad_omd.oi[1]);
			result=-1;
			break;		
	}	
	if(result == 0)	{//�������	
	}
	
	return result;		
	

}

int oi_esam_oib(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,cmd,i=0,priv_len=0,times=0;
	unsigned char temp_char;

	switch (priv_698_state->oad_omd.oi[1]){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		case(0):
			result=-1;
			//�ж�����,ֻ��������2
			cmd=priv_698_state->oad_omd.attribute_id;
			rt_kprintf("[hplc]  (%s)  cmd==%d  \n",__func__,cmd);
			for(times=0;times<5;times++){
				hplc_ScmEsam_Comm.DataTx_len=0;
				if((cmd==2)||(cmd==4)){//esam ���к�   HOST_KEY_AGREE
					
					get_data_class(priv_698_state,hplc_data,Data_octet_string);
					ESAM_Communicattion(cmd,&hplc_ScmEsam_Comm);
					
					temp_char=(hplc_ScmEsam_Comm.DataRx_len-5);//�������������ϴ��߾���Ĭ����һ
				//	rt_kprintf("[hplc]  (%s)   result=%d hplc_ScmEsam_Comm.DataRx_len=%d\n",__func__,result,hplc_ScmEsam_Comm.DataRx_len);
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
					if(hplc_ScmEsam_Comm.DataRx_len<12){

						result=-1;//û��������
					}else{
						result=0;		
						save_char_point_data(hplc_data,hplc_data->dataSize,(hplc_ScmEsam_Comm.Rx_data+4),(hplc_ScmEsam_Comm.DataRx_len-5));
					}				
		
				}else if(cmd==7){//esam ���к�   HOST_KEY_AGREE
					get_data_class(priv_698_state,hplc_data,Data_structure);
					
					temp_char=4;//�������������ϴ��߾���Ĭ����һ
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);				


//					if(_698_frame_rev->addr.ca!=0){//
//						hplc_current_ESAM_CMD=1;
//						rt_kprintf("\n[hplc] addr.ca!=0 \n",__func__);
//					}else{
//						hplc_current_ESAM_CMD=cmd-1;
//						rt_kprintf("\n[hplc] addr.ca==0 \n",__func__);		
//					}					
					ESAM_Communicattion(cmd-1,&hplc_ScmEsam_Comm);


					
					if(hplc_ScmEsam_Comm.DataRx_len<12){

						result=-1;//û��������
					}else{
						result=0;	
						priv_len=	hplc_ScmEsam_Comm.DataRx_len-5;
						for(i=0;i<4;i++){
							
							temp_char=Data_double_long_unsigned;//�������������ϴ��߾���Ĭ����һ
							save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
							
							save_char_point_data(hplc_data,hplc_data->dataSize,(hplc_ScmEsam_Comm.Rx_data+(4*(i+1))),4);
									
						}
						
//						for(i=0;i<1;i++){
//							
//							temp_char=Data_double_long_unsigned;//�������������ϴ��߾���Ĭ����һ
//							save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//							
//							save_char_point_data(hplc_data,hplc_data->dataSize,(hplc_ScmEsam_Comm.Rx_data+(4*(i+1))),4);
//									
//						}
						
						
						
						
					}				
	
				}else{
					temp_char=0;//������Ϣ [0]  DAR��
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);						
					
					temp_char=6;//���󲻴��� �� 6����
					save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		

					
					rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
					return -1;
				}
				if(result==0){
					return 0;
				}
				rt_thread_mdelay(200);	//���ɹ�200�����ٶ�
			}
			temp_char=0;//������Ϣ [0]  DAR��
			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);						
			
			temp_char=2;//��ʱʧЧ �� 2����
			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		

			
				
			break;		

		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  only supply communication addr \n",__func__);
			break;		
	}	
	
	rt_kprintf("[hplc]  (%s)   result=%d\n",__func__,result);	
	if(result == 0)	{//�������	
		
	}
	
	return result;		
	

}


int oi_charge_oib(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;


	switch (priv_698_state->oad_omd.oi[1]){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		case(0)://����ʱ��
			result=-1;
//			printf("[hplc]  (%s)      \n",__func__);
			rt_kprintf("[hplc]  (%s)      \n",__func__);			
			break;
		case(0x01)://
			result=-1;
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){
				if(_698_ChgPlanIssueGet.need_package==1){					
					_698_ChgPlanIssueGet.need_package=0;
					get_data_class(priv_698_state,hplc_data,Data_array);
					result=oi_charge_strategy_package(_698_frame_rev,priv_698_state,hplc_data);
					//�����û�����				
				}else{
					if((my_strategy_event_get()&ChgPlanIssueGet_EVENT)!=0){//�û���û�������һ�γ��ƻ�
							rt_kprintf("[hplc]  (%s) ()&ChgPlanIssueGet_EVENT)!=0     \n",__func__);
//						return 2;
						};
						if(1){//�ٴ���һ���û��ϴ���ҵ��
							check_afair_from_botom(priv_698_state,hplc_data);
						}										
					
					copy_698_frame(&_698_ChgPlanIssueGet,_698_frame_rev);//����698֡
					strategy_event_send(ChgPlanIssueGet_EVENT);
					return 2;//�����¼�	
				}
				//�����ź�������ѳ��ƻ���׼����
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}				
			break;	


	case(0x08)://esam
			result=-1;
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){
	
					get_data_class(priv_698_state,hplc_data,Data_array);
//					hplc_ScmEsam_Comm.DataTx_len=0xff;//test
					result=oi_esam_info_package(_698_frame_rev,priv_698_state,hplc_data);
					//�����û�����				
	
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}				
			break;		







			
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  only supply communication addr \n",__func__);
			break;		
	}	
	if(result == 0)	{//�������	
	}
	
	return result;		
	

}


/*

oi_parameter,//�α��������


*/

int oi_parameter_oib_general(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;


	switch (priv_698_state->oad_omd.oi[1]){//�б�����λ����Ҫ�ж����Ի��߷��������٣���if�������ж�
		case(0)://����ʱ��
			result=-1;
			if(priv_698_state->oad_omd.attribute_id==2){
				get_data_class(priv_698_state,hplc_data,Data_date_time_s);
				result=oi_parameter_get_time(_698_frame_rev,priv_698_state,hplc_data);
			
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}				
			break;
		case(1)://ͨ�ŵ�ַ
			result=-1;
			//�ж�����,ֻ��������2
			if(priv_698_state->oad_omd.attribute_id==2){
				get_data_class(priv_698_state,hplc_data,Data_octet_string);

				result=oi_parameter_get_addr(_698_frame_rev,priv_698_state,hplc_data);
//				if(priv_698_state->meter_addr_send_ok==1){//�����豸�����ˣ���hplcȴû������
					priv_698_state->meter_addr_send_ok=2;
//				}				
			}else{
				rt_kprintf("[hplc]  (%s)  only deal   attribute_id==2  \n",__func__);
				return -1;
			}				
			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  only supply communication addr \n",__func__);
			break;		
	}	
	if(result == 0)	{//�������	
	}	
	return result;		
}



/*
�����������Ĵ���
����ǹ������ã�Ŀǰ�ô�����
*/

int get_response_variable_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
		
	temp_char=priv_698_state->oad_omd.oi[0]&OI2_MASK;//��ȡ�����ʶ
	switch (temp_char){//
		case(0)://����
			result=-1;
			rt_kprintf("[hplc]  (%s)    meterage   \n",__func__);
			result=oi_variable_oib_meterage(_698_frame_rev,priv_698_state,hplc_data);	
			break;
		case(1)://ͳ��
			result=-1;
			rt_kprintf("[hplc]  (%s)     \n",__func__);
			break;		
		case(2)://�ɼ�
			result=-1;
			rt_kprintf("[hplc]  (%s)     \n",__func__);
			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)   \n",__func__);
			break;		
	}	
		if(result == 0)	{//�������		
	}
	
	return result;		
}
/*
�����������Ĵ���
����ǹ������ã�Ŀǰ�ô�����
*/
int get_response_electrical_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
		
	temp_char=priv_698_state->oad_omd.oi[0]&OI2_MASK;//��ȡ�����ʶ
	switch (temp_char){//
		case(0)://��
			result=-1;
			rt_kprintf("[hplc]  (%s)   sum   \n",__func__);

			result=oi_electrical_oib_sum(_698_frame_rev,priv_698_state,hplc_data);	

			break;
		case(1)://����
			result=-1;
			rt_kprintf("[hplc]  (%s)   base component   \n",__func__);

			break;		
		case(2)://г��
			result=-1;
			rt_kprintf("[hplc]  (%s)   harmonic component   \n",__func__);


			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)   \n",__func__);

			break;		
	}	
		if(result == 0)	{//�������
		
	}
	
	return result;		
}

int action_response_charge_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
//	rt_kprintf("[hplc]  (%s)  \n",__func__);
		
	temp_char=priv_698_state->oad_omd.oi[0]&OI2_MASK;//��ȡ�����ʶ
	switch (temp_char){//
		case(0)://,
			result=-1;
			rt_kprintf("[hplc]  (%s)   charge   \n",__func__);

			result=oi_action_response_charge_oib(_698_frame_rev,priv_698_state,hplc_data);	

			break;
		case(1):////�α��������,
			result=-1;
			rt_kprintf("[hplc]  (%s)      \n",__func__);


			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)   \n",__func__);

			break;		
	}	
		if(result == 0)	{//�������
		
	}
	
	return result;		
}


int get_response_esam_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
		
	temp_char=priv_698_state->oad_omd.oi[0]&OI2_MASK;//��ȡ�����ʶ
	switch (temp_char){//
		case(0)://,
			result=-1;
			rt_kprintf("[hplc]  (%s)      \n",__func__);

			break;
		case(1):////�α��������,
			result=-1;

			rt_kprintf("[hplc]  (%s)   oi_esam_oib   \n",__func__);
			result=oi_esam_oib(_698_frame_rev,priv_698_state,hplc_data);	
//			rt_kprintf("[hplc]  (%s)   result=%d\n",__func__,result);
			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)   \n",__func__);

			break;		
	}	
		if(result == 0)	{//�������
		
	}
	
	return result;		
}




int get_response_charge_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
		
	temp_char=priv_698_state->oad_omd.oi[0]&OI2_MASK;//��ȡ�����ʶ
	switch (temp_char){//
		case(0)://,
			result=-1;
			rt_kprintf("[hplc]  (%s)   charge   \n",__func__);
			result=oi_charge_oib(_698_frame_rev,priv_698_state,hplc_data);	

			break;
		case(1):////�α��������,
			result=-1;
			rt_kprintf("[hplc]  (%s)      \n",__func__);


			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)   \n",__func__);

			break;		
	}	
		if(result == 0)	{//�������
		
	}
	
	return result;		
}

/*
������Ĵ���
����ǹ������ã�Ŀǰ�ô�����
*/
int get_response_parameter_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
		
	temp_char=priv_698_state->oad_omd.oi[0]&OI2_MASK;//��ȡ�����ʶ
	switch (temp_char){//
		case(0)://�α��������,
			result=-1;
			rt_kprintf("[hplc]  (%s)   case(0)   \n",__func__);

			result=oi_parameter_oib_general(_698_frame_rev,priv_698_state,hplc_data);	

			break;
		case(1):////�α��������,
			result=-1;
			rt_kprintf("[hplc]  (%s)   case(1)   \n",__func__);


			break;		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  default: \n",__func__);
			break;		
	}	
		if(result == 0)	{//�������		
	}	
	return result;		
}

/*
���� ���������ԡ�
����ֵ��result==1������Ҫ����

*/

int action_response_normal_omd(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;
	unsigned char temp_char;
	//����oad
//	rt_kprintf("[hplc]  (%s)  \n",__func__);
	temp_char=priv_698_state->oad_omd.oi[0];//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=priv_698_state->oad_omd.oi[1];//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=priv_698_state->oad_omd.attribute_id;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=priv_698_state->oad_omd.attribute_index;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	
	temp_char=(priv_698_state->oad_omd.oi[0]&OI1_MASK)>>4;//��ȡ�����ʶ
	switch (temp_char){//
		
		case(9)://9 ��������,
			result=-1;
			rt_kprintf("[hplc]  (%s)     chargepile \n",__func__);
			result=action_response_charge_oia(_698_frame_rev,priv_698_state,hplc_data);
		
			break;

		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  not support  oia=%0x  \n",__func__,temp_char);

			break;		
	}	
		if(result == 0)	{//�������
	
	
	}	
	
		return result;
	
}
/*
���� ���������ԡ�
����ֵ��result==1������Ҫ����

*/

int get_response_normal_oad(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,i=0;
	unsigned char temp_char;
	//����oad
	rt_kprintf("[hplc]  (%s)  \n",__func__);
	temp_char=priv_698_state->oad_omd.oi[0];//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	rt_kprintf("[hplc]  (%s)  priv_698_state->oad_omd.oi[0]=%0x\n",__func__,priv_698_state->oad_omd.oi[0]);	
	
	
	temp_char=priv_698_state->oad_omd.oi[1];//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=priv_698_state->oad_omd.attribute_id;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=priv_698_state->oad_omd.attribute_index;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	
	temp_char=(priv_698_state->oad_omd.oi[0]&OI1_MASK)>>4;//��ȡ�����ʶ
	switch (temp_char){//
		
		case(oi_electrical_energy)://0 �����������
			result=-1;
			rt_kprintf("[hplc]  (%s)     oi_electrical_energy \n",__func__);
			result=get_response_electrical_oia(_698_frame_rev,priv_698_state,hplc_data);

			break;
		
		
		case(oi_variable)://2 ���������
			result=-1;
			rt_kprintf("[hplc]  (%s)     oi_variable \n",__func__);
			result=get_response_variable_oia(_698_frame_rev,priv_698_state,hplc_data);

			break;		
		
		
		
		case(oi_parameter)://4 �α��������,
			result=-1;
			rt_kprintf("[hplc]  (%s)     oi_parameter \n",__func__);
			result=get_response_parameter_oia(_698_frame_rev,priv_698_state,hplc_data);

			break;

		case(9)://9 ��������,
			result=-1;
			rt_kprintf("[hplc]  (%s)     chargepile \n",__func__);
			result=get_response_charge_oia(_698_frame_rev,priv_698_state,hplc_data);

			break;

		case(0xf)://��ȡesam��Ϣ
			result=-1;
			rt_kprintf("[hplc]  (%s)    esam \n",__func__);
//			for(i=0;i<0;i++){
			
				result=get_response_esam_oia(_698_frame_rev,priv_698_state,hplc_data);		
//			}


			break;		
		
		
		
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)  not support  oia=%0x  \n",__func__,temp_char);

			break;		
	}	
		if(result == 0)	{//�������
	
	
	}	
	
		return result;
	
}

/*
  ���Ĵ���

*/
int unpatch_ScmEsam_Comm(struct CharPointDataManage * hplc_data,ScmEsam_Comm* l_stEsam_Comm){
	int result=0,i;
	int len_sid,len_a_data,len_data,len_mac,len_rn;
	int position_sid,position_a_data,position_data,position_mac,position_rn;
	unsigned char temp_char;

	rt_kprintf("[hplc]  (%s)  \n",__func__);
	
	if(hplc_data->_698_frame.usrData[1]!=1){//���Ĵ���
		rt_kprintf("[hplc]  (%s)  [1]!=1\n",__func__);	
	}
	
	if(hplc_data->_698_frame.usrData[2]==0x82){//
		rt_kprintf("[hplc]  (%s) length more then 1024   \n",__func__);	
	}
	
	len_data=hplc_data->_698_frame.usrData[3]*256+hplc_data->_698_frame.usrData[4];//data����

	position_data=(4+1);//��ʼλ�þ���2+1=3   1����������

	
	if(hplc_data->_698_frame.usrData[(position_data+len_data)]==0){
		//��������ͣ�0��������֤�� [0] SID_MAC����վ�ģ�2��  [2]�����+����MAC  
		rt_kprintf("[hplc]  (%s)  [2+len_data]=%0x!=0!!!!  \n",__func__,hplc_data->_698_frame.usrData[(position_data+len_data)]);		
		len_sid=4;
		position_sid=position_data+len_data+1;//��ʼλ�þ���(3+len_data)+1(���1�����������)

		len_a_data=hplc_data->_698_frame.usrData[position_sid+len_sid];
		position_a_data=position_sid+len_data+1;
		
		len_mac=hplc_data->_698_frame.usrData[position_a_data+len_a_data];
		position_mac=position_a_data+len_a_data+1;
		
		my_strcpy(l_stEsam_Comm->Tx_data,hplc_data->_698_frame.usrData,position_sid,len_sid);//
		l_stEsam_Comm->DataTx_len=len_sid;

		my_strcpy(l_stEsam_Comm->Tx_data+l_stEsam_Comm->DataTx_len,hplc_data->_698_frame.usrData,position_a_data,len_a_data);//	
		l_stEsam_Comm->DataTx_len+=len_a_data;
		
		my_strcpy(l_stEsam_Comm->Tx_data+l_stEsam_Comm->DataTx_len,hplc_data->_698_frame.usrData,position_data,len_data);//	
		l_stEsam_Comm->DataTx_len+=len_data;

		my_strcpy(l_stEsam_Comm->Tx_data+l_stEsam_Comm->DataTx_len,hplc_data->_698_frame.usrData,position_mac,len_mac);//	
		l_stEsam_Comm->DataTx_len+=len_mac;
	
	
	}	else 	if(hplc_data->_698_frame.usrData[(position_data+len_data)]==2){
		//��������ͣ�2�ǿ�������  [2]�����+����MAC  
		rt_kprintf("[hplc]  (%s)  [2+len_data]=%0x!=0!!!!  \n",__func__,hplc_data->_698_frame.usrData[(position_data+len_data)]);		

		len_rn=hplc_data->_698_frame.usrData[position_data+len_data+1];//������0
		position_rn=position_data+len_data+1+1;//��ʼλ��

		
		len_mac=hplc_data->_698_frame.usrData[position_rn+len_rn];
		position_mac=position_rn+len_rn+1;
		

		my_strcpy(l_stEsam_Comm->Tx_data+l_stEsam_Comm->DataTx_len,hplc_data->_698_frame.usrData,position_data,len_data);//	
		l_stEsam_Comm->DataTx_len+=len_data;

		my_strcpy(l_stEsam_Comm->Tx_data+l_stEsam_Comm->DataTx_len,hplc_data->_698_frame.usrData,position_mac,len_mac);//	
		l_stEsam_Comm->DataTx_len+=len_mac;
	
	}	
	

		
	return result;	
}


/*
  ���Ĵ���

*/
int security_get_package(struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=0,len_mac=0;//�����ͷ���1,������ͣ�result=0;
	unsigned char temp_char;
	//return -1;//û��ʵ��
	rt_kprintf("[hplc]  (%s) hplc_data->dataSize=%d \n",__func__,hplc_data->dataSize);
	//�Ƚ���

	hplc_ScmEsam_Comm.DataTx_len=hplc_data->_698_frame.usrData_len;
	my_strcpy(hplc_ScmEsam_Comm.Tx_data,hplc_data->_698_frame.usrData,0,hplc_data->_698_frame.usrData_len);
	
	hplc_current_ESAM_CMD=HOST_SESS_CALC_MAC_11;
	ESAM_Communicattion(hplc_current_ESAM_CMD,&hplc_ScmEsam_Comm);


	if(hplc_data->_698_frame.usrData_len>hplc_ScmEsam_Comm.DataRx_len){
		rt_kprintf("[hplc]  (%s) usrData_len>hplc_ScmEsam_Comm.DataRx_len \n",__func__);
		result =-1;
	}else{

		len_mac=hplc_ScmEsam_Comm.DataRx_len-hplc_data->_698_frame.usrData_len;//���ص�mac�ĳ���
		rt_kprintf("[hplc]  (%s)  len_mac=%d\n",__func__,len_mac);
	}

	
	temp_char=security_response;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=1;//����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=hplc_data->_698_frame.usrData_len;//����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_ScmEsam_Comm.Rx_data,hplc_data->_698_frame.usrData_len);	
	

	
	temp_char=1;//mac   ������֤��Ϣ CHOICE OPTIONAL   OPTIONAL��ʶ����ǿ�ѡ��
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=0;//choice
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=len_mac;//����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
		
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_ScmEsam_Comm.Rx_data+hplc_data->_698_frame.usrData_len,len_mac);	
	
	
	
	if(result == 0)	{//�������
		
		hplc_data->_698_frame.usrData_len=hplc_data->dataSize-priv_698_state->HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
		//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

		priv_698_state->FCS_position=hplc_data->dataSize;
		hplc_data->dataSize+=2;//�����ֽڵ�У��

			
		temp_char=hplc_data->_698_frame.end=0x16;
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô


	//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
		hplc_data->priveData[priv_698_state->len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

		hplc_data->priveData[priv_698_state->len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

	//У��ͷ
		//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
		result=tryfcs16(hplc_data->priveData, priv_698_state->HCS_position);
		hplc_data->_698_frame.HCS0=hplc_data->priveData[priv_698_state->HCS_position];	
		hplc_data->_698_frame.HCS1=hplc_data->priveData[priv_698_state->HCS_position+1];

		//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
		result=tryfcs16(hplc_data->priveData, priv_698_state->FCS_position);
		
		hplc_data->_698_frame.FCS0=hplc_data->priveData[priv_698_state->FCS_position];
		hplc_data->_698_frame.FCS1=hplc_data->priveData[priv_698_state->FCS_position+1];		
	}	
	
	
	return result;	
}


/*
  ��Ӧ��ȡһ����¼�Ͷ�����������

*/
int get_response_package_record(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;//�����ͷ���1,������ͣ�result=0;
	unsigned char temp_char;
	//return -1;//û��ʵ��
	rt_kprintf("[hplc]  (%s)  \n",__func__);
	temp_char=get_response;//���Բ���hplc_698_link_response,��Ϊ�˱��ⷽ�㻹�����ˣ����ҵ�¼�ò��˶���ʱ�䣬
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=_698_frame_rev->usrData[1];//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=_698_frame_rev->usrData[2];//PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int time_flag=3;	
	
	//	���´���oad,��������͵ľ���ô����
	oad_package(&priv_698_state->oad_omd,_698_frame_rev,3);	
	//oad_package(&priv_698_state->oad_omd,_698_frame_rev,3+4*i);//��һ��oad	
	//result=get_response_record_oad(_698_frame_rev,priv_698_state,hplc_data);

	if(result==0){
		time_flag+=4;
		
		temp_char=0;//FollowReport OPTIONAL=0 ��ʾû���ϱ���Ϣ
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
		if(_698_frame_rev->usrData[time_flag]==0){//����֡oad������Ǹ���ʱ���ʶ
			temp_char=0;//FollowReport OPTIONAL=0 ��ʾû���ϱ���Ϣ
			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			
		}else{//��ʱ���ǩ
		
		}	
	}				
	return result;	
}


/*
֪ͨ�û�

*/
int action_notice_user_normal(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state){
	int result=0;//�����ͷ���1,������ͣ�result=0;
	unsigned char temp_char;

	//	���´���oad,��������͵ľ���ô����
	oad_package(&priv_698_state->oad_omd,_698_frame_rev,3);//��oad����
	if(_698_frame_rev->need_package!=1){
		_698_frame_rev->time_flag_positon+=4;	//Ҳ�������ݵĿ�ʼ֡
	}	

//	90 02 7F 00  				�� OMD��������룩
//	00 00 						�� �ɹ����޸�������

	if(priv_698_state->oad_omd.oi[0]==0x90){
		if(priv_698_state->oad_omd.oi[1]==0x0){
			if(priv_698_state->oad_omd.attribute_id==0x7F){
				if(priv_698_state->oad_omd.attribute_index==0x0){
					if(_698_frame_rev->usrData[7]==0x0){
//						strategy_event_send();//֪ͨ�ܣ��������
						
					
					}	
				}	
			}	
		}	
	}
	
	
			
	return result;	

}


/*
��Ӧ����һ��������������

*/
int action_response_package_normal(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;//�����ͷ���1,������ͣ�result=0;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
	temp_char=action_response;//
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=_698_frame_rev->usrData[1];//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=_698_frame_rev->usrData[2];//PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	if(_698_frame_rev->need_package!=1){
		_698_frame_rev->time_flag_positon=3;	
	}
	//	���´���oad,��������͵ľ���ô����
	oad_package(&priv_698_state->oad_omd,_698_frame_rev,3);//��oad����
	if(_698_frame_rev->need_package!=1){
		_698_frame_rev->time_flag_positon+=4;	//Ҳ�������ݵĿ�ʼ֡
	}	
	//oad_package(&priv_698_state->oad_omd,_698_frame_rev,3+4*i);//��һ��oad	
	result=action_response_normal_omd(_698_frame_rev,priv_698_state,hplc_data);

	if(result==0){		
		
		temp_char=0;//FollowReport OPTIONAL=0 ��ʾû���ϱ���Ϣ
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
		if(_698_frame_rev->usrData[_698_frame_rev->time_flag_positon]==0){//����֡oad������Ǹ���ʱ���ʶ
			temp_char=0;//û��ʱ���ǩ
			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			
		}else{//��ʱ���ǩ
			rt_kprintf("[hplc]  (%s)  error need timeflag _698_frame_rev->time_flag_positon=%d\n",__func__,_698_frame_rev->time_flag_positon);
			result=-1;
		
		}	
	}				
	return result;	
}







/*
��Ӧ��ȡ���������������

*/


int get_response_package_normal_list(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1,i=0;//�����ͷ���1,������ͣ�result=0;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
	temp_char=get_response;//���Բ���hplc_698_link_response,��Ϊ�˱��ⷽ�㻹�����ˣ����ҵ�¼�ò��˶���ʱ�䣬
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=_698_frame_rev->usrData[1];//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=_698_frame_rev->usrData[2];//PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=_698_frame_rev->usrData[3];//oad����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	int time_flag=4;
		
	
	//	���´���oad,��������͵ľ���ô����
	for(i=0;i<_698_frame_rev->usrData[3];i++){
		oad_package(&priv_698_state->oad_omd,_698_frame_rev,(4+4*i));//��һ��oad	
		result=get_response_normal_oad(_698_frame_rev,priv_698_state,hplc_data);

	}	


//	rt_kprintf("[hplc]  (%s)  error result=%d\n",__func__,result);//		
	
	if(result==0){
		time_flag+=4*_698_frame_rev->usrData[3];
		
		temp_char=0;//FollowReport OPTIONAL=0 ��ʾû���ϱ���Ϣ
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
//		if(_698_frame_rev->usrData[time_flag]==0){//����֡oad������Ǹ���ʱ���ʶ
			temp_char=0;//FollowReport OPTIONAL=0 ��ʾû���ϱ���Ϣ
			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			
//		}else{//��ʱ���ǩ
//			
//			
//			
//		
//		}	
	}				
	return result;	
}

/*
��Ӧ��ȡһ��������������

*/


int get_response_package_normal(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;//�����ͷ���1,������ͣ�result=0;
	unsigned char temp_char;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
	temp_char=get_response;//���Բ���hplc_698_link_response,��Ϊ�˱��ⷽ�㻹�����ˣ����ҵ�¼�ò��˶���ʱ�䣬
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=_698_frame_rev->usrData[1];//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	temp_char=_698_frame_rev->usrData[2];//PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int time_flag=3;	
	
	//	���´���oad,��������͵ľ���ô����
	oad_package(&priv_698_state->oad_omd,_698_frame_rev,3);		
	result=get_response_normal_oad(_698_frame_rev,priv_698_state,hplc_data);

	if(result==0){
		time_flag+=4;
		
		temp_char=0;//FollowReport OPTIONAL=0 ��ʾû���ϱ���Ϣ
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
		
		if(_698_frame_rev->usrData[time_flag]==0){//����֡oad������Ǹ���ʱ���ʶ
			temp_char=0;//FollowReport OPTIONAL=0 ��ʾû���ϱ���Ϣ
			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			
		}else{//��ʱ���ǩ
		
		}	
	}				
	return result;	
}



/*
�������ã����û��ύ��
����ֵ��
*/

int action_response_notice_user(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state){

	int result=1;//�����ͷ���1,������ͣ�result=0;

	unsigned char temp_char;	
                               	
	switch (_698_frame_rev->usrData[1]){//�жϲ�������
		case(ActionRequest)://=1 ����һ�����󷽷����� [1] ��	
			rt_kprintf("[hplc]  (%s)     ActionRequest \n",__func__);
				action_notice_user_normal(_698_frame_rev, priv_698_state);
			break;
		
		case(ActionRequestList):// 2//�������ɸ����󷽷����� [2] ��		
			rt_kprintf("[hplc]  (%s)     ActionRequestList \n",__func__);
			//result=action_response_package_List(_698_frame_rev, priv_698_state,hplc_data);
			result=-1;
			break;		
		case(ActionThenGetRequestNormalList):// ActionThenGetRequestNormalList 3		
			rt_kprintf("[hplc]  (%s)     ActionThenGetRequestNormalList \n",__func__);
			//result=action_get_response_package_List(_698_frame_rev, priv_698_state,hplc_data);
			result=-1;
			break;		
		
		default:
			rt_kprintf("[hplc]  (%s)  not support type=%0x \n",__func__,_698_frame_rev->usrData[1]);
			result=-1;		
			break;		
	}
	
	
	return result;


}

/*
�������ã�����һ�����󷽷�
����ֵ��result==1������Ҫ����
*/

int action_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;//�����ͷ���1,������ͣ�result=0;

	unsigned char temp_char;	
	//�ṹ�帳ֵ����ͬ����
  hplc_data->dataSize=0;

	if(_698_frame_rev->need_package==1){
		
		
	}//��������ˣ���Ϊ���ô��������ܶ����⡣
	temp_char=hplc_data->_698_frame.head =_698_frame_rev->head;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	
	priv_698_state->len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵģ�����

	temp_char=hplc_data->_698_frame.control=CON_STU_U|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	
	
	//����������ַ
	hplc_data->_698_frame.addr.s_addr_len=priv_698_state->addr.s_addr_len;

	my_strcpy(hplc_data->_698_frame.addr.s_addr,priv_698_state->addr.s_addr,0,priv_698_state->addr.s_addr_len);//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);//


	temp_char=hplc_data->_698_frame.addr.ca=priv_698_state->addr.ca;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	priv_698_state->HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ

	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);		                               
	
	switch (_698_frame_rev->usrData[1]){//�жϲ�������
		case(ActionRequest)://=1 ����һ�����󷽷����� [1] ��	
			rt_kprintf("[hplc]  (%s)     ActionRequest \n",__func__);
			result=action_response_package_normal(_698_frame_rev, priv_698_state,hplc_data);
			break;
		
		case(ActionRequestList):// 2//�������ɸ����󷽷����� [2] ��		
			rt_kprintf("[hplc]  (%s)     ActionRequestList \n",__func__);
			//result=action_response_package_List(_698_frame_rev, priv_698_state,hplc_data);
			result=-1;
			break;		
		case(ActionThenGetRequestNormalList):// ActionThenGetRequestNormalList 3		
			rt_kprintf("[hplc]  (%s)     ActionThenGetRequestNormalList \n",__func__);
			//result=action_get_response_package_List(_698_frame_rev, priv_698_state,hplc_data);
			result=-1;
			break;		
		
		default:
			rt_kprintf("[hplc]  (%s)  not support type=%0x \n",__func__,_698_frame_rev->usrData[1]);
			result=-1;		
			break;		
	}

	if(result == 0)	{//�������
		
		hplc_data->_698_frame.usrData_len=hplc_data->dataSize-priv_698_state->HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
		//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

		priv_698_state->FCS_position=hplc_data->dataSize;
		hplc_data->dataSize+=2;//�����ֽڵ�У��

			
		temp_char=hplc_data->_698_frame.end=0x16;
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô


	//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
		hplc_data->priveData[priv_698_state->len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

		hplc_data->priveData[priv_698_state->len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

	//У��ͷ
		//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
		result=tryfcs16(hplc_data->priveData, priv_698_state->HCS_position);
		hplc_data->_698_frame.HCS0=hplc_data->priveData[priv_698_state->HCS_position];	
		hplc_data->_698_frame.HCS1=hplc_data->priveData[priv_698_state->HCS_position+1];

		//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
		result=tryfcs16(hplc_data->priveData, priv_698_state->FCS_position);
		
		hplc_data->_698_frame.FCS0=hplc_data->priveData[priv_698_state->FCS_position];
		hplc_data->_698_frame.FCS1=hplc_data->priveData[priv_698_state->FCS_position+1];		
	}
	
	
	return result;
}

/*

����ֵ��result==1������Ҫ����
*/

int get_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int result=1;//�����ͷ���1,������ͣ�result=0;

	unsigned char temp_char;	
	//�ṹ�帳ֵ����ͬ����
  hplc_data->dataSize=0;

	temp_char=hplc_data->_698_frame.head =_698_frame_rev->head;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	
	priv_698_state->len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵģ�����

	temp_char=hplc_data->_698_frame.control=CON_STU_U|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	
	
	//����������ַ
	hplc_data->_698_frame.addr.s_addr_len=priv_698_state->addr.s_addr_len;

	my_strcpy(hplc_data->_698_frame.addr.s_addr,priv_698_state->addr.s_addr,0,priv_698_state->addr.s_addr_len);//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);//


	temp_char=hplc_data->_698_frame.addr.ca=priv_698_state->addr.ca;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	priv_698_state->HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ

	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);		                               
	
	switch (_698_frame_rev->usrData[1]){//�ж�get����
		case(GetRequestNormal)://=1 ��ȡһ��������������		
			rt_kprintf("[hplc]  (%s)     GetRequestNormal \n",__func__);
			result=get_response_package_normal(_698_frame_rev, priv_698_state,hplc_data);
			break;
		
	
		
		case(GetRequestNormalList)://=2 ��ȡһ��������������		
			rt_kprintf("[hplc]  (%s)     GetRequestNormalList \n",__func__);
			result=get_response_package_normal_list(_698_frame_rev, priv_698_state,hplc_data);
			break;		
		
		
		
		case(GetRequestRecord)://=3 ��ȡһ����¼�Ͷ�����������		
			rt_kprintf("[hplc]  (%s)     GetRequestRecord \n",__func__);
			//result=get_response_package_record(_698_frame_rev, priv_698_state,hplc_data);
			result=-1;
			break;		
		
		default:
			rt_kprintf("[hplc]  (%s)  not support type=%0x \n",__func__,_698_frame_rev->usrData[1]);
			result=-1;		
			break;		
	}
//	rt_kprintf("[hplc]  (%s)  error result=%d\n",__func__,result);//	
	if(result == 0)	{//�������
		
		hplc_data->_698_frame.usrData_len=hplc_data->dataSize-priv_698_state->HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
		//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

		priv_698_state->FCS_position=hplc_data->dataSize;
		hplc_data->dataSize+=2;//�����ֽڵ�У��

			
		temp_char=hplc_data->_698_frame.end=0x16;
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô


	//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
		hplc_data->priveData[priv_698_state->len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

		hplc_data->priveData[priv_698_state->len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

	//У��ͷ
		//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
		result=tryfcs16(hplc_data->priveData, priv_698_state->HCS_position);
		hplc_data->_698_frame.HCS0=hplc_data->priveData[priv_698_state->HCS_position];	
		hplc_data->_698_frame.HCS1=hplc_data->priveData[priv_698_state->HCS_position+1];

		//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
		result=tryfcs16(hplc_data->priveData, priv_698_state->FCS_position);
		
		hplc_data->_698_frame.FCS0=hplc_data->priveData[priv_698_state->FCS_position];
		hplc_data->_698_frame.FCS1=hplc_data->priveData[priv_698_state->FCS_position+1];		
	}
	
	
	return result;
}

/*
�������ã����ؿ��õ�data_tx
          �������������ӵ���Ӧ

������
*/
int connect_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
	int len_position=0,HCS_position=0,result;
	int i=0,len=0;
	struct _698_connect_response prive_struct;
	unsigned char temp_char;	
	unPackage_698_connect_request(priv_698_state,_698_frame_rev,&prive_struct);
	
	//�ṹ�帳ֵ����ͬ����
  hplc_data->dataSize=0;

	temp_char=hplc_data->_698_frame.head =_698_frame_rev->head;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	
	len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵģ�����

	temp_char=hplc_data->_698_frame.control=CON_STU_U|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	
	
	//����������ַ
	hplc_data->_698_frame.addr.s_addr_len=priv_698_state->addr.s_addr_len;
//	if(hplc_data->_698_frame.addr.s_addr==RT_NULL){//���ͷ�
		//rt_free(hplc_data->_698_frame.addr.s_addr);//���жϴ��󣬶����Ե��ڴ�
		//hplc_data->_698_frame.addr.s_addr=(unsigned char *)rt_malloc(sizeof(unsigned char)*(priv_698_state->addr.s_addr_len));//����ռ�	
//	}	

	my_strcpy(hplc_data->_698_frame.addr.s_addr,priv_698_state->addr.s_addr,0,priv_698_state->addr.s_addr_len);//��������
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);//


	temp_char=hplc_data->_698_frame.addr.ca=_698_frame_rev->addr.ca;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ

	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);	
	
	//if(hplc_data->_698_frame.usrData==RT_NULL){//���ͷ�	
		//hplc_data->_698_frame.usrData=(unsigned char *)rt_malloc(sizeof(unsigned char)*(1024));//���û�����ռ�	
		//_698_frame_rev->usrData_size=1024;//�ռ��С
	//}	
	temp_char=connect_response;//���Բ���hplc_698_link_response,��Ϊ�˱��ⷽ�㻹�����ˣ����ҵ�¼�ò��˶���ʱ�䣬
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);



	temp_char=prive_struct.piid_acd;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	save_char_point_data(hplc_data,hplc_data->dataSize,prive_struct.connect_res_fv.manufacturer_code,4);//���̴��루 size(4) �� 
	save_char_point_data(hplc_data,hplc_data->dataSize,prive_struct.connect_res_fv.soft_version,4);//����汾�ţ� size(4) �� 
	save_char_point_data(hplc_data,hplc_data->dataSize,prive_struct.connect_res_fv.soft_date,6);//+����汾���ڣ� size(6) ��
	save_char_point_data(hplc_data,hplc_data->dataSize,prive_struct.connect_res_fv.hard_version,4);//+Ӳ���汾�ţ� size(4) �� 
	save_char_point_data(hplc_data,hplc_data->dataSize,prive_struct.connect_res_fv.hard_date,6);//Ӳ���汾���ڣ� size(6) �� 
	save_char_point_data(hplc_data,hplc_data->dataSize,prive_struct.connect_res_fv.manufacturer_ex_info,8);//+������չ��Ϣ�� size(8) ��


	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+2),2);//������Ӧ�ò�Э��汾��
	
	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+4),8);
	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+12),16);
	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+28),2);

	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+30),2);
	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+32),1);
	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+33),2);

	save_char_point_data(hplc_data,hplc_data->dataSize,(_698_frame_rev->usrData+35),4);//��ʱ
	
//��esamͨ�Ų���//

	//_698_frame_rev->usrData[40];//��SessionData1����
	hplc_ScmEsam_Comm.DataTx_len=_698_frame_rev->usrData[40];
	my_strcpy(hplc_ScmEsam_Comm.Tx_data,_698_frame_rev->usrData,41,_698_frame_rev->usrData[40]);	
	
	
	//_698_frame_rev->usrData[41+_698_frame_rev->usrData[40]];//��ucOutSign����
	hplc_ScmEsam_Comm.DataTx_len+=_698_frame_rev->usrData[41+_698_frame_rev->usrData[40]];

	my_strcpy(hplc_ScmEsam_Comm.Tx_data+_698_frame_rev->usrData[40],_698_frame_rev->usrData,(42+_698_frame_rev->usrData[40]),_698_frame_rev->usrData[41+_698_frame_rev->usrData[40]]);

	rt_kprintf("\n[hplc] hplc_ScmEsam_Comm.Tx_data \n",__func__);
	for(i=0;i<hplc_ScmEsam_Comm.DataTx_len;i++){
	
		rt_kprintf("%0x ",hplc_ScmEsam_Comm.Tx_data[i]);	
	}	
	rt_kprintf("over length=%d\n",hplc_ScmEsam_Comm.DataTx_len);	

	if(_698_frame_rev->addr.ca!=0){//
		hplc_current_ESAM_CMD=CON_KEY_AGREE;
		rt_kprintf("\n[hplc] addr.ca!=0 \n",__func__);
	}else{
		hplc_current_ESAM_CMD=HOST_KEY_AGREE;
		rt_kprintf("\n[hplc] addr.ca==0 \n",__func__);		
	}
	
	
	ESAM_Communicattion(hplc_current_ESAM_CMD,&hplc_ScmEsam_Comm);


	if(hplc_ScmEsam_Comm.DataRx_len<52){
		rt_kprintf("[hplc]  (%s)   result=%d hplc_ScmEsam_Comm.DataRx_len=%d\n",__func__,result,hplc_ScmEsam_Comm.DataRx_len);
		temp_char=0xff;//�������� �� 255��
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

		temp_char=0;//��֤������Ϣ SecurityData OPTIONAL
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
		 return -1;
	}else{
		temp_char=0;//��֤���  ConnectResult��
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

		temp_char=1;//��֤������Ϣ SecurityData OPTIONAL
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
		
		len=temp_char=(hplc_ScmEsam_Comm.Rx_data[2]*256+hplc_ScmEsam_Comm.Rx_data[3]-4);//+SessionData2	
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
		save_char_point_data(hplc_data,hplc_data->dataSize,(hplc_ScmEsam_Comm.Rx_data+4),len);
		
		temp_char=4;//MAC2
		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
		save_char_point_data(hplc_data,hplc_data->dataSize,(hplc_ScmEsam_Comm.Rx_data+len+4),4);	

		
//		//δ���Դ���
//		if(priv_698_state->session_key_negotiation==1){
//			//���������Ĳ��룬������������ͨ�������ݲ�ͬ�ģ���������ͨ��û������Ҫ�Ǻ������Ǹ�Ҫ�޸���Կ������ͨ������һ��ռ����ô��
//			priv_698_state->session_key_negotiation=0;
//		}else{
//			priv_698_state->session_key_negotiation=1;
//		}		
			
	}
	temp_char=0;//FollowReport
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

	temp_char=0;//time_tag
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);			


//	return -1;	
	
	hplc_data->_698_frame.usrData_len=hplc_data->dataSize-HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
	//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

	int FCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��

		
	temp_char=hplc_data->_698_frame.end=0x16;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô



//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	hplc_data->priveData[len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

	hplc_data->priveData[len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	


//У��ͷ
	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(hplc_data->priveData, HCS_position);
	hplc_data->_698_frame.HCS0=hplc_data->priveData[HCS_position];	
	hplc_data->_698_frame.HCS1=hplc_data->priveData[HCS_position+1];


	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(hplc_data->priveData, FCS_position);
	
	hplc_data->_698_frame.FCS0=hplc_data->priveData[FCS_position];
	hplc_data->_698_frame.FCS1=hplc_data->priveData[FCS_position+1];
	
	return result;
	
}	
	



/*

�������ã����ؿ��õ�data_tx

������size,������֮֡���֡���ȡ�
ע�⣺ʹ��ʱ����ȥ��Ҫ��_698_frame.usrDataָ��hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);	
*/

int link_response_package(struct  _698_FRAME  *_698_frame_rev ,struct _698_FRAME  * _698_frame_send,struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx){
	
	int i,len_position=0,HCS_position=0,result;
	struct _698_link_request hplc_698_link_request;
//	rt_kprintf("[hplc]  (%s)   link_response_package get inside \n",__func__); 	
//�Ѷ��徲̬����   hplc_698_link_response
	
	unsigned char temp_char;//����Ĳ�����
  unPackage_698_link_request(_698_frame_rev,&hplc_698_link_request,&i);
	//�ṹ�帳ֵ����ͬ����
  data_tx->dataSize=0;

	temp_char=_698_frame_send->head =_698_frame_rev->head;//��ʼ֡ͷ = 0x68	
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);//���������ô,������ȽϺ�


	
	len_position=data_tx->dataSize;
	data_tx->dataSize+=2;//�����ֽڵĳ���
	
	temp_char=_698_frame_send->control=CON_UTS_S|CON_LINK_MANAGE;   //������c,bit7,���䷽��λ
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);//���������ô

	
	temp_char=_698_frame_send->addr.sa=_698_frame_rev->addr.sa;
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);//���������ô


	//����������ַ
	_698_frame_send->addr.s_addr_len=_698_frame_rev->addr.s_addr_len;
	//if(_698_frame_send->addr.s_addr==RT_NULL){//���ͷ�
	//	_698_frame_send->addr.s_addr=(unsigned char *)rt_malloc(sizeof(unsigned char)*(_698_frame_send->addr.s_addr_len));//����ռ�		
	//}

	my_strcpy(_698_frame_send->addr.s_addr,_698_frame_rev->addr.s_addr,0,_698_frame_send->addr.s_addr_len);//��������
	save_char_point_data(data_tx,data_tx->dataSize,_698_frame_send->addr.s_addr,_698_frame_send->addr.s_addr_len);//���������ô


	temp_char=_698_frame_send->addr.ca=_698_frame_rev->addr.ca;
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);

	HCS_position=data_tx->dataSize;
	data_tx->dataSize+=2;//�����ֽڵ�У��λ



	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	_698_frame_send->usrData_len=0;//�û����ݳ��ȹ���

	
	//if(_698_frame_send->usrData==RT_NULL){//���ͷ�	
	//	_698_frame_send->usrData=(unsigned char *)rt_malloc(sizeof(unsigned char)*(1024));//���û�����ռ�	
	//	_698_frame_rev->usrData_size=1024;//�ռ��С
	//}
	
	temp_char=hplc_698_link_response.type=link_response;//���Բ���hplc_698_link_response,��Ϊ�˱��ⷽ�㻹�����ˣ����ҵ�¼�ò��˶���ʱ�䣬
																											//������ڷ����õ�ʱ��ܶ�����Ż�
	//temp_char=link_response;
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);
	
	temp_char=hplc_698_link_response.piid=hplc_698_link_request.piid_acd;
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);
	
	temp_char=hplc_698_link_response.result=0x80;//ͳͳ�ظ����ţ��ɹ������������˵����������
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);
	
	my_strcpy(hplc_698_link_response.date_time_ask.data,hplc_698_link_request.date_time.data,0,10);//10����������ʱ��
	save_char_point_data(data_tx,data_tx->dataSize,hplc_698_link_response.date_time_ask.data,10);//���������ô
		
	my_strcpy(hplc_698_link_response.date_time_rev.data,_698_frame_rev->rev_tx_frame_date_time.data,0,10);//10�����Ļ��֡��ʱ��
	save_char_point_data(data_tx,data_tx->dataSize,hplc_698_link_response.date_time_rev.data,10);//���������ô
		
	get_current_time(hplc_698_link_response.date_time_response.data);
	save_char_point_data(data_tx,data_tx->dataSize,hplc_698_link_response.date_time_response.data,10);//���������ô
	rt_kprintf("[hplc]  (%s)  3link_response_package get inside _698_frame_rev->head=%0x data_tx->priveData[0]=%0x \n",__func__,_698_frame_rev->head,data_tx->priveData[0]);

	_698_frame_send->usrData_len=33;//�û������ܳ���	,���濽���û����ݵ�usrData	
	//save_char_point_usrdata(_698_frame_send->usrData,&_698_frame_rev->usrData_size,data_tx->priveData,data_tx->dataSize-_698_frame_send->usrData_len,_698_frame_send->usrData_len);		


	int FCS_position=data_tx->dataSize;
	data_tx->dataSize+=2;//�����ֽڵ�У��

		
	temp_char=_698_frame_send->end=0x16;
	save_char_point_data(data_tx,data_tx->dataSize,&temp_char,1);//���������ô



//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	data_tx->priveData[len_position]=_698_frame_send->length0=(data_tx->dataSize-2)%256;//data_tx->size<1024ʱ

	data_tx->priveData[len_position+1]=_698_frame_send->length1=(data_tx->dataSize-2)/256;	


//У��ͷ
	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(data_tx->priveData, HCS_position);
	_698_frame_send->HCS0=data_tx->priveData[HCS_position];	
	_698_frame_send->HCS1=data_tx->priveData[HCS_position+1];


	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(data_tx->priveData, FCS_position);
	
	_698_frame_send->FCS0=data_tx->priveData[FCS_position];
	_698_frame_send->FCS1=data_tx->priveData[FCS_position+1];
	
	return result;


/*	for(i=0;i<data_tx->dataSize;i++){
		rt_kprintf("[hplc]  (%s)  data_tx->priveData[%d]=%0x ",i,data_tx->priveData[i]); 
	
		if(i%4==0){
			rt_kprintf("[hplc]  (%s)   \n",__func__);
		}
		
	}
	rt_kprintf("[hplc]  (%s)   \n",__func__); 	
	
*/

}

/*
���ܣ���������д���

Ҫ����������ı�������֡��ϵ�


���ڲ㣺Ӧ�ò㣬�����ݸ��������Ӧ�ý��̣�Ӧ�ý�����֡��ͽ����ݷ��ظ�
������size,������֮֡���֡���ȡ�

��Ҫ���͵ķ���0��

result==1������Ҫ����
*/


int rev_698_del_affairs(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev){
	int i,result=1,usr_data_size=0;//�����ͷ���1,������ͣ�result=0;
	unsigned char temp_char;
	int security_flag=0;
	if(data_rev->_698_frame.usrData[0]==security_request){//�ǰ�ȫ����,��Ҫ&&�Ѿ���ԿЭ�̹��ˣ��ƺ�ֻ����Կ������
		rt_kprintf("[hplc]  (%s)  security_request \n",__func__);
    //�ṹ�帳ֵ����ͬ����
		
		unpatch_ScmEsam_Comm(data_rev,&hplc_ScmEsam_Comm);
		
		hplc_current_ESAM_CMD=HOST_SESS_VERI_MAC;//��վ�Ự������֤MAC
		ESAM_Communicattion(hplc_current_ESAM_CMD,&hplc_ScmEsam_Comm);//spi���ӡ���

		if(data_rev->_698_frame.usrData[2]!=(hplc_ScmEsam_Comm.Rx_data[2]*256+hplc_ScmEsam_Comm.Rx_data[1])){
			rt_kprintf("[hplc]  (%s)  [2]!=(hplc_ScmEsam_Comm.Rx_data[2]*256 \n",__func__);		
		}


		data_rev->_698_frame.usrData_len=(hplc_ScmEsam_Comm.Rx_data[2]*256+hplc_ScmEsam_Comm.Rx_data[1]);
		my_strcpy(data_rev->_698_frame.usrData,hplc_ScmEsam_Comm.Rx_data,0,data_rev->_698_frame.usrData_len);//	

		rt_kprintf("[hplc]  (%s)  after unsecurity : \n",__func__);
		for(i=0;i<data_rev->_698_frame.usrData_len;i++){
			rt_kprintf("%0x \n",data_rev->_698_frame.usrData[i]);
		}
		rt_kprintf("\n");	
		usr_data_size=data_tx->dataSize;
		
		security_flag=1;
	}

	switch (data_rev->_698_frame.usrData[0]){//��������Ķ������ǡ�
		case(link_request)://�Ƿ����������ģ������������յ�		
			rt_kprintf("[hplc]  (%s)     link_request and do nothing \n",__func__);
			break;

		case(link_response)://��������¼���ͻ�����Ӧ���ǲ��Ƕ���˫��Ķ���Ҫʵ�֣���������
			rt_kprintf("[hplc]  (%s)  link_response  \n",__func__);
			if(data_rev->_698_frame.usrData[2]==0x80){//Ӧ���¼��Ӧ��ɿ�
				if(data_rev->_698_frame.usrData[2]==link_request_unload){	
					rt_kprintf("[hplc]  (%s)  link_request_unload \n",__func__);
					priv_698_state->link_flag=0;
					priv_698_state->connect_flag=0;//������Ҳ�Ͽ�				
				}
				if(data_rev->_698_frame.usrData[2]==link_request_load){	
					rt_kprintf("[hplc]  (%s)  link_request_load \n",__func__);
					priv_698_state->link_flag=1;
					priv_698_state->connect_flag=1;//��������λ				
				}				
				if(data_rev->_698_frame.usrData[2]==link_request_heart_beat){	
					rt_kprintf("[hplc]  (%s)  link_request_heart_beat and do nothing \n",__func__);										
				}								
			}else{
				rt_kprintf("[hplc]  (%s)  link_response result!=0x80  \n",__func__);
				return -1;
			}								
			break;		

		case(connect_request)://�ͻ�����������������յ�
			rt_kprintf("[hplc]  (%s)  connect_request  \n",__func__);			
			result=connect_response_package(&data_rev->_698_frame,priv_698_state,data_tx);//Ӧ��connect_request
			priv_698_state->connect_flag=1;
			break;		

		case(connect_response)://�������������Ӧ����Զ�������յ�
			rt_kprintf("[hplc]  (%s)  connect_response do nothing  \n",__func__);
			break;		

		case(release_request)://�ɿͻ���Ӧ�ý��̵���,�յ�����Ӧ��֡����λ�Ͽ����ӱ�־���������˳���¼
			rt_kprintf("[hplc]  (%s)  release_request \n",__func__);
			//result=release_response_package();
			priv_698_state->connect_flag=0;
			break;				
		
		case(release_response)://�������ɷ�����Ӧ�ý��̵��ã�
			rt_kprintf("[hplc]  (%s)  release_response  do nothing \n",__func__);
			break;		


		case(release_notification):	//�������ɷ�����Ӧ�ý��̵��ã�������Ҫ�ͻ������κ���Ӧ�����Բ����յ�
			rt_kprintf("[hplc]  (%s)  release_notification  do nothing \n",__func__);			
			break;		

		
		case(get_request)://�ɿͻ���Ӧ�ý��̵���
			rt_kprintf("[hplc]  (%s)  get_request \n",__func__);
			result=get_response_package(&data_rev->_698_frame,priv_698_state,data_tx);
			//����ʱ��������Ϳ�����
			break;

		case(action_request)://�ɿͻ���Ӧ�ý��̵���,����һ�����󷽷�
//			rt_kprintf("[hplc]  (%s)  action_request \n",__func__);
			result=action_response_package(&data_rev->_698_frame,priv_698_state,data_tx);
			//����ʱ��������Ϳ�����
			break;	

		case(action_response)://ֱ�ӽ���Ӧ�ò�Ϳ����ˣ�����ֱ�ӾͲ���
//			rt_kprintf("[hplc]  (%s)  action_request \n",__func__);
			result=action_response_notice_user(&data_rev->_698_frame,priv_698_state);

			break;						
		default:
			result=-1;
			rt_kprintf("[hplc]  (%s)   can not del the affair=%0x \n",__func__,data_rev->_698_frame.usrData[0]);
			break;		
	}
//	rt_kprintf("[hplc]  (%s)  error result=%d\n",__func__,result);//	
	
	
	if((security_flag==1)&&(result==0)){//�ǰ�ȫ����,��Ҫ&&�Ѿ���ԿЭ�̹��ˣ��ƺ�ֻ����Կ������
		//���û����������ؼ��ܣ�Ȼ�����´��

		data_tx->dataSize=usr_data_size;
		result=security_get_package(priv_698_state,data_tx);
	}	
	
	
	
	
	return result;
}
/*
* ����������Calculate a new fcs given the current fcs and the new data.
*/
unsigned short pppfcs16( unsigned short fcs, unsigned char *cp, int len){


	int i;
	while (len--){
		
		i=(fcs ^ *cp++) & 0xff;
		fcs=(fcs >> 8) ^ fcstab[i];
		/*if(len%10==0){
			rt_kprintf("[hplc]  (%s)  (fcs ^ *cp++) & 0xff %d  fcstab[(fcs ^ *cp++) & 0xff] %2x \n",__func__,i,fcstab[i]);
		
		}*/
	}
//	rt_kprintf("[hplc]  (%s)  fcs %2x PPPINITFCS16 %0x\n",__func__,fcs,PPPINITFCS16);
	return fcs;
}



/*
* How to use the fcs
�����Ҫ����������,�����Ķ�����ʱ�ļ��ſ��Դ���,
���ǰѲ��Զ�����Ӧ�ò�ȽϺ�
*/
int tryfcs16(unsigned char *cp, int len){
	unsigned short trialfcs;
	// add on output 
	
	trialfcs=pppfcs16(PPPINITFCS16, cp+1,len-1);//��У��68
	trialfcs ^= 0xffff; // complement 
	cp[len]=(trialfcs & 0x00ff); // least significant byte first
	cp[len+1]=((trialfcs >> 8) & 0x00ff);
	//rt_kprintf("[hplc]  (%s)  cp[len]= %0x,cp[len+1]= %0x\n",__func__,cp[len],cp[len+1]);
	// check on input 
	trialfcs=pppfcs16(PPPINITFCS16, cp+1,len + 2-1);
	unsigned short tempcompare = PPPGOODFCS16;
	if ( trialfcs == tempcompare ){
		rt_kprintf("[hplc]  (%s)  Good FCS\n",__func__);
		return 0;
	}
	return -1;
	
}



/*
��������
����������
����ֵ��

�������ã�
���ṹ�帳��static struct  _698_FRAME  _698_frame_rev
					������Ӧ����,


*/
int _698_unPackage(unsigned char * data,struct  _698_FRAME  *_698_frame_rev,int size){
//	unsigned char * usrdata;
	rt_kprintf("[hplc]  (%s)  \n",__func__);
	//int i;
	//_698_frame_rev->control=data[3];//ǰ���Ѿ�У��������Ϊ��һ֡�ǶԵ�
	_698_frame_rev->frame_apart=data[3]&CON_MORE_FRAME;//ȡ��֡��־���ȴ���û�з����,ֻ��־���룬���������һ��

	//�ṹ�帳ֵ	
	_698_frame_rev->head = data[0];//��ʼ֡ͷ = 0x68
	_698_frame_rev->length0=data[1];
	_698_frame_rev->length1=data[2];
	_698_frame_rev->length=((data[2]&0X3F)*256)+data[1];
	if(_698_frame_rev->length!=size-2){
		rt_kprintf("[hplc]  (%s)  _698_frame_rev->length!=size-2 \n",__func__);
		return -1;//���Ƚ�������
	}	
	//����,�����ֽ�,��bit0-bit13��Ч,�Ǵ���֡�г���ʼ�ַ��ͽ����ַ�֮���֡�ֽ������y
	_698_frame_rev->control=data[3];   //������c,bit7,���䷽��λ
	
	_address_unPackage(data,&_698_frame_rev->addr,4);//��ַ��a
	_698_frame_rev->addr.s_addr_len=6;
	_698_frame_rev->HCS0=data[6+_698_frame_rev->addr.s_addr_len];
	_698_frame_rev->HCS1=data[7+_698_frame_rev->addr.s_addr_len];


	_698_frame_rev->usrData_len=size-11-_698_frame_rev->addr.s_addr_len;//�����û����ݳ���

	if(_698_frame_rev->usrData_len<0||(_698_frame_rev->usrData_len>1024)){
		rt_kprintf("[hplc]  (%s)   _698_frame_rev->usrData_len<0 or >1024 \n",__func__);		
		return -1;//���Ƚ�������
	}else{
//		rt_kprintf("[hplc]  (%s)   _698_frame_rev->usrData_len=%d \n",__func__,_698_frame_rev->usrData_len);
	}

	//my_strcpy(_698_frame_rev->usrData,data,8+_698_frame_rev->addr.s_addr_len,_698_frame_rev->usrData_len);
	_698_frame_rev->usrData=data+(8+_698_frame_rev->addr.s_addr_len);
	
	_698_frame_rev->FCS0=data[size-3];
	_698_frame_rev->FCS1=data[size-2];
	_698_frame_rev->end=data[size-1];	
	
	return 0;
}
/*
	unsigned char sa;   //��bit0-bit3����,��Ӧ1��16,������Ҫ��һ    bit4-bit5���߼���ַ��bit6-bit�ǵ�ַ���� 
	unsigned char *s_add;//������sa����
	unsigned char ca;//�ͻ�����ַ
	int s_add_len;

*/
int _address_unPackage(unsigned char * data,struct _698_ADDR *_698_addr,int start_size){
//	unsigned char * s_addr;
	int i;
//	rt_kprintf("[hplc]  (%s)   \n",__func__);

	_698_addr->sa=data[start_size];	
	_698_addr->s_addr_len=(_698_addr->sa&0x0f)+1;

	if(_698_addr->s_addr_len<0||(_698_addr->s_addr_len>8)){
		rt_kprintf("[hplc]  (%s)   _698_addr->s_addr_len<0 or >8\n",__func__);		
		return -1;//���Ƚ�������
	}else{
		rt_kprintf("[hplc]  (%s)  s_addr_len=%d \n",__func__,_698_addr->s_addr_len);
	}
		
	for(i=0;i<_698_addr->s_addr_len;i++){
		_698_addr->s_addr[i]=data[start_size+1+i];
	}	
	_698_addr->ca=data[start_size+1+_698_addr->s_addr_len];
	return 0;	
}





/*

�����������Ҫ33λ�Ŀ��м��

*/
int _698_package(unsigned char * data,int size){

	return 0;
}
/*


*/

int _698_HCS(unsigned char *data, int start_size,int size,unsigned short HCS){

//	int i;
	unsigned short trialfcs;
	if (start_size<0){
		return -1;	
	}
	trialfcs=pppfcs16( PPPINITFCS16,data+start_size,size );
	unsigned short tempcompare = PPPGOODFCS16;
	if ( trialfcs == tempcompare ){
		rt_kprintf("[hplc]  (%s)  Good HCS\n",__func__);
		return 0;
	}else{
		rt_kprintf("[hplc]  (%s)  error HCS\n",__func__);
		return -1;
	}
}

/*
������������,����,���������ֽڳ���,У��ֵĬ������

�Ƕ�֡ͷ���ֳ���ʼ�ַ���HCS����֮��������ֽڵ�У��,start_sizeӦ�ú����1
*/

int _698_FCS(unsigned char *data, int start_size,int size,unsigned short FCS){

	//int i;
	unsigned short trialfcs;
	if (start_size<0){
		return -1;	
	}
	trialfcs=pppfcs16( PPPINITFCS16,data+start_size,size );

	//rt_kprintf("[hplc]  (%s)  size %d  data[size-2]= %0x,data[len+1]= %0x\n",__func__,size,data[size-2],data[size-1]);	
	unsigned short tempcompare = PPPGOODFCS16;
	if ( trialfcs == tempcompare ){
		rt_kprintf("[hplc]  (%s)  Good FCS\n",__func__);
		return 0;
	}else{
		rt_kprintf("[hplc]  (%s)  error FCS \n",__func__);
		return -2;
	}
}


/*
	��ȫ����Ӧ�ò㸺��
*/
int my_strcpy_char(char *dst,char *src,int startSize,int size){
	for(int i=0;i<size;i++){
		dst[i]=src[startSize+i];		
		//if(startSize==0){//������
		//	rt_kprintf("[hplc]  (%s)  dst[%d]= %0x src[%d]= %0x\n",__func__,i,dst[i],startSize+i,src[startSize+i]);	
		//}
	}				
	return 0;
}
int my_strcpy(unsigned char *dst,unsigned char *src,int startSize,int size){
	for(int i=0;i<size;i++){
		dst[i]=src[startSize+i];		
		//if(startSize==0){//������
		//	rt_kprintf("[hplc]  (%s)  dst[%d]= %0x src[%d]= %0x\n",__func__,i,dst[i],startSize+i,src[startSize+i]);	
		//}
	}				
	return 0;
}


/*

˵����������,������������,Ĭ������1024
*/

int array_inflate(unsigned char *data, int size,int more_size){
//	int tempSzie=0;
//	int i;
	return -1;
/*	unsigned char *p=RT_NULL; 	
	if(more_size<0){		
		rt_kprintf("[hplc]  (%s)  Hello RT-Thread! \n",__func__);
		return -1;
	}else if(more_size==0){
		more_size=1024;	
	}
	
	tempSzie=size + more_size;
	rt_kprintf("[hplc]  (%s)  size= %d tempSzie= %d\n",__func__,size,tempSzie);	

	p=(unsigned char *)rt_malloc(sizeof(unsigned char)*(tempSzie));

	//rt_memset(p,0,tempSzie);//���		
  if(size>0){
		for (i = 0; i < size; i++)
		{
			p[i] = data[i];
			//rt_kprintf("[hplc]  (%s)  data[%d] %0x\n",__func__,i,data[i]);
		}	
	
	}

	
	//rt_kprintf("[hplc]  (%s)  array_inflate will to  rt_free size=%d \n",__func__,size);
	if((data!=RT_NULL)&&(size!=0)){
		rt_free(data);	
	}
	rt_kprintf("[hplc]  (%s)  array_inflate over  rt_free size=%d \n",__func__,size);
	data = (unsigned char *)rt_malloc(sizeof(unsigned char)*(tempSzie));// data = p�������У�����p����Ϊ�Ǿ���ָ���ԭ��
	if(p!=RT_NULL){
		rt_free(p);	
	}
	if(size>0){
		for (i = 0; i < size; i++)
		{
			 data[i]= p[i];
		}		
	}		
	if(p!=RT_NULL){
		rt_free(p);	
	}
	return -1;*/	
}
/*

˵����������,�����������
*/

int array_deflate(unsigned char *data, int size,int de_size){
//	int tempSzie=0;
//	int i;
	return -1;
/*	
	if((size < de_size)||(de_size<0)){		
		rt_kprintf("[hplc]  (%s)  array_deflate  size < de_size\n",__func__);
		return -1;
	}else if((de_size==0)&&(size > 1024)){//������ʼ��С
		size=1024;
	}
	tempSzie=size - de_size;
	
	
	unsigned char *p = (unsigned char *)rt_malloc(sizeof(unsigned char)*(tempSzie));

	for (i = 0; i < tempSzie; i++)
	{
		p[i] = data[i];
	}
	//array_free(a);
	rt_free(data);
	data = p;
	return -1;	*/
}
/*



*/

int save_char_point_data(struct CharPointDataManage *hplc_data,int position,unsigned char *Res,int size){
	int i=0;
	if(size<0){
		return -1;
	
	}	
	//rt_kprintf("[hplc]  (%s)  save_char_point_data  get inside hplc_data->size=%d !\n",__func__,hplc_data->size);
	
	if(hplc_data->size > (position+size)){//�ռ仹��
		for(i=0;i<size;i++){
			hplc_data->priveData[position+i]=Res[i];
			
		}
		
		hplc_data->dataSize+=size;//��һ������position,�û���֡��Ӧ�����
		//rt_kprintf("[hplc]  (%s)  save_char_point_data  hplc_data->dataSize = %d!\n",__func__,hplc_data->dataSize);
	}else{
		rt_kprintf("[hplc]  (%s)  hplc_data->size=%d < !position+size %d  and return -1\n",__func__,hplc_data->size,position+size);		
		return -1;
/*		if(size<=1){
			if(array_inflate(hplc_data->priveData,hplc_data->size,1024)==0){
				hplc_data->size+=1024;
				rt_kprintf("[hplc]  (%s)  add 1024 size for hplc_data->size = %d!\n",__func__,hplc_data->size);
			}else{
				//rt_kprintf("[hplc]  (%s)  add 1024 size for hplc_data->size = %d   faild!\n",__func__,hplc_data->size);
				return -1;
			}		
		}else {
			if(array_inflate(hplc_data->priveData,hplc_data->size,size)==0){
				hplc_data->size+=size;
			}else{
				return -1;
			}						
		}		
		for(i=0;i<size;i++){
			hplc_data->priveData[position+i]=Res[i];	
			rt_kprintf("[hplc]  (%s)  hplc_data->priveData[position+i]=%0x Res[i]=%0x!\n",__func__,hplc_data->priveData[position+i],Res[i]);	
		}		
		hplc_data->dataSize+=size;//��һ������position,�û���֡��Ӧ�����			
*/
	}		
	
	return 0;	
}

/*

*/

int copy_ChgPlanIssue_rsp(CHARGE_STRATEGY_RSP *des,CHARGE_STRATEGY_RSP *src){
	my_strcpy_char(des->cRequestNO,src->cRequestNO,0,17);	
	my_strcpy_char(des->cAssetNO,src->cAssetNO,0,23);	//·�����ʲ����  visible-string��SIZE(22)��
	des->cSucIdle=src->cSucIdle;
	return 0;
}




/*�Է�����*/
int copy_charge_strategy(CHARGE_STRATEGY *des,CHARGE_STRATEGY *src){
//	int i;
	my_strcpy_char(des->cRequestNO,src->cRequestNO,0,17);
	//*des->cRequestNO=*src->cRequestNO;
	my_strcpy_char(des->cUserID,src->cUserID,0,65);
	//*des->cUserID=*src->cUserID;
	des->ucDecMaker=src->ucDecMaker;				//������  {��վ��1������������2��}
	des->ucDecType=src->ucDecType; 					//��������{���ɣ�1�� ��������2��}
	STR_SYSTEM_TIME strDecTime;							//����ʱ��
	des->strDecTime=src->strDecTime;
	
	my_strcpy_char(des->cAssetNO,src->cAssetNO,0,23);	//·�����ʲ����  visible-string��SIZE(22)��
	//*des->cAssetNO=*src->cAssetNO;
	des->ulChargeReqEle=src->ulChargeReqEle;					//��������������λ��kWh�����㣺-2��
	des->ulChargeRatePow=src->ulChargeRatePow;				//������� ����λ��kW�����㣺-4��
	des->ucChargeMode=src->ucChargeMode;							//���ģʽ {������0��������1��}
	des->ucTimeSlotNum=src->ucTimeSlotNum;						//ʱ�������
	//CHARGE_TIMESOLT strChargeTimeSolts[50];					//ʱ������ݣ����50��
	*des->strChargeTimeSolts=*src->strChargeTimeSolts;//�Ƿ�����ã�����֤

	return 0;
}


/*
Ŀ���Ǵ�0��ʼ��

*/
//int hplc_lock1=0,hplc_lock2=0;
//rt_uint32_t strategy_event;
//rt_uint32_t hplc_event=0;

rt_uint8_t strategy_event_send(COMM_CMD_C cmd){
//	rt_uint32_t event;
//����Դ
	while(hplc_lock1==1){
		rt_kprintf("[hplc]  (%s)   lock1==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_lock1=1;
	while(hplc_lock2==1){
		rt_kprintf("[hplc]  (%s)   lock2==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_lock2=1;	
	
	rt_kprintf("[hplc]  (%s)   cmd=%d  \n",__func__,cmd);	
	strategy_event=strategy_event|cmd;
	
	hplc_lock2=0;
	hplc_lock1=0;		
	return 0;
	
}

rt_uint32_t strategy_event_get(void){
	rt_uint32_t result=CTRL_NO_EVENT;
//	rt_uint32_t event;
//����Դ
	while(hplc_lock1==1){
		rt_kprintf("[hplc]  (%s)   lock1==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_lock1=1;
	while(hplc_lock2==1){
		rt_kprintf("[hplc]  (%s)   lock2==1  \n",__func__);		
		rt_thread_mdelay(20);
	}	
	hplc_lock2=1;	
//	rt_kprintf("[hplc]  (%s)   cmd=%d  \n",__func__,cmd);		


	result=strategy_event;
	strategy_event=CTRL_NO_EVENT;
	
	hplc_lock2=0;
	hplc_lock1=0;		
	return result;
}


rt_uint32_t my_strategy_event_get(void){
	rt_uint32_t result=CTRL_NO_EVENT;
//	rt_uint32_t event;
//����Դ
	while(hplc_lock1==1){
		rt_kprintf("[hplc]  (%s)   lock1==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_lock1=1;
	while(hplc_lock2==1){
		rt_kprintf("[hplc]  (%s)   lock2==1  \n",__func__);		
		rt_thread_mdelay(20);
	}	
	hplc_lock2=1;	
//	rt_kprintf("[hplc]  (%s)   cmd=%d  \n",__func__,cmd);		


	result=strategy_event;
	
	hplc_lock2=0;
	hplc_lock1=0;		
	return result;
}






/**
	��������
	1:���û������ݿ�������������λ�¼�

	2:���߽��û�������������������

*/

unsigned char * frome_user_tx;
rt_uint8_t CtrlUnit_RecResp(COMM_CMD_C cmd,void *STR_SetPara,int count){
	rt_uint8_t result=1;
	rt_uint32_t event;
	//frome_user_tx=STR_SetPara;//ÿ��ָ���һ����
	CHARGE_STRATEGY *prive_struct;
	CHARGE_STRATEGY_RSP * prive_struct_RSP;
	CHARGE_EXE_STATE * prive_struct_EXE_STATE;
//����Դ
	while(hplc_698_state.lock1==1){
		rt_kprintf("[hplc]  (%s)   lock1==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_698_state.lock1=1;
	while(hplc_698_state.lock2==1){
		rt_kprintf("[hplc]  (%s)   lock2==1  \n",__func__);
		rt_thread_mdelay(20);
	}
	hplc_698_state.lock2=1;	
	
//  if(cmd<32){	

	event=0x00000001<<cmd;
	rt_kprintf("[hplc]  (%s)   event=0x%4x  \n",__func__,event);
//	}
	switch(cmd){							//�ɼӲ���	
		case(Cmd_ChgPlanIssue):	//���ƻ��������û�
			rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanIssue  \n",__func__);	
			prive_struct=(CHARGE_STRATEGY *)STR_SetPara;			
			copy_charge_strategy(prive_struct,&charge_strategy_ChgPlanIssue);
//			*prive_struct=charge_strategy_ChgPlanIssue;
		
			//��������,ָ��ṹ��ֱ�Ӹ�ֵ���ɹ�����
			result=0;										
			break;

 		case(Cmd_ChgPlanAdjust)://������ƻ�,Ӧ�ò�õ����ݣ�����������һ��
			rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanAdjust  \n",__func__);
	
			prive_struct=(CHARGE_STRATEGY *)STR_SetPara;
			//copy_charge_strategy(prive_struct,&charge_strategy_ChgPlanAdjust);
			*prive_struct=charge_strategy_ChgPlanAdjust;//��������
			result=0;													
			break; 		

		
		case(Cmd_ChgPlanIssueAck)://�ƻ�����ɣ�����Ӧ��֡
			rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanIssueAck  \n",__func__);
			prive_struct_RSP=(CHARGE_STRATEGY_RSP *)STR_SetPara;
			ChgPlanIssue_rsp=*prive_struct_RSP;//������
			//copy_ChgPlanIssue_rsp(&ChgPlanIssue_rsp,prive_struct_RSP);
			hplc_event=hplc_event|event;
			result=0;								
			break;		
		
	
		case(Cmd_ChgPlanAdjustAck)://������ƻ�,�Դ�����
			rt_kprintf("[hplc]  (%s)  Cmd_ChgPlanAdjustAck  \n",__func__);
			prive_struct_RSP=(CHARGE_STRATEGY_RSP *)STR_SetPara;
			copy_ChgPlanIssue_rsp(&ChgPlanAdjust_rsp,prive_struct_RSP);		
			hplc_event=hplc_event|event;
			result=0;				
			break;

		case(Cmd_StartChg)://�����������·�,�޲�������
			rt_kprintf("[hplc]  (%s)   Cmd_StartChg  \n",__func__);	
			result=0;		
							
			break;
		
		case(Cmd_StartChgAck)://�������Ӧ��Ӧ���޲���		
			rt_kprintf("[hplc]  (%s)   Cmd_StartChgAck cmd=%d \n",__func__,cmd);
			hplc_event=hplc_event|event;
			result=0;		
									
			break;
		
		case(Cmd_StopChg)://ֹͣ�������·�		
			rt_kprintf("[hplc]  (%s)   Cmd_StopChg  \n",__func__);								
			break;
		
		case(Cmd_StopChgAck)://ֹͣ���Ӧ��		
			rt_kprintf("[hplc]  (%s)   Cmd_StopChgAck   cmd=%d \n",__func__,cmd);
			hplc_event=hplc_event|event;
			result=0;				
			break;

		case(Cmd_ChgRecord)://���ͳ��ƻ���
			prive_struct=(CHARGE_STRATEGY *)STR_SetPara;
			copy_charge_strategy(&charge_strategy_ChgRecord,prive_struct);		
			hplc_event=hplc_event|event;
			result=0;				
			rt_kprintf("[hplc]  (%s)   Cmd_ChgRecord  \n",__func__);								
			break;		
										
		case(Cmd_ChgPlanExeState)://ֻ�ϴ���Ӧ����� //���ͳ��ƻ�ִ��״̬

			prive_struct_EXE_STATE=(CHARGE_EXE_STATE *)STR_SetPara;
			Report_charge_exe_state=*((CHARGE_EXE_STATE *)STR_SetPara);//���ܸ�ֵ����
			hplc_event=hplc_event|event;
			result=0;				
			rt_kprintf("[hplc]  (%s)   Cmd_ChgPlanExeState  \n",__func__);								
			break;
		
		case(Cmd_DeviceFault)://ֻ�ϴ���Ӧ�����//����·�����쳣״̬ 
			rt_kprintf("[hplc]  (%s)   Cmd_DeviceFault  \n",__func__);
			_698_router_fail_event.plan_fail_event=(PLAN_FAIL_EVENT *)STR_SetPara;//��һ���ɹ�	
			_698_router_fail_event.array_size=count;
			hplc_event=hplc_event|event;
			result=0;	
			break;	

		
		case(Cmd_PileFault)://ֻ�ϴ���Ӧ�����//���ͳ��׮�쳣״̬ 
			rt_kprintf("[hplc]  (%s)   Cmd_PileFault  \n",__func__);
			_698_pile_fail_event.plan_fail_event=(PLAN_FAIL_EVENT *)STR_SetPara;//��һ���ɹ�	
			_698_pile_fail_event.array_size=count;
			hplc_event=hplc_event|event;
			result=0;	
			break;
		
		case(Cmd_ChgPlanIssueGetAck)://����Ҫ��������Ҫ���ݣ�ִֻ�оͿ�����
			_698_charge_strategy.charge_strategy=(CHARGE_STRATEGY *)STR_SetPara;	
			//����������⣬������ˣ�����������������е����ݣ��Ҳ�����ҲӦ�ò���
			_698_charge_strategy.array_size=count;
			hplc_event=hplc_event|event;
			//�Ƿ�Ҫ�ж��Ƿ����гɹ����ɹ���֮����Ƴ���
			result=0;	
			break;				
		default:
			rt_kprintf("[hplc]  (%s)   not  \n",__func__);
			return 1;
			break;	
	}
	
//����
	hplc_698_state.lock2=0;	
	hplc_698_state.lock1=0;	
	return result;			
}

int send_event(void){
	

	
//		rt_kprintf("[hplc]  send  NO_EVENT\n");
//		rt_event_send(&PowerCtrlEvent,NO_EVENT);
//		rt_thread_mdelay(500);
//		
	
//		rt_kprintf("[hplc]  send  ChgPlanIssue_EVENT\n");	
//		rt_event_send(&PowerCtrlEvent,ChgPlanIssue_EVENT);
//		rt_thread_mdelay(500);
	while(1){
	
		for(int i=1;i<5;i++){
			rt_kprintf("[hplc]  send  i=%d\n",i);		
			rt_event_send(&PowerCtrlEvent,(0x1<<i));	
			rt_thread_mdelay(500);
			rt_event_send(&PowerCtrlEvent,StopChg_EVENT);
		}	
	
	}


}
int save_char_point_usrdata(unsigned char *data,int *length,unsigned char *Res,int position,int size){

	int i=0;
	if(size<0){//������
		return -1;
	
	}
	
	if(*length > (size)){//�ռ仹��
		for(i=0;i<size;i++){
			data[i]=Res[position+i];		
		}

		
	}else{
		rt_kprintf("[hplc]  (%s)  *usrdata_size=%d < (size=%d) mem is not enough!\n",__func__,*length,size);
		return -1;
		/*if(size<=1){//��һ����������̶�
			if(array_inflate(data,*length,1024)==0){
				*length+=1024;
				rt_kprintf("[hplc]  (%s)  add 1024 size for   usrdata size*length =%d!\n",__func__,*length);//��Ҫ��Ϣ��Ҫ��ӡ
			}else{
				return -1;
			}	
	
		}else {
			if(array_inflate(data,*length,size)==0){
				*length+=size;
			}else{
				return -1;
			}				
		
		}
		for(i=0;i<size;i++){
			data[i]=Res[position+i];		
		}*/		

	}
	return 0;	
}

int get_current_time(unsigned char * data){
	
	//�Ӻ����ӿ��л�ȡʵʱʱ��

		
	
	
	for(int i=0 ;i<10; i++){
		data[i]=0xfe;	
	}
	return 0;

}
/*
���ܣ������֮֡���ʱ�������豸���Ĳ���
//�����֮֡���ʱ�������豸���Ĳ��ԣ�																																															
//�����жϵ�ַ�Բ��ԣ��������ǰ��Ҳ�ɲ���,��ʱ������
*/

int hplc_tx_frame(struct _698_STATE  * priv_698_state,rt_device_t serial,struct CharPointDataManage * data_tx){
	int i;
	rt_uint8_t buf[4]={0xfe,0xfe,0xfe,0xfe};
	//rt_kprintf("[hplc]  (%s)  data_tx->dataSize=%d\n",__func__,data_tx->dataSize);//��Ҫ��Ϣ��Ҫ��ӡ
	rt_device_write(serial, 0, &buf, 4);
	rt_device_write(serial, 0, data_tx->priveData, data_tx->dataSize);//������dma�ķ�ʽ��Ҫ��Ȼ������̫��ʱ��	
	rt_kprintf("[hplc]  (%s) :\n",__func__);
	for(i=0;i<data_tx->dataSize;i++){
		rt_kprintf("%02x ",data_tx->priveData[i]);	
	}
	rt_kprintf("\n");
	
	return 0;
}

int init_CharPointDataManage(struct CharPointDataManage *des){

//	des->priveData=RT_NULL;
//	rt_memset()
	des->size=1024;
	des->dataSize=0;
	des->next=RT_NULL;
	des->list_no=0;//�ȴ���Ӧ��������У������ʱ�˾ͷ��ͣ������Ӧ���˾���Ӧ��
	init_698_FRAME(&des->_698_frame);
	des->_698_frame.usrData=des->priveData;
	return 0;	
}

int free_CharPointDataManage(struct CharPointDataManage *des){

//	free_698_FRAME(&des->_698_frame);
//	if(des->priveData!=RT_NULL){
//		rt_free(des->priveData)	;
//	}
//	rt_free(des);	
	return 0;	
}

int my_free(unsigned char  *des){
//	unsigned char  *p;
//	if(des!=RT_NULL){
//		p=des;
//		rt_free(p);
//		des=RT_NULL;
//	}
	return 0;
}


int init_698_FRAME(struct  _698_FRAME  * des){
	des->addr.ca=0;
	des->addr.sa=0;
	
	//des->addr.s_addr=(unsigned char *)rt_malloc(sizeof(unsigned char)*(8));;
	
//	des->usrData_size=1024-8;
	des->usrData_len=0;
	des->need_package=0;
//	des->usrData=(unsigned char *)rt_malloc(sizeof(unsigned char)*(des->usrData_size));//����Ĳ�����ֻҪ���˿ռ�ͣ�����̬�ͷ��ˣ��Ͱ�������

	return 0;
}







/*
�������ָ��

*/
int free_698_FRAME(struct  _698_FRAME  * des){

//	if(des->addr.s_addr!=RT_NULL){
//		rt_free(des->addr.s_addr);	
//	}
//	if(des->usrData!=RT_NULL){
//		rt_free(des->usrData);	
//	}
//	rt_free(des);	
	return 0;
}








/*
	�ж���֡�ı��
*/



int judge_meter_no(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev){
	int i;
	for(i=0;i<priv_698_state->addr.s_addr_len;i++){
		if((data_rev->_698_frame.addr.s_addr[i]==0xaa)||(priv_698_state->addr.s_addr[i]==data_rev->_698_frame.addr.s_addr[i])){
//		if((priv_698_state->addr.s_addr[i]!=0xaa)&&(priv_698_state->addr.s_addr[i]==data_rev->_698_frame.addr.s_addr[i])){
//			rt_kprintf("[hplc]  (%s) right  \n",__func__);	
		}else{
			rt_kprintf("[hplc]  (%s)  NOT right %2X %2X \n",__func__,priv_698_state->addr.s_addr[i],data_rev->_698_frame.addr.s_addr[i]);
			return -1;
		}
	}
	return 0;		


}

int get_meter_addr(unsigned char * addr){//��Ҫ�����ṩ�ӿ�
unsigned char tmp_addr[6];
	addr[0]=0x1;
	addr[1]=0x00;	
	addr[2]=0x00;
	addr[3]=0x00;
	addr[4]=0x00;	
	addr[5]=0x00;
	
//	addr[0]=0x43;
//	addr[1]=0x04;	
//	addr[2]=0x00;
//	addr[3]=0x28;
//	addr[4]=0x00;	
//	addr[5]=0x00;	
	
//	if(GetStorageData(Cmd_MeterNumRd,tmp_addr,6)==0){
//		for(int i=0;i<6;i++){
//			addr[i]=tmp_addr[5-i];
//			rt_kprintf("[hplc]  (%s)  tmp_addr[%d]=%0x addr[]=%0x\n",__func__,i,tmp_addr[i],addr[i]);//��Ҫ��Ϣ��Ҫ��ӡ		
//		}
//		return 0;
//	
//	}else{
//		return -1;
//	}	

	return 0;	
}
int init_698_state(struct _698_STATE  * priv_698_state){
	//int i;	
	//����ַ
	priv_698_state->piid=0;//PIID �����ڿͻ��� APDU�� Client-APDU���ĸ��������������У�������
													//�����£�������Ӧ��Լ��Ӧ����ʵ��ϵͳҪ�������
	priv_698_state->version[0]=0x18;
	priv_698_state->version[1]=0x19;	
	
	
	priv_698_state->addr.sa=05;
	priv_698_state->addr.s_addr_len=6;

	get_meter_addr(priv_698_state->addr.s_addr);
	

	priv_698_state->addr.ca=00;//�ͻ�����ַ
			
	priv_698_state->heart_beat_time0=0x00;//̫��,������Ӧ��ֻ������������_698_frame���������ٷ�װһ��������
	priv_698_state->heart_beat_time1=0xb4;
	
	priv_698_state->FactoryVersion.manufacturer_code[0]=0x54;
	priv_698_state->FactoryVersion.manufacturer_code[1]=0x4F; 
	priv_698_state->FactoryVersion.manufacturer_code[2]=0x50;
	priv_698_state->FactoryVersion.manufacturer_code[3]=0x53;
	
	priv_698_state->FactoryVersion.soft_version[0]=0x30; 
	priv_698_state->FactoryVersion.soft_version[1]=0x31;
	priv_698_state->FactoryVersion.soft_version[2]=0x30; 
	priv_698_state->FactoryVersion.soft_version[3]=0x32; 
	
	priv_698_state->FactoryVersion.soft_date[0]=0x31;
	priv_698_state->FactoryVersion.soft_date[1]=0x36; 
	priv_698_state->FactoryVersion.soft_date[2]=0x30; 
	priv_698_state->FactoryVersion.soft_date[3]=0x37;  
	priv_698_state->FactoryVersion.soft_date[4]=0x33; 
	priv_698_state->FactoryVersion.soft_date[5]=0x31;
	
	priv_698_state->FactoryVersion.hard_version[0]=0x30; 
	priv_698_state->FactoryVersion.hard_version[1]=0x31;
	priv_698_state->FactoryVersion.hard_version[2]=0x30;
	priv_698_state->FactoryVersion.hard_version[3]=0x32; 
	
	priv_698_state->FactoryVersion.hard_date[0]=0x31; 
	priv_698_state->FactoryVersion.hard_date[1]=0x36;
	priv_698_state->FactoryVersion.hard_date[2]=0x30; 
	priv_698_state->FactoryVersion.hard_date[3]=0x37; 
	priv_698_state->FactoryVersion.hard_date[4]=0x33; 
	priv_698_state->FactoryVersion.hard_date[5]=0x31; 
	
	priv_698_state->FactoryVersion.manufacturer_ex_info[0]=0x00; 
	priv_698_state->FactoryVersion.manufacturer_ex_info[1]=0x00; 
	priv_698_state->FactoryVersion.manufacturer_ex_info[2]=0x00; 
	priv_698_state->FactoryVersion.manufacturer_ex_info[3]=0x00;
	priv_698_state->FactoryVersion.manufacturer_ex_info[4]=0x00; 
	priv_698_state->FactoryVersion.manufacturer_ex_info[5]=0x00; 
	priv_698_state->FactoryVersion.manufacturer_ex_info[6]=0x00;
	priv_698_state->FactoryVersion.manufacturer_ex_info[7]=0x00;
	
	get_current_time(priv_698_state->last_link_requset_time.data);//Э����ҲҪһ��ʱ��
	//ʱ��ȫΪ�㣬��ȥ�ĵ�һ�ξͳ�ʱ��Ȼ����
	
	priv_698_state->try_link_type=-1;
	priv_698_state->meter_addr_send_ok=0;
	priv_698_state->len_left=0;
	priv_698_state->len_sa=0;
	priv_698_state->len_all=0;
	priv_698_state->FE_no=0;
	priv_698_state->lock1=0;
	priv_698_state->lock2=0;
	priv_698_state->link_flag=0;
	priv_698_state->connect_flag=0;
	
	priv_698_state->session_key_negotiation=0;
	
	
	return 0;
	
}
int STR_SYSTEM_TIME_to__date_time_s(STR_SYSTEM_TIME * SYSTEM_TIME,struct _698_date_time_s *date_time_s){

	date_time_s->data[0]=20;//20��
	date_time_s->data[1]=(SYSTEM_TIME->Year>>4)*10+SYSTEM_TIME->Year&0x0f;//��	
	date_time_s->data[2]=(SYSTEM_TIME->Month>>4)*10+SYSTEM_TIME->Year&0x0f;//��
	date_time_s->data[3]=(SYSTEM_TIME->Day>>4)*10+SYSTEM_TIME->Day&0x0f;//��	
	date_time_s->hour=date_time_s->data[4]=(SYSTEM_TIME->Hour>>4)*10+SYSTEM_TIME->Hour&0x0f;//ʱ
	date_time_s->minute=date_time_s->data[5]=(SYSTEM_TIME->Minute>>4)*10+SYSTEM_TIME->Minute&0x0f;//��	
	date_time_s->second=date_time_s->data[6]=(SYSTEM_TIME->Second>>4)*10+SYSTEM_TIME->Second&0x0f;//��	
	return 0;
}


int Report_Cmd_PileFault(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state){
	int result=1;
	unsigned char temp_char;
	//�ṹ�帳ֵ����ͬ����

	hplc_data->dataSize=0;	
	temp_char=hplc_data->_698_frame.head = 0x68;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	int len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵĳ���	
	
	temp_char=hplc_data->_698_frame.control=CON_STU_S|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa ;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//������������ַ
	hplc_data->_698_frame.addr=priv_698_state->addr;
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);

	temp_char=hplc_data->_698_frame.addr.ca=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ	
	
	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);	                              
	
	
	temp_char=report_notification;//�ϱ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=ReportNotificationList;//�ϱ�����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x09;//�Լ�����PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	

	/**�û����ݵĽṹ�岿�֣��ο���ȡһ����¼�Ͷ�������**/

	//SEQUENCE OF A-ResultNormal
	temp_char=0x01;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	//�������������� OAD		
	temp_char=0x34;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x06;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x06;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//Get-Result	
	temp_char=0x01;//���� [1] Data
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	//	//	
	plan_fail_event_package(_698_pile_fail_event.plan_fail_event,hplc_data);
	

	temp_char=0x00;// û��ʱ���ǩ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		


	hplc_data->_698_frame.usrData_len=hplc_data->dataSize-HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
	//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

	
	
	
	int FCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��
		
	temp_char=hplc_data->_698_frame.end=0x16;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	if(hplc_data->dataSize>HPLC_DATA_MAX_SIZE){
			rt_kprintf("[hplc]  (%s)  >HPLC_DATA_MAX_SIZE too long   \n",__func__);
			return -1;	
	}
	
	hplc_data->priveData[len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

	hplc_data->priveData[len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

//У��ͷ
	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(hplc_data->priveData, HCS_position);
	hplc_data->_698_frame.HCS0=hplc_data->priveData[HCS_position];	
	hplc_data->_698_frame.HCS1=hplc_data->priveData[HCS_position+1];

	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(hplc_data->priveData, FCS_position);
	
	hplc_data->_698_frame.FCS0=hplc_data->priveData[FCS_position];
	hplc_data->_698_frame.FCS1=hplc_data->priveData[FCS_position+1];		


  //���账���



	return result;//������
}






/*

�������ã�

������

*/




int Report_Cmd_ChgRecord(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state){
	int result=1;
	unsigned char temp_char;
	//�ṹ�帳ֵ����ͬ����


	
	temp_char=ReportNotificationList;//�ϱ�����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x09;//�Լ�����PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	

	/**�û����ݵĽṹ�岿�֣��ο���ȡһ����¼�Ͷ�������**/  
	//SEQUENCE OF A-ResultNormal
	temp_char=0x01;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	//�������������� OAD		
	temp_char=0x34;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x02;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x06;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//Get-Result
	

	return result;//������
}//_698_frame_rev->��

int Report_Cmd_DeviceFault(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state){
	int result=1;
	unsigned char temp_char;
	//�ṹ�帳ֵ����ͬ����

	hplc_data->dataSize=0;	
	temp_char=hplc_data->_698_frame.head = 0x68;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	int len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵĳ���	
	
	temp_char=hplc_data->_698_frame.control=CON_STU_S|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa ;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//������������ַ
	hplc_data->_698_frame.addr=priv_698_state->addr;
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);

	temp_char=hplc_data->_698_frame.addr.ca=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ	
	
	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);	                              
	
	
	temp_char=report_notification;//�ϱ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=ReportNotificationList;//�ϱ�����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x09;//�Լ�����PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	

	/**�û����ݵĽṹ�岿�֣��ο���ȡһ����¼�Ͷ�������**/

	//SEQUENCE OF A-ResultNormal
	temp_char=0x01;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	//�������������� OAD		
	temp_char=0x34;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x06;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x06;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//Get-Result	
	temp_char=0x01;//���� [1] Data
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	//	//	
	plan_fail_event_package(_698_router_fail_event.plan_fail_event,hplc_data);
	

	temp_char=0x00;// û��ʱ���ǩ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		


	hplc_data->_698_frame.usrData_len=hplc_data->dataSize-HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
	//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

	
	
	
	int FCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��
		
	temp_char=hplc_data->_698_frame.end=0x16;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	if(hplc_data->dataSize>HPLC_DATA_MAX_SIZE){
			rt_kprintf("[hplc]  (%s)  >HPLC_DATA_MAX_SIZE too long   \n",__func__);
			return -1;	
	}
	
	hplc_data->priveData[len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

	hplc_data->priveData[len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

//У��ͷ
	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(hplc_data->priveData, HCS_position);
	hplc_data->_698_frame.HCS0=hplc_data->priveData[HCS_position];	
	hplc_data->_698_frame.HCS1=hplc_data->priveData[HCS_position+1];

	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(hplc_data->priveData, FCS_position);
	
	hplc_data->_698_frame.FCS0=hplc_data->priveData[FCS_position];
	hplc_data->_698_frame.FCS1=hplc_data->priveData[FCS_position+1];		


  //���账���



	return result;//������
}


/**
�ϱ� report_notification 0x88


**/

int report_notification_package(COMM_CMD_C report_type,void *report_struct,struct CharPointDataManage * hplc_data,struct _698_STATE  * priv_698_state){
	
	int result=1;
	unsigned char temp_char;
	//�ṹ�帳ֵ����ͬ����

	hplc_data->dataSize=0;	
	temp_char=hplc_data->_698_frame.head = 0x68;//��ʼ֡ͷ = 0x68	
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
	int len_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵĳ���	
	
	temp_char=hplc_data->_698_frame.control=CON_STU_S|CON_U_DATA;   //������c,bit7,���䷽��λ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	

	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa ;//& ADDR_SA_ADDR_LENGTH_MASK;//ֻȡ����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//������������ַ
	hplc_data->_698_frame.addr=priv_698_state->addr;
	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);

	temp_char=hplc_data->_698_frame.addr.ca=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	int HCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��λ	
	
	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);	                              
	
	
	temp_char=report_notification;//�ϱ�
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	temp_char=ReportNotificationList;//�ϱ�����
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x09;//�Լ�����PIID-ACD
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	

	/**�û����ݵĽṹ�岿�֣��ο���ȡһ����¼�Ͷ�������**/  
	//SEQUENCE OF A-ResultNormal
	temp_char=0x01;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
	
	//�������������� OAD		
	temp_char=0x34;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x02;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x06;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

	temp_char=0x00;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
	
	//Get-Result
	
	temp_char=0x01;//���� [1] Data
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
		
	charge_strategy_package(&charge_strategy_ChgRecord,hplc_data);

	temp_char=0x00;// û��ʱ���ǩ
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
	
	hplc_data->_698_frame.usrData_len=hplc_data->dataSize-HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
	//save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

	int FCS_position=hplc_data->dataSize;
	hplc_data->dataSize+=2;//�����ֽڵ�У��
		
	temp_char=hplc_data->_698_frame.end=0x16;
	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

//�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
	if(hplc_data->dataSize>HPLC_DATA_MAX_SIZE){
			rt_kprintf("[hplc]  (%s)  >HPLC_DATA_MAX_SIZE too long   \n",__func__);
			return -1;	
	}
	
	hplc_data->priveData[len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

	hplc_data->priveData[len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

//У��ͷ
	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
	result=tryfcs16(hplc_data->priveData, HCS_position);
	hplc_data->_698_frame.HCS0=hplc_data->priveData[HCS_position];	
	hplc_data->_698_frame.HCS1=hplc_data->priveData[HCS_position+1];

	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
	result=tryfcs16(hplc_data->priveData, FCS_position);
	
	hplc_data->_698_frame.FCS0=hplc_data->priveData[FCS_position];
	hplc_data->_698_frame.FCS1=hplc_data->priveData[FCS_position+1];		

	return result;//������

}



//................


int hplc_thread_init(void)
{
	rt_err_t res;
	int result;
	rt_kprintf("[hplc]  (%s)   \n",__func__);
	
	result=rt_event_init(&PowerCtrlEvent,"PowerCtrlEvent",RT_IPC_FLAG_FIFO);
	if(result!=RT_EOK){		
			rt_kprintf("[hplc]  (%s)  rt_event PowerCtrlEvent faild! \n",__func__);
	}else{
	
	}	
	res=rt_thread_init(&hplc,
											"hplc",
											hplc_thread_entry,
											RT_NULL,//parameter
											hplc_stack,//stack_start
											THREAD_HPLC_STACK_SIZE,
											THREAD_HPLC_PRIORITY,
											THREAD_HPLC_TIMESLICE);
	if (res == RT_EOK) 
	{
			rt_thread_startup(&hplc);
	}
	return res;
}

#if defined (RT_HPLC_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(hplc_thread_init);
#endif
MSH_CMD_EXPORT(hplc_thread_init, hplc thread run);







//����������





//��esam��ԿЭ��
//unsigned char esam_data[1024]={0x68 , 0x5e , 0x00 , 0x43 , 0x05 , 0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00
//	, 0x99 , 0xc2 , 
//	0x02 , 0x1e , 0x00 , 0x16 , 0xff , 0xff , 0xff , 0xff , 0xc0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x01
//	, 0xff , 0xfe , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xfa , 0x00
//	, 0xfa , 0x00 , 0x01 , 0xfa , 0x00 , 0x01 , 0xe1 , 0x33 , 0x80 , 0x02 , 0x20 , 0xa1 , 0x7e , 0x15 , 0x6b , 0xe2
//	, 0x13 , 0x8e , 0x65 , 0x09 , 0x6d , 0xe6 , 0x25 , 0x4a , 0x40 , 0xb0 , 0x28 , 0xbf , 0x74 , 0x55 , 0x5a , 0x67
//	, 0x22 , 0x8d , 0xf3 , 0xe5 , 0x86 , 0x42 , 0xc3 , 0xb7 , 0x57 , 0x15 , 0xa8 , 0x04 , 0x24 , 0xca , 0x11 , 0xa0
//	, 0x00 , 0x9e , 0x47 , 0x16
//};


//��ȡ��ѹ����
//unsigned char esam_data[1024]={0x68 , 0x29 , 0x00 , 0x81 , 0x05 , 0x07, 0x09 , 0x19 , 0x05 , 0x16 , 0x20 
//, 0x00 , 0x11 , 0x11 , 0x05 , 0x02 , 0x02 , 0x02 , 0x20 , 0x00 , 0x02 , 0x00 , 0x20 , 0x01 , 0x02 , 0x00 
//, 0x00 , 0x05 , 0x02 , 0x02 , 0x02 , 0x20 , 0x00 , 0x02 , 0x00 , 0x20 , 0x01 , 0x02 , 0x00 , 0x00 , 0x11 
//, 0x11, 0x16};

////��ȡ��ѹ����
//unsigned char esam_data[1024]={0x68 , 0xe1 , 0x00 , 0x43 , 0x05 , 0x23 , 0x01 , 0x00 , 0x00 , 0x30 
//	, 0x12 , 0x00 , 0x9d , 0xaf , 0x10 , 0x01 , 0x82 , 0x00 , 0xc0 , 0x4b , 0x8b , 0x45 , 0x42 , 0x9c 
//	, 0x57 , 0x59 , 0xf7 , 0x78 , 0xb9 , 0x79 , 0xdf , 0x0c , 0x55 , 0xab , 0x97 , 0x2c , 0xbf , 0x8a 
//	, 0xd8 , 0x70 , 0xe5 , 0x2d , 0x19 , 0x8a , 0xb0 , 0x89 , 0x3e , 0xdf , 0x4b , 0xe6 , 0x49 , 0xa5 
//	, 0xd2 , 0xe9 , 0x56 , 0x5c , 0x4a , 0x30 , 0x77 , 0x6c , 0x2d , 0x63 , 0x6a , 0xb9 , 0xe0 , 0xc3 
//	, 0xc4 , 0x39 , 0xc3 , 0x44 , 0x3a , 0x9a , 0xe1 , 0x29 , 0xe0 , 0x0c , 0x6c , 0x14 , 0x8f , 0xcf 
//	, 0x8c , 0x4a , 0xa1 , 0xef , 0x39 , 0x92 , 0x97 , 0xc3 , 0xe6 , 0x7f , 0x49 , 0x5a , 0xb4 , 0x30 
//	, 0x64 , 0x4b , 0xeb , 0x29 , 0x8e , 0xdd , 0xee , 0xf3 , 0xc8 , 0x33 , 0x33 , 0xbf , 0xf9 , 0x79 
//	, 0x51 , 0x77 , 0x71 , 0x9a , 0xcc , 0x28 , 0x32 , 0xbb , 0xb8 , 0xda , 0xd6 , 0x41 , 0xf1 , 0x7b 
//	, 0x44 , 0xa3 , 0x6f , 0x35 , 0xbf , 0x09 , 0x7b , 0x66 , 0x36 , 0xe6 , 0x4c , 0xd2 , 0xf3 , 0x1e 
//	, 0x85 , 0xdf , 0xe0 , 0x2a , 0x18 , 0x4c , 0xaf , 0xff , 0x2e , 0x0e , 0xf8 , 0xdb , 0x91 , 0x16 
//	, 0x7e , 0xfc , 0x91 , 0x99 , 0x59 , 0xd9 , 0x9d , 0xf6 , 0x66 , 0xe4 , 0x23 , 0xd3 , 0x44 , 0xa8 
//	, 0x07 , 0x99 , 0x19 , 0xc9 , 0x20 , 0x5b , 0xec , 0xca , 0x6a , 0x4e , 0xf2 , 0xbc , 0xaa , 0x6c 
//	, 0xef , 0x0e , 0x8e , 0x57 , 0x49 , 0x27 , 0x24 , 0xfc , 0xc8 , 0x21 , 0x2d , 0x42 , 0x92 , 0xd7 
//	, 0x2a , 0xeb , 0x31 , 0x6f , 0xe9 , 0x73 , 0x5d , 0x28 , 0xe4 , 0xd9 , 0x9c , 0x85 , 0x3f , 0x3b 
//	, 0xe1 , 0x31 , 0xd3 , 0x1d , 0x0f , 0x00 , 0x80 , 0x1c , 0x33 , 0x10 , 0x02 , 0x00 , 0xc4 , 0x04 
//	, 0xda , 0xd0 , 0x35 , 0x5b , 0xa6 , 0x65 , 0x16
//};


//����ͨ��esam���мӽ���
//	send_event();
	
//	package_for_test(&hplc_data_rev._698_frame,&hplc_698_state,&hplc_data_tx);	
//	rt_kprintf("[hplc]  (%s)  print data_tx:\n",__func__);//����	
//	printmy(&hplc_data_tx._698_frame);			

//	for(i=0;i<hplc_data_tx.dataSize;i++){
//			rt_kprintf("%0x ",hplc_data_tx.priveData[i]);//����	
//	}
//	rt_kprintf("\n---------------------------------\n");//����		
//	rt_kprintf("\n---------------------------------\n");//����		

//		while(1){
//			hplc_current_ESAM_CMD=RD_INFO;
//			ESAM_Communicattion(hplc_current_ESAM_CMD,&hplc_ScmEsam_Comm);
//			hplc_ScmEsam_Comm.DataRx_len=hplc_ScmEsam_Comm.Rx_data[2]*256+hplc_ScmEsam_Comm.Rx_data[3]+5;
//			for(i=0;i < hplc_ScmEsam_Comm.DataRx_len;i++){	
//				rt_kprintf("[%d]= %0x |",i,hplc_ScmEsam_Comm.Rx_data[i]);	
//			}
//			rt_kprintf("\n");			
//			rt_kprintf("[hplc]  (%s)  test esam hplc_ScmEsam_Comm.DataRx_len=%d!!!!!!\n",__func__,hplc_ScmEsam_Comm.DataRx_len);
//			rt_thread_mdelay(5000);			
//			
//		}

//int package_for_test(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data){
//	int result=1;//�����ͷ���1,������ͣ�result=0;

//	unsigned char temp_char,temp_arry[65]={0x90,0x01,0x7f,0x00,0x00,0x01,0x02,0x03,0x04,0x05,
//																				 0x01,0x7f,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x00,
//																				 0x01,0x7f,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x00,
//																				 0x01,0x7f,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x00,
//																				 0x01,0x7f,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x00,
//																				 0x01,0x7f,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x00,
//																				 0x01,0x7f,0x00,0x00,0x01};
//	
//	
//	unsigned char date_times[7]=	{0x07,0xe3,0x07,0x13,0x0e,0x32,0x02};		// ���� ����ʱ�� 2019-07-19 14:50:00
//		
//	//�ṹ�帳ֵ����ͬ����
//  hplc_data->dataSize=0;

//	temp_char=hplc_data->_698_frame.head =0x68;//��ʼ֡ͷ = 0x68	
//	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô,������ȽϺ�
//	
//	priv_698_state->len_position=hplc_data->dataSize;
//	hplc_data->dataSize+=2;//�����ֽڵģ�����

//	temp_char=hplc_data->_698_frame.control=CON_UTS_U|CON_U_DATA;   //������c,bit7,���䷽��λ
//	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô

//	temp_char=hplc_data->_698_frame.addr.sa=priv_698_state->addr.sa;
//	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô	
//	
//	//����������ַ
//	hplc_data->_698_frame.addr.s_addr_len=priv_698_state->addr.s_addr_len;
//	//*hplc_data->_698_frame.addr.s_addr=*priv_698_state->addr.s_addr;
//	my_strcpy(hplc_data->_698_frame.addr.s_addr,priv_698_state->addr.s_addr,0,hplc_data->_698_frame.addr.s_addr_len);
//	save_char_point_data(hplc_data,hplc_data->dataSize,hplc_data->_698_frame.addr.s_addr,hplc_data->_698_frame.addr.s_addr_len);//


//	temp_char=hplc_data->_698_frame.addr.ca=priv_698_state->addr.ca;
//	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//	
//	priv_698_state->HCS_position=hplc_data->dataSize;
//	hplc_data->dataSize+=2;//�����ֽڵ�У��λ

//	//�����ֻ�������ݣ��������ָ�����ͳһ����û����ݡ�
//	hplc_data->_698_frame.usrData_len=0;//�û����ݳ��ȹ���
//	hplc_data->_698_frame.usrData=hplc_data->priveData+(8+hplc_data->_698_frame.addr.s_addr_len);		                               

//	if(1){
//		temp_char=action_request;//����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=ActionRequest;//����һ������
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=0x05;//PIID
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	

//		save_char_point_data(hplc_data,hplc_data->dataSize,temp_arry,4);//OMD
//			
//		temp_char=Data_structure;//00 ���� Data OPTIONAL=0 ��ʾû������
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=10;//���� ��Ա����λ��
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
//		
//		temp_char=Data_octet_string;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=16;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		//������뵥�� octet-string��SIZE(16)��
//		save_char_point_data(hplc_data,hplc_data->dataSize,temp_arry,16);

//		temp_char=Data_visible_string;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=64;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		//�û�ID visible-string��SIZE(64)��
//		save_char_point_data(hplc_data,hplc_data->dataSize,temp_arry,64);

//		temp_char=Data_enum;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=1;//������  {��վ��1������������2��}
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

//		temp_char=Data_enum;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=1;//��������{���ɣ�1�� ��������2��}
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);

//		temp_char=Data_date_time_s;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		//����ʱ��
//		save_char_point_data(hplc_data,hplc_data->dataSize,date_times,7);
//			
//		temp_char=Data_visible_string;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=22;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		//·�����ʲ����  visible-string��SIZE(22)��
//		save_char_point_data(hplc_data,hplc_data->dataSize,temp_arry,22);

//		temp_char=Data_double_long_unsigned;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=00;//��������������λ��kWh�����㣺-2��double-long-unsigned
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=22;
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=02;
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=22;
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=Data_double_long;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=00;//�������  double-long����λ��kW�����㣺-4����//�����жϸ���ԭ��ת���Ϳ�����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=13;
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=32;
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		temp_char=22;
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	


//		
//		temp_char=Data_array;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//		
//		temp_char=5;//���� ����
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//		
//		
//		//���ʱ��  arrayʱ�γ�繦��	
//		
//		
//		for(int j=0;j<5;j++){
//			
//			temp_char=Data_structure;//00 ���� Data OPTIONAL=0 ��ʾû������
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//			
//			temp_char=3;//���� ��Ա����λ��
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);				

//			
//			
//			temp_char=Data_date_time_s;//���� ����
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//			
//			date_times[4]+=j*1;
//			//��ʼʱ��    date_time_s	
//			save_char_point_data(hplc_data,hplc_data->dataSize,date_times,7);	



//			temp_char=Data_date_time_s;//���� ����
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//			date_times[4]+=j*1+1;
//			//����ʱ��    date_time_s��	
//			save_char_point_data(hplc_data,hplc_data->dataSize,date_times,7);		
//				
//			temp_char=Data_double_long;//���� ����
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//			
//			temp_char=00;//�������  double-long����λ��kW�����㣺-4����//�����жϸ���ԭ��ת���Ϳ�����
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//			
//			temp_char=3;
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//			
//			temp_char=32;
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);
//			
//			temp_char=22;
//			save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);	
//				
//		}

//		temp_char=0x00;// û��ʱ���ǩ
//		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		

//		/*����������ֹͣ���*/
////		save_char_point_data(hplc_data,hplc_data->dataSize,temp_arry,4);//OMD

////		temp_char=0x00;//������=NULL
////		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
////		temp_char=0x00;// û��ʱ���ǩ
////		save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);		
//	}




//	hplc_data->_698_frame.usrData_len=hplc_data->dataSize-priv_698_state->HCS_position-2;//�û������ܳ���	,���濽���û����ݵ�usrData,���ʽ�ӻ�Ҫ���ԡ�	
//	save_char_point_usrdata(hplc_data->_698_frame.usrData,&hplc_data->_698_frame.usrData_size,hplc_data->priveData,hplc_data->dataSize-hplc_data->_698_frame.usrData_len,hplc_data->_698_frame.usrData_len);		

//	priv_698_state->FCS_position=hplc_data->dataSize;
//	hplc_data->dataSize+=2;//�����ֽڵ�У��

//		
//	temp_char=hplc_data->_698_frame.end=0x16;
//	save_char_point_data(hplc_data,hplc_data->dataSize,&temp_char,1);//���������ô


////�����Ƚṹ�帳ֵ,�����ж��ǲ�����Ҫ����
//	hplc_data->priveData[priv_698_state->len_position]=hplc_data->_698_frame.length0=(hplc_data->dataSize-2)%256;//hplc_data->size<1024ʱ

//	hplc_data->priveData[priv_698_state->len_position+1]=hplc_data->_698_frame.length1=(hplc_data->dataSize-2)/256;	

////У��ͷ
//	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the HCS_positon=%d \n",__func__,HCS_position); 	
//	result=tryfcs16(hplc_data->priveData, priv_698_state->HCS_position);
//	hplc_data->_698_frame.HCS0=hplc_data->priveData[priv_698_state->HCS_position];	
//	hplc_data->_698_frame.HCS1=hplc_data->priveData[priv_698_state->HCS_position+1];

//	//rt_kprintf("[hplc]  (%s)   link_response_package calculate the FCS_position=%d \n",__func__,FCS_position); 	
//	result=tryfcs16(hplc_data->priveData, priv_698_state->FCS_position);
//	
//	hplc_data->_698_frame.FCS0=hplc_data->priveData[priv_698_state->FCS_position];
//	hplc_data->_698_frame.FCS1=hplc_data->priveData[priv_698_state->FCS_position+1];		

//	return result;
//}
