/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2017-04-10     lizhen9880      the first version
 */
#include "drv_nand.h"

#include <rtdevice.h>
#include <string.h>
#include "stdlib.h"

#ifdef RT_USING_NFTL
#include <nftl.h>
#endif
#define NAND_DEBUG    rt_kprintf
/* nandflash confg */
#define PAGES_PER_BLOCK         64
#define PAGE_DATA_SIZE          2048
#define PAGE_OOB_SIZE           64
#define ECC_SIZE                4

#define SET_NAND_CMD(_c)    do{*(volatile rt_uint8_t*)(NAND_ADDRESS|NAND_CMD) = _c;}while(0)
#define SET_NAND_ADD(_a)    do{*(volatile rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR) = _a;}while(0)    
#define SET_NAND_DAT(_d)    do{*(volatile rt_uint8_t*)NAND_ADDRESS = _d;}while(0) 
#define GET_NAND_DAT(_d)    do{_d = *(volatile rt_uint8_t*)NAND_ADDRESS;}while(0) 



static struct stm32f4_nand _device;
static rt_bool_t read_status(rt_uint8_t cmd);

NAND_HandleTypeDef NAND_Handler;    //NAND FLASH���

//NAND��ʱ
void NAND_Delay(volatile rt_uint32_t i)
{
	while(i>0)i--;
}

//�ȴ�RB�ź�Ϊĳ����ƽ
//rb:0,�ȴ�RB==0
//   1,�ȴ�RB==1
//����ֵ:0,�ɹ�
//       1,��ʱ
rt_uint8_t NAND_WaitRB(volatile rt_uint8_t rb)
{
    volatile rt_uint16_t time=0;  
	while(time<10000)
	{
		time++;
		if(NAND_RB==rb)
		{
//            NAND_DEBUG("time:%d/%d  R/B:%d\n",time,10000,rb);
				return 0;
		}
	}
//    NAND_DEBUG("timeOUT\n");
	return 1;
}
//��NAND״̬
//����ֵ:NAND״ֵ̬
//bit0:0,�ɹ�;1,����(���/����/READ)
//bit6:0,Busy;1,Ready
rt_uint8_t NAND_ReadStatus(void)
{
    volatile rt_uint8_t data=0; 
    SET_NAND_CMD(NAND_READSTA);//���Ͷ�״̬����
	data++;data++;data++;data++;data++;	//����ʱ,��ֹ-O2�Ż�,���µĴ���.
 	data=*(volatile rt_uint8_t*)NAND_ADDRESS;			//��ȡ״ֵ̬
    return data;
}
//�ȴ�NAND׼����
//����ֵ:NSTA_TIMEOUT �ȴ���ʱ��
//      NSTA_READY    �Ѿ�׼����
static rt_uint8_t wait_for_ready(void)
{
    rt_uint8_t status=0;
    volatile rt_uint32_t time=0; 
	while(1)						//�ȴ�ready
	{
		status=NAND_ReadStatus();	//��ȡ״ֵ̬
		if(status&NSTA_READY)break;
		time++;
		if(time>=0X1FFFF)return NSTA_TIMEOUT;//��ʱ
	}  
    return NSTA_READY;//׼����
}  

//��λNAND
//����ֵ:0,�ɹ�;
//    ����,ʧ��
static rt_uint8_t nand_reset(void)
{ 
    SET_NAND_CMD(NAND_RESET);//��λNAND
    if(wait_for_ready()==NSTA_READY)return 0;//��λ�ɹ�
    else return 1;								//��λʧ��
} 
//��ȡNAND FLASH��ID
//����ֵ:0,�ɹ�;
//    ����,ʧ��
rt_uint8_t NAND_ModeSet(rt_uint8_t mode)
{   
    SET_NAND_CMD(NAND_FEATURE);
    SET_NAND_ADD(0X01);
    SET_NAND_DAT(mode);
    SET_NAND_DAT(0);
    SET_NAND_DAT(0);
    SET_NAND_DAT(0);
    if(wait_for_ready()==NSTA_READY)return 0;//�ɹ�
    else return 1;								//ʧ��
}


//��ʼ��NAND FLASH
void NAND_Init(void)
{
    FSMC_NAND_PCC_TimingTypeDef ComSpaceTiming,AttSpaceTiming;
                                              
    NAND_Handler.Instance=FSMC_NAND_DEVICE;
    NAND_Handler.Init.NandBank=FSMC_NAND_BANK2;                          //NAND����BANK3��
    NAND_Handler.Init.Waitfeature=FSMC_NAND_PCC_WAIT_FEATURE_DISABLE;    //�رյȴ�����
    NAND_Handler.Init.MemoryDataWidth=FSMC_NAND_PCC_MEM_BUS_WIDTH_8;     //8λ���ݿ��
    NAND_Handler.Init.EccComputation=FSMC_NAND_ECC_DISABLE;              //��ʹ��ECC
    NAND_Handler.Init.ECCPageSize=FSMC_NAND_ECC_PAGE_SIZE_2048BYTE;      //ECCҳ��СΪ2k
    NAND_Handler.Init.TCLRSetupTime=0;                                  //����TCLR(tCLR=CLE��RE����ʱ)=(TCLR+TSET+2)*THCLK,THCLK=1/180M=5.5ns
    NAND_Handler.Init.TARSetupTime=1;                                   //����TAR(tAR=ALE��RE����ʱ)=(TAR+TSET+2)*THCLK,THCLK=1/180M=5.5n��   
   
    ComSpaceTiming.SetupTime=2;         //����ʱ��
    ComSpaceTiming.WaitSetupTime=3;     //�ȴ�ʱ��
    ComSpaceTiming.HoldSetupTime=2;     //����ʱ��
    ComSpaceTiming.HiZSetupTime=1;      //����̬ʱ��
    
    AttSpaceTiming.SetupTime=2;         //����ʱ��
    AttSpaceTiming.WaitSetupTime=3;     //�ȴ�ʱ��
    AttSpaceTiming.HoldSetupTime=2;     //����ʱ��
    AttSpaceTiming.HiZSetupTime=1;      //����̬ʱ��
    
    HAL_NAND_Init(&NAND_Handler,&ComSpaceTiming,&AttSpaceTiming); 
    nand_reset();       		        //��λNAND
//    delay_ms(100);
    wait_for_ready();
	NAND_ModeSet(4);			        //����ΪMODE4,����ģʽ 

}

//NAND FALSH�ײ�����,�������ã�ʱ��ʹ��
//�˺����ᱻHAL_NAND_Init()����
void HAL_NAND_MspInit(NAND_HandleTypeDef *hnand)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_FSMC_CLK_ENABLE();             //ʹ��FMCʱ��
    __HAL_RCC_GPIOD_CLK_ENABLE();           //ʹ��GPIODʱ��
    __HAL_RCC_GPIOE_CLK_ENABLE();           //ʹ��GPIOEʱ��
    __HAL_RCC_GPIOG_CLK_ENABLE();           //ʹ��GPIOGʱ��
    
		//��ʼ��PD6 R/B����
		GPIO_Initure.Pin=GPIO_PIN_6;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;          //����
    GPIO_Initure.Pull=GPIO_PULLUP;    			//����          
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;         //����
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);
	   
	//��ʼ��PD7 NCE2����
    GPIO_Initure.Pin=GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //����
    GPIO_Initure.Pull=GPIO_NOPULL;    			//����          
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;         //����
		GPIO_Initure.Alternate=GPIO_AF12_FSMC;       //����ΪFMC
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);  
	
    //��ʼ��PD0,1,4,5,11,12,14,15
    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|\
                     GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_Initure.Pull=GPIO_NOPULL;              
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);

    //��ʼ��PE7,8,9,10
    GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
}



//��NAND״̬
//����ֵ:NAND״ֵ̬
//bit0:0,�ɹ�;1,����(���/����/READ)
//bit6:0,Busy;1,Ready
static rt_bool_t read_status(rt_uint8_t cmd)
{
    volatile rt_uint8_t value=0; 
    SET_NAND_CMD(NAND_READSTA);//���Ͷ�״̬����
	value++;value++;value++;value++;value++;	//����ʱ,��ֹ-O2�Ż�,���µĴ���.
 	value=*(volatile rt_uint8_t*)NAND_ADDRESS;			//��ȡ״ֵ̬
    switch (cmd)
    {
    case NAND_WRITE0:
    case NAND_ERASE1:
        if (value & 0x01)           /* Erase/Program failure(1) or pass(0) */
            return (RT_FALSE);
        else
            return (RT_TRUE);

    case NAND_AREA_TRUE1:            /* bit 5 and 6, Read busy(0) or ready(1) */
        return (RT_TRUE);

    default:
        break;
    }

    return (RT_FALSE);
}

static rt_err_t nand_MT29F4G08_readid(struct rt_mtd_nand_device *device)
{
    rt_uint32_t id;  
    SET_NAND_CMD(NAND_READID); //���Ͷ�ȡID����
    SET_NAND_ADD(0X00);
    GET_NAND_DAT(_device.id[0]);//IDһ����5���ֽ�
    GET_NAND_DAT(_device.id[1]);
    GET_NAND_DAT(_device.id[2]);
    GET_NAND_DAT(_device.id[3]);
    GET_NAND_DAT(_device.id[4]);

    //þ���NAND FLASH��IDһ��5���ֽڣ�����Ϊ�˷�������ֻȡ4���ֽ����һ��32λ��IDֵ
    //����NAND FLASH�������ֲᣬֻҪ��þ���NAND FLASH����ôһ���ֽ�ID�ĵ�һ���ֽڶ���0X2C
    //�������ǾͿ����������0X2C��ֻȡ�������ֽڵ�IDֵ��
    id=((rt_uint32_t)_device.id[1])<<24|((rt_uint32_t)_device.id[2])<<16|((rt_uint32_t)_device.id[3])<<8|_device.id[4];
    
    rt_kprintf("\nNAND ID: 0x%08X\n", id);
    return RT_EOK;
    
}
static rt_err_t nand_datacorrect(uint32_t generatedEcc, uint32_t readEcc, uint8_t *data)
{
#define ECC_MASK28    0x0FFFFFFF          /* 28 valid ECC parity bits. */
#define ECC_MASK      0x05555555          /* 14 ECC parity bits.       */

    rt_uint32_t count, bitNum, byteAddr;
    rt_uint32_t mask;
    rt_uint32_t syndrome;
    rt_uint32_t eccP;                            /* 14 even ECC parity bits. */
    rt_uint32_t eccPn;                           /* 14 odd ECC parity bits.  */

    syndrome = (generatedEcc ^ readEcc) & ECC_MASK28;

    if (syndrome == 0)
        return (RT_MTD_EOK);                  /* No errors in data. */

    eccPn = syndrome & ECC_MASK;              /* Get 14 odd parity bits.  */
    eccP  = (syndrome >> 1) & ECC_MASK;       /* Get 14 even parity bits. */

    if ((eccPn ^ eccP) == ECC_MASK)           /* 1-bit correctable error ? */
    {
        bitNum = (eccP & 0x01) |
                 ((eccP >> 1) & 0x02) |
                 ((eccP >> 2) & 0x04);
        NAND_DEBUG("ECC bit %d\n",bitNum);
        byteAddr = ((eccP >> 6) & 0x001) |
                   ((eccP >> 7) & 0x002) |
                   ((eccP >> 8) & 0x004) |
                   ((eccP >> 9) & 0x008) |
                   ((eccP >> 10) & 0x010) |
                   ((eccP >> 11) & 0x020) |
                   ((eccP >> 12) & 0x040) |
                   ((eccP >> 13) & 0x080) |
                   ((eccP >> 14) & 0x100) |
                   ((eccP >> 15) & 0x200) |
                   ((eccP >> 16) & 0x400) ;

        data[ byteAddr ] ^= 1 << bitNum;

        return RT_MTD_EOK;
    }

    /* Count number of one's in the syndrome. */
    count = 0;
    mask  = 0x00800000;
    while (mask)
    {
        if (syndrome & mask)
            count++;
        mask >>= 1;
    }

    if (count == 1)           /* Error in the ECC itself. */
        return RT_MTD_EECC;

    return -RT_MTD_EECC;       /* Unable to correct data. */

#undef ECC_MASK
#undef ECC_MASK24
}
static rt_err_t nand_MT29F4G08_readpage(struct rt_mtd_nand_device *device,
                                     rt_off_t                   page,
                                     rt_uint8_t                *data,
                                     rt_uint32_t                data_len,
                                     rt_uint8_t                *spare,
                                     rt_uint32_t                spare_len)
{
    rt_uint32_t index;
    rt_uint32_t gecc, recc;
    rt_uint8_t tmp[4];
    rt_err_t result;
    rt_uint32_t i;
    page = page + device->block_start * device->pages_per_block;
    if (page/device->pages_per_block > device->block_end)
    {
        return -RT_MTD_EIO;
    }

    result = RT_MTD_EOK;
    rt_mutex_take(&_device.lock, RT_WAITING_FOREVER);

    if (data && data_len)
    {
        SET_NAND_CMD(NAND_AREA_A); //���͵�ַ
        SET_NAND_ADD((rt_uint8_t)(0&0xFF));
        SET_NAND_ADD((rt_uint8_t)(0>>8));
        SET_NAND_ADD((rt_uint8_t)(page & 0xFF));
        SET_NAND_ADD((rt_uint8_t)(page >> 8));
//        SET_NAND_ADD((rt_uint8_t)(page >> 16));
        SET_NAND_CMD(NAND_AREA_TRUE1);
        //�������д����ǵȴ�R/B���ű�Ϊ�͵�ƽ����ʵ��Ҫ����ʱ���õģ��ȴ�NAND����R/B���š���Ϊ������ͨ��
        //��STM32��NWAIT����(NAND��R/B����)����Ϊ��ͨIO��������ͨ����ȡNWAIT���ŵĵ�ƽ���ж�NAND�Ƿ�׼��
        //�����ġ����Ҳ����ģ��ķ������������ٶȺܿ��ʱ���п���NAND��û���ü�����R/B��������ʾNAND��æ
        //��״̬���Ͷ�ȡ��R/B����,���ʱ��϶������ģ���ʵ��ȷʵ�ǻ����!
        NAND_WaitRB(0);			//�ȴ�RB=0 
        //����2�д����������ж�NAND�Ƿ�׼���õ�
        NAND_WaitRB(1);			//�ȴ�RB=1 
        FSMC_NAND_ECC_Enable(NAND_Handler.Instance,FSMC_NAND_BANK2);
        
        for (i = 0; i < data_len; i ++)
        {
            GET_NAND_DAT(data[i]);  
        }
        gecc = FSMC_NAND_GetECC(NAND_Handler.Instance,(uint32_t*)&gecc,FSMC_NAND_BANK2,10);

        if (data_len == PAGE_DATA_SIZE)
        {
            for (index = 0; index < ECC_SIZE; index ++)
            {
                GET_NAND_DAT(tmp[index]);
            }
                
            if (spare && spare_len)
            {
                for (i = ECC_SIZE; i < spare_len; i ++)
                {
                    GET_NAND_DAT(spare[i]);      
                }
                rt_memcpy(spare, tmp , ECC_SIZE);
            }

            recc   = (tmp[3] << 24) | (tmp[2] << 16) | (tmp[1] << 8) | tmp[0];

            if (recc != 0xFFFFFFFF && gecc != 0)
                result = nand_datacorrect(gecc, recc, data);

            if (result != RT_MTD_EOK)
                NAND_DEBUG("page: %d, gecc %X, recc %X>",page, gecc, recc);

            goto _exit;
        }
    }

    if (spare && spare_len)
    {
        SET_NAND_CMD(NAND_AREA_A); //���͵�ַ
        SET_NAND_ADD((rt_uint8_t)(PAGE_DATA_SIZE&0xFF));
        SET_NAND_ADD((rt_uint8_t)(PAGE_DATA_SIZE>>8));
        SET_NAND_ADD((rt_uint8_t)(page & 0xFF));
        SET_NAND_ADD((rt_uint8_t)(page >> 8));
        SET_NAND_ADD((rt_uint8_t)(page >> 16));
        SET_NAND_CMD(NAND_AREA_TRUE1);
        
        //�������д����ǵȴ�R/B���ű�Ϊ�͵�ƽ����ʵ��Ҫ����ʱ���õģ��ȴ�NAND����R/B���š���Ϊ������ͨ��
        //��STM32��NWAIT����(NAND��R/B����)����Ϊ��ͨIO��������ͨ����ȡNWAIT���ŵĵ�ƽ���ж�NAND�Ƿ�׼��
        //�����ġ����Ҳ����ģ��ķ������������ٶȺܿ��ʱ���п���NAND��û���ü�����R/B��������ʾNAND��æ
        //��״̬���Ͷ�ȡ��R/B����,���ʱ��϶������ģ���ʵ��ȷʵ�ǻ����!
        NAND_WaitRB(0);			//�ȴ�RB=0 
        //����2�д����������ж�NAND�Ƿ�׼���õ�
        NAND_WaitRB(1);			//�ȴ�RB=1 

        for (i = 0; i < spare_len; i ++)
        {
            GET_NAND_DAT(spare[i]);
        }

    }
_exit:
    rt_mutex_release(&_device.lock);

    return (result);
}

static rt_err_t nand_MT29F4G08_writepage(struct rt_mtd_nand_device *device,
                                      rt_off_t                   page,
                                      const rt_uint8_t          *data,
                                      rt_uint32_t                data_len,
                                      const rt_uint8_t          *spare,
                                      rt_uint32_t                spare_len)
{
    rt_err_t result;
    rt_uint32_t gecc;
    rt_uint32_t i;
    
    page = page + device->block_start * device->pages_per_block;
    if (page/device->pages_per_block > device->block_end)
    {
        return -RT_MTD_EIO;
    }

    result = RT_MTD_EOK;
    rt_mutex_take(&_device.lock, RT_WAITING_FOREVER);

    if (data && data_len)
    {
        SET_NAND_CMD(NAND_WRITE0); //���͵�ַ
        SET_NAND_ADD((rt_uint8_t)(0&0xFF));
        SET_NAND_ADD((rt_uint8_t)(0>>8));
        SET_NAND_ADD((rt_uint8_t)(page & 0xFF));
        SET_NAND_ADD((rt_uint8_t)(page >> 8));
//        SET_NAND_ADD((rt_uint8_t)(page >> 16));
        
        FSMC_NAND_ECC_Enable(NAND_Handler.Instance,FSMC_NAND_BANK2);
        
        for (i = 0; i < data_len; i ++)
        {
            SET_NAND_DAT(data[i]);     
        }
        gecc = FSMC_NAND_GetECC(NAND_Handler.Instance,(uint32_t*)&gecc,FSMC_NAND_BANK2,10);
        
        FSMC_NAND_ECC_Disable(NAND_Handler.Instance,FSMC_NAND_BANK2);

        if (data_len == PAGE_DATA_SIZE)
        {
            SET_NAND_DAT((uint8_t)gecc);
            SET_NAND_DAT((uint8_t)(gecc >> 8));
            SET_NAND_DAT((uint8_t)(gecc >> 16));
            SET_NAND_DAT((uint8_t)(gecc >> 24));

            if (spare && spare_len)
            {
                for (i = ECC_SIZE; i < spare_len; i ++)
                {
                    SET_NAND_DAT(spare[i]);     
                }
            }
            
        }
        SET_NAND_CMD(NAND_WRITE_TURE1);
        if(wait_for_ready()!=NSTA_READY)
        {
            nand_reset();
            result = -RT_MTD_EIO;//ʧ��
					
						rt_kprintf("waite for ready time out.\r\n");
        }
        goto _exit;
    }

    if (spare && spare_len)
    {
        SET_NAND_CMD(NAND_WRITE0); //���͵�ַ
        SET_NAND_ADD((rt_uint8_t)(PAGE_DATA_SIZE&0xFF));
        SET_NAND_ADD((rt_uint8_t)(PAGE_DATA_SIZE>>8));
        SET_NAND_ADD((rt_uint8_t)(page & 0xFF));
        SET_NAND_ADD((rt_uint8_t)(page >> 8));
//        SET_NAND_ADD((rt_uint8_t)(page >> 16));
        
        if (spare && spare_len)
        for (i = ECC_SIZE; i < spare_len; i ++)
        {
            SET_NAND_DAT(spare[i]);
        }
        SET_NAND_CMD(NAND_WRITE_TURE1);
        if(wait_for_ready()!=NSTA_READY)
        {
            nand_reset();
            result = -RT_MTD_EIO;//ʧ��
						rt_kprintf("waite for ready time out.\r\n");
        }
    }

_exit:
    rt_mutex_release(&_device.lock);

    return (result);

}

static rt_err_t nand_MT29F4G08_eraseblock(struct rt_mtd_nand_device *device,
                                       rt_uint32_t block)
{
    unsigned int blockPage;
    rt_err_t result;
    /* add the start blocks */
    block = block + device->block_start;
    blockPage = (block << 6);
    result = RT_MTD_EOK;
    
    rt_mutex_take(&_device.lock, RT_WAITING_FOREVER);

    SET_NAND_CMD(NAND_ERASE0); //���͵�ַ
    SET_NAND_ADD((rt_uint8_t)blockPage);
    SET_NAND_ADD((rt_uint8_t)(blockPage>>8));
    SET_NAND_ADD((rt_uint8_t)(blockPage>>16));
    SET_NAND_CMD(NAND_ERASE1);
	if(wait_for_ready()!=NSTA_READY)
    {
        nand_reset();
        result = -RT_MTD_EIO;//ʧ��
    }
    rt_mutex_release(&_device.lock);
    return result;
}

static rt_err_t nand_MT29F4G08_pagecopy(struct rt_mtd_nand_device *device,
                                     rt_off_t                   src_page,
                                     rt_off_t                   dst_page)
{
    rt_err_t result = RT_MTD_EOK;
    rt_uint32_t source_block=0,dest_block=0; 
    src_page = src_page + device->block_start * device->pages_per_block;
    dst_page = dst_page + device->block_start * device->pages_per_block;
    //�ж�Դҳ��Ŀ��ҳ�Ƿ���ͬһ��plane��
    source_block=src_page/device->pages_per_block;
    dest_block=dst_page/device->pages_per_block;
    if((source_block%2)!=(dest_block%2))return RT_MTD_ESRC;	//����ͬһ��plane�� 
    
    SET_NAND_CMD(NAND_MOVEDATA_CMD0);//��������0X00
    SET_NAND_ADD((rt_uint8_t)(0&0xFF));    //����Դҳ��ַ
    SET_NAND_ADD((rt_uint8_t)(0>>8));
    SET_NAND_ADD((rt_uint8_t)(src_page & 0xFF));
    SET_NAND_ADD((rt_uint8_t)(src_page >> 8));
//    SET_NAND_ADD((rt_uint8_t)(src_page >> 16));
    SET_NAND_CMD(NAND_MOVEDATA_CMD1);//��������0X35 

    //�������д����ǵȴ�R/B���ű�Ϊ�͵�ƽ����ʵ��Ҫ����ʱ���õģ��ȴ�NAND����R/B���š���Ϊ������ͨ��
    //��STM32��NWAIT����(NAND��R/B����)����Ϊ��ͨIO��������ͨ����ȡNWAIT���ŵĵ�ƽ���ж�NAND�Ƿ�׼��
    //�����ġ����Ҳ����ģ��ķ������������ٶȺܿ��ʱ���п���NAND��û���ü�����R/B��������ʾNAND��æ
    //��״̬��������ǾͶ�ȡ��R/B����,���ʱ��϶������ģ���ʵ��ȷʵ�ǻ����!���Ҳ���Խ���������
    //���뻻����ʱ����,ֻ������������Ϊ��Ч������û������ʱ������
	result=NAND_WaitRB(0);			//�ȴ�RB=0 
	if(result)return -RT_MTD_EIO;	//��ʱ�˳�
    //����2�д����������ж�NAND�Ƿ�׼���õ�
	result=NAND_WaitRB(1);			//�ȴ�RB=1 
    if(result)return -RT_MTD_EIO;	//��ʱ�˳� 
    
    SET_NAND_CMD(NAND_MOVEDATA_CMD2);//��������0X85
    SET_NAND_ADD((rt_uint8_t)(0&0xFF)); //����Ŀ��ҳ��ַ
    SET_NAND_ADD((rt_uint8_t)(0>>8));
    SET_NAND_ADD((rt_uint8_t)(dst_page & 0xFF));
    SET_NAND_ADD((rt_uint8_t)(dst_page >> 8));
//    SET_NAND_ADD((rt_uint8_t)(dst_page >> 16));
    SET_NAND_CMD(NAND_MOVEDATA_CMD3);//��������0X10
 
    if(wait_for_ready()!=NSTA_READY)
    {
        nand_reset();
        return -RT_MTD_EIO;//ʧ��
    }

    return RT_MTD_EOK;


}

static rt_err_t nand_MT29F4G08_checkblock(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    return (RT_MTD_EOK);
}

static rt_err_t nand_MT29F4G08_markbad(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    return (RT_MTD_EOK);
}

static const struct rt_mtd_nand_driver_ops ops =
{
    nand_MT29F4G08_readid,
    nand_MT29F4G08_readpage,
    nand_MT29F4G08_writepage,
    nand_MT29F4G08_pagecopy,
    nand_MT29F4G08_eraseblock,
//	#if defined(RT_USING_DFS_UFFS) && !defined(RT_UFFS_USE_CHECK_MARK_FUNCITON)
//    RT_NULL,
//    RT_NULL,
//	#else
    nand_MT29F4G08_checkblock,
    nand_MT29F4G08_markbad,
//	#endif
};
static struct rt_mtd_nand_device _partition[1];

int nand_MT29F4G08_hw_init(void)
{
   NAND_Init();
    rt_mutex_init(&_device.lock, "nand", RT_IPC_FLAG_FIFO);
    /* register nand0 */
    _partition[0].page_size       = PAGE_DATA_SIZE;
    _partition[0].pages_per_block = PAGES_PER_BLOCK;
    _partition[0].plane_num       = 1;
    _partition[0].oob_size        = PAGE_OOB_SIZE;
//		_partition[0].oob_free				= 60;
    _partition[0].oob_free        = PAGE_OOB_SIZE - ((PAGE_DATA_SIZE) * 3 / 256);
		_partition[0].block_total     = 1024;
    _partition[0].block_start     = 0;
    _partition[0].block_end       = 1023;

    _partition[0].ops             = &ops;

    rt_mtd_nand_register_device("nand0", &_partition[0]);
    nand_MT29F4G08_readid(&_partition[0]);

    return RT_EOK;
}
INIT_BOARD_EXPORT(nand_MT29F4G08_hw_init);

void nerase_all(void)
{
    rt_uint32_t index;
    struct rt_mtd_nand_device *device;

    device = &_partition[0];
    for (index = 0; index < device->block_total; index ++)
    {
        nand_MT29F4G08_eraseblock(device, index);
    }
}

//void nread(rt_uint32_t partion, rt_uint32_t page)
void nread(int argc, char**argv)
{
    int i;
    rt_uint8_t spare[64];
    rt_uint8_t *data_ptr;
		rt_uint32_t partion,page;
    struct rt_mtd_nand_device *device;

		partion = strtol(argv[1],NULL,10);
		page = strtol(argv[2],NULL,10);
	
    if (partion >= 1)
        return;
    device = &_partition[partion];
    data_ptr = (rt_uint8_t*) rt_malloc(PAGE_DATA_SIZE);
    if (data_ptr == RT_NULL)
    {
        rt_kprintf("no memory\n");
        return;
    }
    rt_memset(spare, 0, sizeof(spare));
    rt_memset(data_ptr, 0, PAGE_DATA_SIZE);
    nand_MT29F4G08_readpage(device, page, data_ptr, PAGE_DATA_SIZE, spare, sizeof(spare));
    for (i = 0; i < 512; i ++)
    {
        rt_kprintf("0x%X,",data_ptr[i]);
    }
    rt_kprintf("\n spare\n");
    for (i = 0; i < sizeof(spare); i ++)
    {
        rt_kprintf("0x%X,",spare[i]);
    }
    rt_kprintf("\n\n");

    /* release memory */
    rt_free(data_ptr);
}

//void nwrite(rt_uint32_t partion, int page)
void nwrite(int argc, char**argv)
{
	int i;
	rt_uint8_t spare[64];
	rt_uint8_t *data_ptr;
	rt_uint32_t partion,page;
	struct rt_mtd_nand_device *device;

	partion = strtol(argv[1],NULL,10);
	page = strtol(argv[2],NULL,10);

	if (partion >= 1)
	{
				return;
	}

	device = &_partition[partion];

	data_ptr = (rt_uint8_t*) rt_malloc (PAGE_DATA_SIZE);
	if (data_ptr == RT_NULL)
	{
			rt_kprintf("no memory.\n");
			return;
	}
    /* Need random data to test ECC */
  for (i = 0; i < PAGE_DATA_SIZE; i ++)
	{
        data_ptr[i] = 0x22;
	}
	rt_memset(spare, 0x11, sizeof(spare));
	nand_MT29F4G08_writepage(device, page, data_ptr, PAGE_DATA_SIZE, spare, sizeof(spare));
	rt_free(data_ptr);
}

//void ncopy(rt_uint32_t partion, int src, int dst)
void ncopy(int argc, char**argv)
{
	struct rt_mtd_nand_device *device;
	rt_uint32_t partion,src,dst;

	partion = strtol(argv[1],NULL,10);
	src = strtol(argv[2],NULL,10);
	dst = strtol(argv[3],NULL,10);

	if (partion >= 3)
			return;
	device = &_partition[partion];
	nand_MT29F4G08_pagecopy(device, src, dst);
}

void nerase(int partion, int block)
{
  struct rt_mtd_nand_device *device;

  if (partion >= 3)
	{
    return;
	}
	device = &_partition[partion];
	nand_MT29F4G08_eraseblock(device, block);
}

void nid(void)
{
    nand_MT29F4G08_readid(0);
}

MSH_CMD_EXPORT(nid, nand id);
MSH_CMD_EXPORT(ncopy, nand copy page);
MSH_CMD_EXPORT(nerase, nand erase a block of one partiton);
MSH_CMD_EXPORT(nwrite, nand write page);
MSH_CMD_EXPORT(nread, nand read page);

MSH_CMD_EXPORT(nerase_all, erase all nandflash);






