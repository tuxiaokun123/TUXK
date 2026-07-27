#include "air32f10x.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	
#include "timer.h"
#include "HLW8110.h"
#include "test.h"
#include "LCD_Displayinformation.h"
#include "relay.h"
#include "Power_EN.h"
#include "exti.h"
#include "Battery.h"
#include "GUI.h"
#include "Lcd_Driver.h"
#include "LCD_Config.h"
#include "E_Meter_CRT.h"
#include "BC260Y.h"
#include "IWDG.h"
#include "String.h"
#include "AIR32.h"
extern uint8_t B_ReadData_HLW8110;
extern uint8_t BC260Y_NOTIFY_CNT;
extern uint8_t onenet_ok;
extern uint8_t u8_RxBuf1[50];
extern uint8_t Rx_Data_ING1;
extern uint8_t u8_RX_Index1;
extern uint8_t NB_MIPLEXECUTERSP_CMD[30];
extern char onenet_order[10];
uint16_t Timer;
uint16_t Timer_residue;
uint16_t Timer_residue_BAK;
uint8_t NOTIFY_state;

int main(void)
{
	Stm32_Clock_Init(9);//系统时钟设置
	
    delay_init(72);
	
	Power_ON();
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	
	Shut_Down_Init(); 
	while(pA8==0);	
	delay_ms(50);
	E_Clear_Init();
	EXTIX_Init();
	HLW8112_INT_Init();

	Adc_Init();
	battery_EN();
	
#if DEBUG	
	uart3_init();
#endif

	uart1_init(9600);
	delay_ms (50);
	
	uart2_init(18,38400);
	Init_HLW8110();
	
	Lcd_Init();
	LCD_LED_SET;
	Lcd_Clear(BLACK);
	IWDG_Init(6,4095);
	
	if(*PWR_CNT_Read()==0)
	{
		Gui_DrawFont_GBK16(5,5,Mycolor,BLACK,"Progress:");
		Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"5%");
		Gui_DrawFont_GBK16(5,25,Mycolor,BLACK,"HLW8112_Inited");
		Gui_DrawFont_GBK16(5,25,BLACK,BLACK,"              ");
		
	//	BC260Y_Init();
		delay_ms(50);
//	Connect_Onenet_Init();
//		Lcd_Clear(BLACK);
//		Gui_DrawFont_GBK16(5,5,Mycolor,BLACK,"Progress:");
//		Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"30%");
//		Gui_DrawFont_GBK16(5,25,Mycolor,BLACK,"Connect_Onenet");	

//		while(onenet_ok<9)
//		{	
//			Wait_Onenet_Response(5000);
//#if DEBUG
//			printf("main\r\n%s\r\n",u8_RxBuf1);
//#endif
//			NB_Rec_Handle();
//			Clear_Buf_CMD(u8_RxBuf1,50);
//		}
	}
	
	delay_ms(100);
	PWR_CNT_Write();
	delay_ms(50);

	relay_Init();
	relay_ON();	
	delay_ms(500);
	
	TIM3_Int_Init(14999,7199);//10Khz的计数频率，计数5K次为500ms,定时为500ms  
	delay_ms (10);
	TIM4_Int_Init(50000,7199);
	delay_ms(50);
	
	Lcd_Clear(WHITE);
//	showimage(gImage_main);
	delay_ms(1000);
	Lcd_Clear(BLACK);
	DisplayMenu();
	
	B_ReadData_HLW8110 = 0;
	BC260Y_NOTIFY_CNT=0;
	
	while(1)
	{
		IWDG_ReloadCounter();
		if(Rx_Data_ING1==1)
		{
			do{
				Rx_Data_ING1++;
				delay_ms(1);
			}while(Rx_Data_ING1<10);

#if DEBUG		
			printf("%s\r\n",u8_RxBuf1);
#endif		
			u8_RX_Index1=0;
			Rx_Data_ING1=0;	
		
//			if(strstr((char*)u8_RxBuf1,"5850"))
//			{
//				NB_Rec_Handle();
//			
//				if (strstr ((char*)onenet_order,"00")!=NULL)
//				{
//					while(Send_Cmd(NB_MIPLEXECUTERSP_CMD,"OK")!=0)
//					{
//						static uint8_t i=0;
//						i++;
//						if(i>5) break;
//						delay_ms(500);
//					}
//					
//			//		relay_OFF();
//					
//#if DEBUG
//					printf("relay_OFF\r\n");
//#endif
//				}
//				if (strstr ((char*)onenet_order,"01")!=NULL)
//				{
//					while(Send_Cmd(NB_MIPLEXECUTERSP_CMD,"OK")!=0)
//					{
//						static uint8_t i=0;
//						i++;
//						if(i>5) break;
//						delay_ms(500);
//					}
//					
//					relay_ON();
//					
//#if DEBUG
//					printf("relay_ON\r\n");
//#endif				
//					
//				}
//			}
//			
			if(strstr((char*)u8_RxBuf1,"5527"))
			{

				NB_Rec_Handle();

				Timer=0;
				Timer_residue=0;
				Timer_residue_BAK=0;

				for(uint8_t i=1;i<strlen(onenet_order);i+=2)
				{
					Timer=Timer*10+(onenet_order[i]-0x30);
				}
		
				if(Timer>5)
				{
					Timer_residue=Timer/5;
					Timer_residue_BAK=Timer_residue;
					Timer=5;
				}
#if DEBUG				
				printf("Timer:%d\r\n",Timer);
				printf("Timer_residue:%d\r\n",Timer_residue);
				printf("Timer_residue_BAK:%d\r\n",Timer_residue_BAK);
#endif
				Timer=Timer*10000-1;
				TIM4_Int_Init(Timer,7199);

				while(Send_Cmd(NB_MIPLEXECUTERSP_CMD, "OK")!=0)
				{
					static uint8_t i=0;
					i++;
					if(i>5) break;
					delay_ms(500);
				}
			}
		}
				
		if ( B_ReadData_HLW8110 != 0 )
		{
			Calculate_HLW8110_MeterData();
			Get_VBAT_Average(1);
			DisplayParameter();
			
			B_ReadData_HLW8110=0;

		}
		
		if ( BC260Y_NOTIFY_CNT != NOTIFY_state )
		{
			Notify_Control(NOTIFY_state);
			NOTIFY_state=BC260Y_NOTIFY_CNT;
			if(NOTIFY_state==1) 
			{
				Signal_Value_Check();
		        Show_Signal();
			}
		}
	}
}
