/**
 ****************************************************************************************************
 * @file        touch.c
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
#include "touch.h" 
#include "lcd.h"
#include "stm32f4xx_hal.h"
#include "stdlib.h"
#include "math.h"
#include "GT911.h"
#include "text.h" 

extern u8 AUTO;
u16 x0temp,y0temp;
//触摸相关参数
_m_tp_dev tp_dev=
{
	TP_Init,
	TP_Scan,
	0,
	0, 
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};					
// 提示字符串
u8* const TP_REMIND_MSG_TBL = "Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";

				  
//////////////////////////////////////////////////////////////////////////////////		  

//触摸屏初始化  		    
//返回值:1,没有进行校准
//       1,进行过校准
u8 TP_Init(void)
{	
	if(GT911_Init()==0)
	{
		tp_dev.scan=GT911_Scan;       	//扫描函数指向GT911触摸屏扫描
		tp_dev.touchtype |= 0X80;       /* 标记电容屏 */
		return 1;
	}
	return 0;
}
//触摸按键扫描
//tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 TP_Scan(u8 tp)
{			   
	return GT911_Scan(tp);//返回当前的触屏状态
}	  

//判断触点是不是在指定区域之内
//(x,y):起始坐标
//xlen,ylen:在x,y方向上的偏移长度
//返回值 :1,在该区域内.0,不在该区域内.
u8 Is_In_Area(u16 x,u16 y,u16 xlen,u16 ylen)
{
	tp_dev.scan(0);//扫描
	if(tp_dev.sta&TP_PRES_DOWN)//有按键被按下
		{
			HAL_Delay(20);//必要的延时,否则老认为有按键按下.
			if(tp_dev.x[0]<=(x+xlen)&&tp_dev.x[0]>=x&&tp_dev.y[0]<=(y+ylen)&&tp_dev.y[0]>=y)
			{
				return 1;
			}
			else return 0;
		}
	 return 0;
} 


//坐标地址坐标
u16 PositionH[4*2]={
0,LCD_WIDTH-40,40,40,     //第一个按键位置： 起始位置X，起始位置Y，宽度，长度
LCD_HEIGHT-40,LCD_WIDTH-40,40,40,    //第二个按键位置： 起始位置X，起始位置Y，宽度，长度
};
u16 PositionV[4*2]={
0,LCD_HEIGHT-40,40,40,    //第一个按键位置： 起始位置X，起始位置Y，宽度，长度
LCD_WIDTH-40,LCD_HEIGHT-40,40,40,    //第二个按键位置： 起始位置X，起始位置Y，宽度，长度
};

u16 touchcount=0;
u8 lock=0;
//触摸位置扫描
u8 py_get_keynum(void)
{
	u8 KeyNO;
	u16 X,Y,WIDTH,HEIGHT;
	u8 i;
	static u8 key_x=0;//0,没有任何按键按下;1~9,1~9号按键按下
	u8 key=0;
	u16 *Pos;//按键坐标
	tp_dev.scan(0); 		 
	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{	
		 if(lcddev.dir==0) Pos = PositionV; //竖屏
		 else              Pos = PositionH; //横屏 
		 KeyNO=2;//只有两个按键
		//扫描按键位置
		for (i = 0; i < KeyNO; i++)
		{
				X = Pos[i*4 + 0];
				Y = Pos[i*4 + 1];
			  WIDTH = Pos[i*4 + 2]; 
			  HEIGHT = Pos[i*4 + 3]; //获取长宽坐标
			
				if(tp_dev.x[0]<(X+WIDTH)&&tp_dev.x[0]>(X)&&tp_dev.y[0]<(Y+HEIGHT)&&tp_dev.y[0]>(Y)) { key=i+1; break;}//找到按键位置，跳出循环
		 }	
			if(key) //检查值的正确性
			{	  
          if((touchcount++>500)&&(key_x!=0))//长按了	
					{
						key_x=key;
					}else//短按
          {					
						if(key_x==key)key=0;
						else          key_x=key;
					}
			}
		}else if(key_x) //弹起 改用LOCK作为判断弹起的坐标，防止移动改变坐标
	  {
			  touchcount=0;
		    key_x=0;
		    lock=0;//释放按键锁
 
	  } 
	 return key;
}


//清空屏幕并在右上角显示"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);	//清屏   
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST",BLUE);//显示清屏区域
	Show_Str(0,lcddev.height-20,50,16,"return",16,1,BLUE);
}

//显示返回和继续
void ShowUI(u8 i)
{
	if(i>1) Show_Str(0,lcddev.height-20,50,16,"return",16,1,POINT_COLOR); //不在第一个画面才需要返回
	        Show_Str(lcddev.width-40,lcddev.height-20,50,16,"continue",16,1,POINT_COLOR);
}

////////////////////////////////////////////////////////////////////////////////
//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 
//5个触控点的颜色												 
const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
//电容触摸屏测试函数
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//记录最后一次的数据 
	lastpos[0][0]=0;
	lastpos[0][1]=0;
	while(1)
	{
		tp_dev.scan(0);    //从扫描函数取X Y坐标 
		for(t=0;t<CT_MAX_TOUCH;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
//					printf("x:%d\r\n",tp_dev.x[0]);	
//					printf("y:%d\r\n",tp_dev.y[0]);	 
					
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//画线
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-40)&&tp_dev.y[t]<40)
					{
						Load_Drow_Dialog();//清除
					}else if(tp_dev.x[t]<(0+24)&&tp_dev.x[t]>(0)&&tp_dev.y[t]<(lcddev.height)&&tp_dev.y[t]>(lcddev.height-24))//左下角
					{
						LCD_Clear(WHITE);
						ShowUI(1);
						return; //返回
					}
				}
			}else lastpos[t][0]=0XFFFF;	
		}
		HAL_Delay(5);i++;
	}	
}
