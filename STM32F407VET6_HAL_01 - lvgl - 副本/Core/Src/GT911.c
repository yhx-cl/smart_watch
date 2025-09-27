/**
 ****************************************************************************************************
 * @file        gt911.c
 * @author      EYA-DISPLAY
 * @version     V2.0
 * @date        2022-04-28
 * @brief       液晶屏驱动Demo
 * @license     Copyright (c) 2022-2032, 亦亞徽科技集团(广东)
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:EYA-ETmcu开发板
 * 公司网址:www.eya-display.com
 *
 ****************************************************************************************************
 **/
#include "GT911.h"
#include "touch.h"
#include "ctiic.h"
#include "string.h" 
#include "lcd.h" 
  
//向GT911写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
u8 GT911_WR_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i;
	u8 ret=0;
	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   	//发送写命令 	 
	CT_IIC_Wait_Ack();
	CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//发数据
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					//产生一个停止条件	    
	return ret; 
}
//从GT911读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度			  
void GT911_RD_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   //发送写命令 	 
	CT_IIC_Wait_Ack();
 	CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(GT_CMD_RD);   //发送读命令		   
	CT_IIC_Wait_Ack();
	
	for(i=0;i<len;i++)
	{	   
		
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据
	} 
    CT_IIC_Stop();//产生一个停止条件    
} 
//初始化GT911触摸屏
//返回值:0,初始化成功;1,初始化失败 
u8 GT911_Init(void)
{
    u8 temp[5];
	
	
    GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOC_CLK_ENABLE();
    // 先将INT引脚配置为输出模式
    GPIO_InitStruct.Pin = GT_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GT_INT_GPIO_Port, &GPIO_InitStruct);

	printf("GT_INT out_pp init !\r\n");
    // 设置INT引脚电平，根据地址选择：高电平选择0x14，低电平选择0x5D
    // 如果不确定，可以尝试两种电平
    HAL_GPIO_WritePin(GT_INT_GPIO_Port, GT_INT_Pin, GPIO_PIN_SET); // 尝试高电平，地址0x14
    //HAL_GPIO_WritePin(GT_INT_GPIO_Port, GT_INT_Pin, GPIO_PIN_RESET); // 或低电平，地址0x5D

    HAL_GPIO_WritePin(GT_RST_GPIO_Port, GT_RST_Pin, GPIO_PIN_SET);

    // 复位序列
    HAL_GPIO_WritePin(GT_RST_GPIO_Port, GT_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GT_RST_GPIO_Port, GT_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN; // 或GPIO_PULLUP
    HAL_GPIO_Init(GT_INT_GPIO_Port, &GPIO_InitStruct);
	
	printf("GT_INT input init !\r\n");
	
    HAL_Delay(100);
	GT911_RD_Reg(GT_PID_REG,temp,4);//读取产品ID
	temp[4] = 0;
	printf("CTP ID:%s\r\n",temp);	//打印ID
	printf("CTP ID Bytes: %02X %02X %02X %02X\r\n", 
           temp[0], temp[1], temp[2], temp[3]);
	if(strcmp((char*)temp,"911")==0)//ID==911
	{
		temp[0]=0X02;			
		GT911_WR_Reg(GT_CTRL_REG,temp,1);//软复位GT911
		printf("GT911_WR_Reg1!\r\n");
		GT911_RD_Reg(GT_CFGS_REG,temp,1);//读取GT_CFGS_REG寄存器
		printf("GT911_DR_Reg1!\r\n");
		if(temp[0]<0X60)//默认版本比较低,需要更新flash配置
		{
			printf("Default Ver:%d\r\n",temp[0]);
		}
		HAL_Delay(10);
		temp[0]=0X00;	 
		GT911_WR_Reg(GT_CTRL_REG,temp,1);//结束复位
		printf("GT911_WR_Reg2!\r\n");		
		return 0;
	} 
	return 1;
}
const u16 GT911_TPX_TBL[5]={GT_TP1_REG,GT_TP2_REG,GT_TP3_REG,GT_TP4_REG,GT_TP5_REG};
//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 GT911_Scan(u8 mode)
{
	u8 buf[4];
	u8 i=0;
	u8 res=0;
	u8 temp;
	static u8 t=0;//控制查询间隔,从而降低CPU占用率   
	t++;
	if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
	{
		GT911_RD_Reg(GT_GSTID_REG,&mode,1);//读取触摸点的状态 
		//printf("mode:0x%x\r\n",mode);
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);//将点的个数转换为1的位数,匹配tp_dev.sta定义 
			tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES; 
			for(i=0;i<5;i++)
			{
				if(tp_dev.sta&(1<<i))	//触摸有效?
				{
					GT911_RD_Reg(GT911_TPX_TBL[i],buf,4);	//读取XY坐标值
					if(lcddev.dir==0)	//竖屏
					{
						tp_dev.x[i]=((u16)buf[1]<<8)+buf[0];
						tp_dev.y[i]=(((u16)buf[3]<<8)+buf[2]);
					}else //横屏
					{
						tp_dev.y[i]=((u16)buf[1]<<8)+buf[0];
						tp_dev.x[i]=lcddev.width-(((u16)buf[3]<<8)+buf[2]);
					}  
					printf("%d %d %d %d\r\n",buf[0],buf[1],buf[2],buf[3]);
					// printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);//串口打印坐标，用于调试
				}			
			} 
			res=1;
			if(tp_dev.x[0]==0 && tp_dev.y[0]==0)mode=0;	//读到的数据都是0,则忽略此次数据
			t=0;		//触发一次,则会最少连续监测10次,从而提高命中率
		}
 		if(mode&0X80&&((mode&0XF)<6))
		{
			temp=0;
			GT911_WR_Reg(GT_GSTID_REG,&temp,1);//清标志 		
		}
	}
	if((mode&0X8F)==0X80)//无触摸点按下
	{ 
		if(tp_dev.sta&TP_PRES_DOWN)	//之前是被按下的
		{
			tp_dev.sta&=~(1<<7);	//标记按键松开
		}else						//之前就没有被按下
		{ 
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
			tp_dev.sta&=0XE0;	//清除点有效标记	
		}	 
	} 	
	if(t>240)t=10;//重新从10开始计数
	return res;
}


























