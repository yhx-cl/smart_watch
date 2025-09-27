#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"

//跑马灯实验 -库函数版本
//STM32F4工程模板-库函数版本
//genbotter.com

int main(void)
{ 
 
	uint16_t key = 0;
	delay_init(168);		  //初始化延时函数
	LED_Init();		        //初始化LED端口
	
  /**下面是通过直接操作库函数的方式实现IO控制**/	
	KEY_Init();
	
	while(1)
	{
		key = KEY_Scan(0);
		if(key == 1)
		{
			GPIO_ResetBits(GPIOA,GPIO_Pin_6);  //LED0对应引脚GPIOA.6拉低，亮  等同LED0=0;
			GPIO_SetBits(GPIOA,GPIO_Pin_7);   //LED1对应引脚GPIOA.7拉高，灭 等同LED1=1;
		}
		else if(key == 2)
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_6);	   //LED0对应引脚GPIOA.6拉高，灭  等同LED0=1;
			GPIO_ResetBits(GPIOA,GPIO_Pin_7); //LED1对应引脚GPIOA.7拉低，亮 等同LED1=0;
		}
	}
}



 



