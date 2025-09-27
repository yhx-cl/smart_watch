#include "lcd.h"
#include "text.h"	
#include "string.h"			

//显示一个指定大小的字符
//x,y   : 字符的坐标
//ch    : 字符
//size  : 字体大小
//mode  : 显示模式
//0, 正常显示
//1, 叠加显示
//color : 字体颜色
void text_show_char(uint16_t x, uint16_t y, uint8_t ch, uint8_t size, uint8_t mode, uint16_t color)
{
    LCD_ShowChar(x, y, ch, size, mode, color);
}

//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式   
//color : 字体颜色
void Show_Str(u16 x,u16 y,u16 width,u16 height,char*str,u8 size,uint8_t mode, uint16_t color)
{					
    u8 x0 = x;
    u8 y0 = y;
    uint8_t *pstr = (uint8_t *)str; /* 指向char*型字符串首地址 */
    width += x;
    height += y;

    while (*pstr != 0)   /* 数据未结束 */
    {
        if (*pstr <= 0x7F)   /* 英文字符 */
        {
            if(x >= width){x = x0; y += size;}
            if(y >= height) break; /* 退出 */
            
            text_show_char(x, y, *pstr, size, mode, color);
            x += size/2;
            pstr++;
        }
        else
        {
            /* 跳过非ASCII字符（原中文处理部分） */
            pstr += 2;
        }
    }
} 			 		 

//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void Show_Str_Mid(u16 x,u16 y,char*str,u8 size,u8 len,u16 color)
{
    u16 strlenth = 0;
    strlenth = strlen((const char*)str);
    strlenth *= size/2;
    
    if(strlenth > len)
    {
        Show_Str(x, y, lcddev.width, lcddev.height, str, size, 1, color);
    }
    else
    {
        strlenth = (len - strlenth)/2;
        Show_Str(strlenth + x, y, lcddev.width, lcddev.height, str, size, 1, color);
    }
}