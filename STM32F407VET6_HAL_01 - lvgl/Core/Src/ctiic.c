/**
 ****************************************************************************************************
 * @file        ctiic.c
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
#include "ctiic.h"
#include "stm32f4xx_hal.h" 
// 控制I2C速度的延时
void CT_Delay(void)
{
    delay_us(10);
}
//产生IIC起始信号
void CT_IIC_Start(void)
{
	CT_IIC_SDA(GPIO_PIN_SET);
    CT_IIC_SCL(GPIO_PIN_SET);
    CT_Delay();
    CT_IIC_SDA(GPIO_PIN_RESET);
    CT_Delay();
    CT_IIC_SCL(GPIO_PIN_RESET);
}	  
//产生IIC停止信号
void CT_IIC_Stop(void)
{ 
    CT_IIC_SCL(GPIO_PIN_RESET);
    CT_IIC_SDA(GPIO_PIN_RESET);
    CT_Delay();
    CT_IIC_SCL(GPIO_PIN_SET);
    CT_Delay();
    CT_IIC_SDA(GPIO_PIN_SET); 
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 CT_IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    
    // SDA已经是开漏输出，不需要切换模式
    CT_IIC_SDA(GPIO_PIN_SET); // 释放SDA线
    CT_Delay();
    CT_IIC_SCL(GPIO_PIN_SET);
    CT_Delay();
    
    while (CT_READ_SDA == GPIO_PIN_SET)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            CT_IIC_Stop();
            return 1;
        }
    }
    
    CT_IIC_SCL(GPIO_PIN_RESET);
    return 0;
} 
//产生ACK应答
void CT_IIC_Ack(void)
{
    CT_IIC_SCL(GPIO_PIN_RESET);
    // SDA已经是开漏输出，不需要切换模式
    CT_IIC_SDA(GPIO_PIN_RESET);
    CT_Delay();
    CT_IIC_SCL(GPIO_PIN_SET);
    CT_Delay();
    CT_IIC_SCL(GPIO_PIN_RESET);
}
//不产生ACK应答		    
void CT_IIC_NAck(void)
{
    CT_IIC_SCL(GPIO_PIN_RESET);
    // SDA已经是开漏输出，不需要切换模式
    CT_IIC_SDA(GPIO_PIN_SET);
    CT_Delay();
    CT_IIC_SCL(GPIO_PIN_SET);
    CT_Delay();
    CT_IIC_SCL(GPIO_PIN_RESET);
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void CT_IIC_Send_Byte(u8 txd)
{                        
    uint8_t t;
    // SDA已经是开漏输出，不需要切换模式
    CT_IIC_SCL(GPIO_PIN_RESET);
    
    for (t = 0; t < 8; t++)
    {
        CT_IIC_SDA((txd & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        txd <<= 1;
        CT_Delay();
        CT_IIC_SCL(GPIO_PIN_SET);
        CT_Delay();
        CT_IIC_SCL(GPIO_PIN_RESET);
        CT_Delay();
    }
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 CT_IIC_Read_Byte(unsigned char ack)
{
    uint8_t i, receive = 0;
    // SDA已经是开漏输出，不需要切换模式
    CT_IIC_SDA(GPIO_PIN_SET);	
    for (i = 0; i < 8; i++)
    {
        CT_IIC_SCL(GPIO_PIN_RESET);
        CT_Delay();
        CT_IIC_SCL(GPIO_PIN_SET);
		CT_Delay(); // 额外延时，确保SDA稳定
        receive <<= 1;
        if (CT_READ_SDA == GPIO_PIN_SET)
            receive++;
        CT_Delay();
    }
    
    if (!ack)
        CT_IIC_NAck();
    else
        CT_IIC_Ack();
    return receive;

}




























