#include "air32f10x.h"
#include "GUI.h"
#include "Lcd_Driver.h"
#include "LCD_Config.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

extern float   F_AC_V;
extern float   F_AC_I;
extern float   F_AC_P;
extern float   F_AC_E;	                                                
extern uint8_t  BAT_Q;
extern uint8_t NB_Signal_Value;

void Transform(unsigned char temp[10],float *a)
{

	sprintf((char *)temp,"%6.3f",*a);
}



void DisplayMenu(void)
{
                

	Gui_DrawFont_GBK24(143,26,BLUE,BLACK,(u8 *)"V");
delay_ms(10);
	Gui_DrawFont_GBK24(143,51,RED,BLACK,(u8 *)"A");
delay_ms(10);
	Gui_DrawFont_GBK24(143,76,YELLOW,BLACK,(u8 *)"W");
delay_ms(10);
	Gui_DrawFont_GBK24(110,101,WHITE,BLACK,(u8 *)"kWh");
delay_ms(10);
	Gui_DrawFont_GBK24(143,2,GREEN,BLACK,(u8 *)"%");
delay_ms(10);
} 	

void Clear_temp(unsigned char *temp)
{
	for(uint8_t i=0;i<10;i++)
	{
		temp[i]=0;
	}
}

void DisplayParameter(void)
{
	unsigned char temp[10]={0};
	Transform(temp,&F_AC_V);
	Gui_DrawFont_GBK24(121,27,BLUE,BLACK," ");
	Gui_DrawFont_GBK24(25,27,BLUE,BLACK,temp);
	delay_ms(10);	
	
	Clear_temp(temp);
	Transform(temp,&F_AC_I);
	Gui_DrawFont_GBK24(25,52,RED,BLACK,temp);
	delay_ms(10);
	
	Clear_temp(temp);
	if(F_AC_P<1000)
	{
	Transform(temp,&F_AC_P);
	Gui_DrawFont_GBK24(121,77,YELLOW,BLACK," ");
	Gui_DrawFont_GBK24(25,77,YELLOW,BLACK,temp);	
	}
	else
	{
		sprintf((char *)temp,"%6.2f",F_AC_P);
		Gui_DrawFont_GBK24(121,77,YELLOW,BLACK," ");
		Gui_DrawFont_GBK24(25,77,YELLOW,BLACK,temp);
	}
	
	Clear_temp(temp);
	if(F_AC_E<1000)
	{
	Transform(temp,&F_AC_E);
	Gui_DrawFont_GBK24(0,102,WHITE,BLACK,temp);	
	}
	else
	{
		sprintf((char *)temp,"%6.2f",F_AC_E);
		Gui_DrawFont_GBK24(0,102,WHITE,BLACK,temp);	
	}
	
	Clear_temp(temp);
	sprintf((char *)temp,"%3d",BAT_Q);
	Gui_DrawFont_GBK24(87,2,GREEN,BLACK,temp);
	delay_ms(10);
}

void Show_Signal(void)
{
	unsigned char temp[2]={0};
	sprintf((char *)temp,"%3d",NB_Signal_Value);
	if(NB_Signal_Value>=71)
	{
		Gui_DrawFont_GBK24(0,2,Mycolor,BLACK,temp);
		Gui_DrawFont_GBK24(49,2,Mycolor,BLACK,"%");
	}
	else
	{
		Gui_DrawFont_GBK24(0,2,ORANGE,BLACK,temp);
		Gui_DrawFont_GBK24(49,2,ORANGE,BLACK,"%");
	}		
}

void showimage(const unsigned char *p) //显示40*40 QQ图片
{
  	int i;
//		j,k; 
	unsigned char picH,picL;
	Lcd_Clear(WHITE); //清屏  
	
//	for(k=0;k<4;k++)
//	{
//	   	for(j=0;j<3;j++)
//		{	
			Lcd_SetRegion(0,0,159,127);	
//          Lcd_SetRegion(40*j+2,40*k,40*j+39,40*k+39);	//坐标设置
		    for(i=0;i<160*128;i++)
			 {	
			 	picL=*(p+i*2);	//数据低位在前
				picH=*(p+i*2+1);				
				LCD_WriteData_16Bit(picH<<8|picL);  						
			 }	
//		 }
//	}		
}
