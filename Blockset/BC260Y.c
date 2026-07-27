#include "air32f10x.h"
#include "stdio.h"
#include "string.h"
#include "delay.h"
#include "usart.h"
#include "BC260Y.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "LCD_Config.h"
#include "relay.h"
#include "test.h"
#include "IWDG.h"


extern float   F_AC_V;										// 电压有效值
extern float   F_AC_I;										// A通道电流
extern float   F_AC_P;										// A通道有功功率
extern float   F_AC_E;										// A通道有功电能(量)
extern float   F_AC_PF;										// 功率因素，A通道和B通道只能选其一 
extern float   F_Angle;										// 相角，A通道和B通道只能选其一 
extern uint8_t BAT_Q;

extern uint8_t relay_state;
char timer[10]={5};

uint8_t u8_RX_Index1=0;
uint8_t u8_RxBuf1[50];
uint8_t	Rx_Data_ING1=0;

uint8_t onenet_ok=0;

char message_ID_3316[10];
char message_ID_3317[10];
char message_ID_3328[10];
char message_ID_3331[10];
char message_ID_3332[10];
char message_ID_3329[10];
char message_ID_3320[10];
char message_ID_3338[10];
char message_ID_3341[10];

uint8_t NB_Signal_Value;

uint8_t NB_OBSERVERSP_CMD[50];
uint8_t NB_DISCOVERRSP_CMD[50];
uint8_t NB_MIPLEXECUTERSP_CMD[40];
char onenet_order[10];

uint8_t NB_NOTIFY_Value_CMD[60];
uint8_t NB_NOTIFY_Unit_CMD[60];


	
void USART1_IRQHandler(void)
{
	if(USART1->SR&(1<<5))	
	{	 
		u8_RxBuf1[u8_RX_Index1] = USART1->DR;		// 数据接收中
		u8_RX_Index1++;
			
		Rx_Data_ING1 = 1;		// 置数据接收标志位	
	}
}

void Wait_Onenet_Response(uint16_t time)
{
	uint16_t count=0;
	while((Rx_Data_ING1==0)&&(count<time))
	{
		count++;
		delay_ms(1);
	}
	if(count==time)// 超时
	{
#if DEBUG
		printf("Waiting Response ERROR\r\n"); // 未接收到Onenet的响应
#endif			
	}
	else // 接收到了响应
	{
		do{
				Rx_Data_ING1++;
				delay_ms(1);
		}while(Rx_Data_ING1<10);

		u8_RX_Index1=0;
		Rx_Data_ING1=0;
		IWDG_ReloadCounter();
	}
}

void Serial_SendByte(uint8_t Byte)
{
	USART1->DR=Byte;
	while((USART1->SR&0X40) == 0);	//等待发送结束
}


void Serial_SendString(unsigned char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}

void Clear_Buf_CMD(uint8_t *Buf,uint8_t length)
{
	for(uint8_t i=0;i<length;i++)
	{
		Buf[i]=0;
	}
}

uint8_t Send_Cmd(uint8_t *cmd, char *recdata)
{
	uint8_t ret;
	uint16_t count=0;
	
	Serial_SendString(cmd);//把命令发送到NB模组
	
	while((Rx_Data_ING1==0)&&(count<300))
	{
		count++;
		delay_ms(1);
	}
	if(count==300)// 超时
	{
		ret=1; // 未接收到BC20的响应
	}
	else // 接收到了响应
	{
		// 接收到最后一个字节 再延时9ms
		do{
				Rx_Data_ING1++;
				delay_ms(1);
		}while(Rx_Data_ING1<10);

		u8_RX_Index1=0;
		Rx_Data_ING1=0;
		ret=2;
		
		if(strstr((const char*)u8_RxBuf1, recdata))// 如果接收到的数据里面有这个字符串
		{
			ret=0; // 正确接收到响应
		}
	}
	
	return ret;
}


void BC260Y_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	      
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	delay_ms(600);
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
	delay_ms(60);
    GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	
	delay_ms(2000);

	while(Send_Cmd((uint8_t *)"AT\r\n", "OK")!=0)
	{
		delay_ms(1000);
	}

	Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"10%");
	Gui_DrawFont_GBK16(5,25,Mycolor,BLACK,"BC260Y is normal");

	while(Send_Cmd((uint8_t *)"ATE0\r\n", "OK")!=0)
	{
		delay_ms(1000);
	}

	Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"15%");
	Gui_DrawFont_GBK16(5,45,Mycolor,BLACK,"Echo turned off");

	NB_Signal_Value=0;
	while((NB_Signal_Value==0)||(NB_Signal_Value==99))
	{
		if(Send_Cmd((uint8_t *)"AT+CSQ\r\n", "OK")==0)
		{
			if(u8_RxBuf1[2]=='+') 
			{
				if(u8_RxBuf1[9]==',')// 信号值是个位数
				{
					NB_Signal_Value=u8_RxBuf1[8]-0x30;	
				}
				else if(u8_RxBuf1[10]==',')// 信号值是十位数
				{
					NB_Signal_Value=(u8_RxBuf1[8]-0x30)*10+(u8_RxBuf1[9]-0x30);
				}
			}
		}
		delay_ms(1000);
	}

	uint8_t temp[2];
	Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"20%");
	sprintf((char *)temp,"%3d",NB_Signal_Value);
	Gui_DrawFont_GBK16(5,65,Mycolor,BLACK,"Signal_Value=");
	Gui_DrawFont_GBK16(105,65,Mycolor,BLACK,temp);
	
	while(Send_Cmd((uint8_t *)"AT+CEREG?\r\n", "+CEREG: 0,1")!=0)
	{
		delay_ms(1000);
	}

	Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"25%");
	Gui_DrawFont_GBK16(5,85,Mycolor,BLACK,"EPS registed");

	while(Send_Cmd((uint8_t *)"AT+CGATT?\r\n","+CGATT: 1")!=0)
	{
		delay_ms(1000);
	}

	Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"30%");
	Gui_DrawFont_GBK16(5,105,Mycolor,BLACK,"PS attached to");
}

void Signal_Value_Check(void)
{
	if(Send_Cmd((uint8_t *)"AT+CSQ\r\n", "OK")==0)
	{
		if(u8_RxBuf1[2]=='+') 
		{
			if(u8_RxBuf1[9]==',')// 信号值是个位数
			{
				NB_Signal_Value=u8_RxBuf1[8]-0x30;
				NB_Signal_Value=(NB_Signal_Value-2)*100/28;			
			}
			else if(u8_RxBuf1[10]==',')// 信号值是十位数
			{
				NB_Signal_Value=(u8_RxBuf1[8]-0x30)*10+(u8_RxBuf1[9]-0x30);
				if(NB_Signal_Value>=31)  NB_Signal_Value=100;
				else NB_Signal_Value=(NB_Signal_Value-2)*99/28;
			}
			Clear_Buf_CMD (u8_RxBuf1,50);
		}
	}
}

void Connect_Onenet_Init()
{
		Send_Cmd((uint8_t *)"AT+MIPLCREATE\r\n", "OK");
		
		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3316,1,\"1\",2,0\r\n","OK");
		delay_ms(50);

		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3317,1,\"1\",2,0\r\n","OK");
		delay_ms(50);	

		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3328,1,\"1\",2,0\r\n","OK");
		delay_ms(50);

		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3320,1,\"1\",2,0\r\n","OK");
		delay_ms(50);

		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3331,1,\"1\",2,0\r\n","OK");
		delay_ms(50);	

		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3332,1,\"1\",1,0\r\n","OK");
		delay_ms(50);	

		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3329,1,\"1\",1,0\r\n","OK");
		delay_ms(50);	

		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3338,1,\"1\",1,0\r\n","OK");
		delay_ms(50);
		
		Send_Cmd((uint8_t *)"AT+MIPLADDOBJ=0,3341,1,\"1\",1,0\r\n","OK");

		Send_Cmd((uint8_t *)"AT+MIPLOPEN=0,86400\r\n","OK");

}

void NB_Rec_Handle(void)
{
	char messages_ID[10]={0};
	char * string;
	char * stringnext;
	
	if(strstr((const char*)u8_RxBuf1, "+MIPLOBSERVE:")) // 接收到订阅请求
	{
#if DEBUG		
		printf("OBS\r\n%s\r\n",u8_RxBuf1);
#endif			
		string = strstr((const char*)u8_RxBuf1,"MIPLOBSERVE");
		string = strstr(string,",") + 1;
		
		stringnext = strstr(string,",");
		
		memcpy(messages_ID, string, stringnext - string);
		
		strcat((char *)NB_OBSERVERSP_CMD,"AT+MIPLOBSERVERSP=0,");
		strcat((char *)NB_OBSERVERSP_CMD,messages_ID);
		strcat((char *)NB_OBSERVERSP_CMD,",1\r\n");

		if(strstr((const char*)stringnext, "3316"))
		{
			strcpy(message_ID_3316,messages_ID);
#if DEBUG			
			printf("V_OBSERVED\r\n");
#endif	 		
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"35%");			
			Gui_DrawFont_GBK16(5,45,Mycolor,BLACK,"V_OBSERVED");
		}
		else if(strstr((const char*)stringnext, "3317"))
		{
			strcpy(message_ID_3317,messages_ID);
#if DEBUG			
			printf("I_OBSERVED\r\n");
#endif	 			
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"40%");
			Gui_DrawFont_GBK16(5,65,Mycolor,BLACK,"I_OBSERVED");
		}
		else if(strstr((const char*)stringnext, "3328"))
		{
			strcpy(message_ID_3328,messages_ID);
#if DEBUG			
			printf("P_OBSERVED\r\n");
#endif	 		
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"50%");			
			Gui_DrawFont_GBK16(5,105,Mycolor,BLACK,"P_OBSERVED");
		}
		else if(strstr((const char*)stringnext, "3331"))
		{
			strcpy(message_ID_3331,messages_ID);
#if DEBUG			
			printf("E_OBSERVED\r\n");
#endif	 				
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"60%");
			Gui_DrawFont_GBK16(5,45,Mycolor,BLACK,"E_OBSERVED");
		}
		else if(strstr((const char*)stringnext, "3332"))
		{
			strcpy(message_ID_3332,messages_ID);
#if DEBUG			
			printf("PA_OBSERVED\r\n");
#endif	 	
			
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"65%");
			Gui_DrawFont_GBK16(5,65,Mycolor,BLACK,"PA_OBSERVED");
		}
		else if(strstr((const char*)stringnext, "3329"))
		{
			strcpy(message_ID_3329,messages_ID);
#if DEBUG			
			printf("PF_OBSERVED\r\n");
#endif	 	
			Lcd_Clear(BLACK);
			Gui_DrawFont_GBK16(5,5,Mycolor,BLACK,"Progress:");			
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"55%");
			Gui_DrawFont_GBK16(5,25,Mycolor,BLACK,"PF_OBSERVED");
		}
		else if(strstr((const char*)stringnext, "3320"))
		{
			strcpy(message_ID_3320,messages_ID); 
#if DEBUG			
			printf("BAT_OBSERVED\r\n");
#endif	 			
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"45%");
			Gui_DrawFont_GBK16(5,85,Mycolor,BLACK,"BAT_OBSERVED");
		}
		else if(strstr((const char*)stringnext, "3338"))
		{
			strcpy(message_ID_3338,messages_ID); 
#if DEBUG			
			printf("Relay_OBSERVED\r\n");
#endif	 		
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"70%");
			Gui_DrawFont_GBK16(5,85,Mycolor,BLACK,"Relay_OBSERVED");		
		}
		else if(strstr((const char*)stringnext, "3341"))
		{
			strcpy(message_ID_3341,messages_ID); 
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"76%");
			Gui_DrawFont_GBK16(5,105,Mycolor,BLACK,"Timer_OBSERVED");
			Lcd_Clear(BLACK);	
			Gui_DrawFont_GBK16(5,5,Mycolor,BLACK,"Progress:");			
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"76%");	
#if DEBUG			
			printf("Timer_OBSERVED\r\n");
#endif	 			
		}
		
		Clear_Buf_CMD(u8_RxBuf1,50);
#if DEBUG		
		printf("%s",NB_OBSERVERSP_CMD);
#endif	 		
		Send_Cmd(NB_OBSERVERSP_CMD, "OK");
		Clear_Buf_CMD(NB_OBSERVERSP_CMD,50);
				
	}

	else if(strstr((const char*)u8_RxBuf1, "+MIPLDISCOVER:")) //接收到发现资源请求
	{
#if DEBUG		
		printf("DIS\r\n%s\r\n",u8_RxBuf1);
#endif	 
		static uint8_t progress=76;
		uint8_t temp[3];
		if(progress<97) 
		{
			progress+=3;
			sprintf((char*)temp,"%d",progress);
			Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,temp);		
			Gui_DrawFont_GBK16(5,25,Mycolor,BLACK,"MIPLDISCOVER_ING");
		}
		else Gui_DrawFont_GBK16(100,5,Mycolor,BLACK,"100%");
		string = strstr((const char*)u8_RxBuf1,"MIPLDISCOVER");
		string = strstr(string,",") + 1;
		stringnext = strstr(string,",");
		memcpy(messages_ID, string, stringnext - string);

		strcat((char *)NB_DISCOVERRSP_CMD,"AT+MIPLDISCOVERRSP=0,");

		strcat((char *)NB_DISCOVERRSP_CMD,messages_ID);

		if(strstr((const char*)stringnext, "3316")||
		   strstr((const char*)stringnext, "3317")||
		   strstr((const char*)stringnext, "3328")||
		   strstr((const char*)stringnext, "3320"))
		{
			strcat((char *)NB_DISCOVERRSP_CMD,",1,9,\"5700;5701\"\r\n");
#if DEBUG			
			printf("V/I/P/BAT_DISCOVERED\r\n");
#endif	 			
		}
		else if(strstr((const char*)stringnext, "3331"))
		{
			strcat((char *)NB_DISCOVERRSP_CMD,",1,9,\"5805;5701\"\r\n");
#if DEBUG			
			printf("E_DISCOVERED\r\n");
#endif	 			
		}
		else if(strstr((const char*)stringnext, "3332"))
		{
			strcat((char *)NB_DISCOVERRSP_CMD,",1,4,\"5705\"\r\n");
#if DEBUG			
			printf("PA_DISCOVERED\r\n");
#endif	 	
		}
		else if(strstr((const char*)stringnext, "3329"))
		{
			strcat((char *)NB_DISCOVERRSP_CMD,",1,4,\"5700\"\r\n");
#if DEBUG			
			printf("PF_DISCOVERED\r\n");
#endif	 			
		}
		else if(strstr((const char*)stringnext, "3338"))
		{
			strcat((char *)NB_DISCOVERRSP_CMD,",1,4,\"5850\"\r\n");
#if DEBUG			
			printf("Relay_DISCOVERED\r\n");
#endif	 			
		}
		else if(strstr((const char*)stringnext, "3341"))
		{
			strcat((char *)NB_DISCOVERRSP_CMD,",1,4,\"5527\"\r\n");
#if DEBUG			
			printf("Timer_DISCOVERED\r\n");
#endif	 			
		}
		
		if(progress==100) Gui_DrawFont_GBK16(5,45,Mycolor,BLACK,"Init finished");
		Clear_Buf_CMD(u8_RxBuf1,50);

#if DEBUG		
		printf("%s\r\n",NB_DISCOVERRSP_CMD);
#endif	 		
		
		uint8_t tmp = Send_Cmd(NB_DISCOVERRSP_CMD,"OK");
		
#if DEBUG		
		printf("%d\r\n",tmp);
		printf("%s\r\n",u8_RxBuf1);
#endif		
		
		Clear_Buf_CMD(NB_DISCOVERRSP_CMD,50);
		onenet_ok++;
		Clear_Buf_CMD(u8_RxBuf1,50);
	}
	
	else if(strstr((const char*)u8_RxBuf1, "+MIPLWRITE:"))
	{
#if DEBUG		
		printf("EXE\r\n%s\r\n",u8_RxBuf1);
#endif	 		
		Clear_Buf_CMD(NB_MIPLEXECUTERSP_CMD,50);
		for(uint8_t i=0;i<10;i++)
		{
			onenet_order[i]=0;
		}
		
		string = strstr((const char*)u8_RxBuf1,"MIPLWRITE");
		string = strstr(string,",") + 1;
		
		stringnext = strstr(string,",");
		
		memcpy(messages_ID, string, stringnext - string);
		
		strcat((char *)NB_MIPLEXECUTERSP_CMD,"AT+MIPLWRITERSP=0,");
		strcat((char *)NB_MIPLEXECUTERSP_CMD,messages_ID);
		strcat((char *)NB_MIPLEXECUTERSP_CMD,",2\r\n");

		if(strstr((const char*)stringnext, "5850"))
		{
			string = strstr(stringnext,"5850");
			onenet_order[0]=string[9];
			onenet_order[1]=string[10];
		}
		else if(strstr((const char*)stringnext,"5527"))
		{	
			char *temp;
			char *temp1;
			char *temp2;
			char *temp3;
			char *temp4;

			temp  = strstr(stringnext,"5527");
			temp1 = strstr(temp,",")+1;
			temp2 = strstr(temp1,",")+1;
			temp3 = strstr(temp2,",")+1;
			temp4 = strstr(temp3,",");
			
			memcpy(onenet_order , temp3 , temp4 - temp3);
			memcpy(timer, temp3, temp4 - temp3);
		}
		
#if DEBUG		
		printf("onenet_order:%s\r\n",onenet_order);	
		printf("%s\r\n",NB_MIPLEXECUTERSP_CMD);
	 	printf("MIPLEXECUTE_RECEVED\r\n");
#endif		
		Clear_Buf_CMD(u8_RxBuf1,50);
		
	}
}


void Data_Processing(uint8_t Pr_ID)
{
	uint8_t temp[10]={0};
	
	strcat((char *)NB_NOTIFY_Value_CMD,"AT+MIPLNOTIFY=0,");
	strcat((char *)NB_NOTIFY_Unit_CMD,"AT+MIPLNOTIFY=0,");

	switch(Pr_ID)
	{
		case 0:
				sprintf((char *)temp,"%.3f",F_AC_V);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3316);
				strcat((char *)NB_NOTIFY_Value_CMD,",3316,0,5700,4,4,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",1,0\r\n");
				strcat((char *)NB_NOTIFY_Unit_CMD,message_ID_3316);
				strcat((char *)NB_NOTIFY_Unit_CMD,",3316,0,5701,1,1,\"V\",0,0\r\n");
#if DEBUG		
				printf("V_Processing_Completed\r\n");
#endif	 				
		break;
		
		case 1:
				sprintf((char *)temp,"%.3f",F_AC_I);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3317);
				strcat((char *)NB_NOTIFY_Value_CMD,",3317,0,5700,4,4,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",1,0\r\n");				
				strcat((char *)NB_NOTIFY_Unit_CMD,message_ID_3317);
				strcat((char *)NB_NOTIFY_Unit_CMD,",3317,0,5701,1,1,\"A\",0,0\r\n");	
#if DEBUG		
				printf("I_Processing_Completed\r\n");
#endif	 		
		break;
		
		case 2:
				sprintf((char *)temp,"%.3f",F_AC_P);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3328);
				strcat((char *)NB_NOTIFY_Value_CMD,",3328,0,5700,4,4,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",1,0\r\n");	
				strcat((char *)NB_NOTIFY_Unit_CMD,message_ID_3328);
				strcat((char *)NB_NOTIFY_Unit_CMD,",3328,0,5701,1,1,\"W\",0,0\r\n");	
#if DEBUG		
				printf("P_Processing_Completed\r\n");
#endif	 		
		break;
		
		case 3:
				sprintf((char *)temp,"%.3f",F_AC_E);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3331);
				strcat((char *)NB_NOTIFY_Value_CMD,",3331,0,5805,4,4,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",1,0\r\n");
				strcat((char *)NB_NOTIFY_Unit_CMD,message_ID_3331);
				strcat((char *)NB_NOTIFY_Unit_CMD,",3331,0,5701,1,3,\"kWh\",0,0\r\n");	
#if DEBUG		
				printf("E_Processing_Completed\r\n");
#endif	 		
		break;
		
		case 4:
				sprintf((char *)temp,"%.1f",F_Angle);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3332);
				strcat((char *)NB_NOTIFY_Value_CMD,",3332,0,5705,4,4,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",0,0\r\n");	
#if DEBUG		
				printf("PA_Processing_Completed\r\n");
#endif	 		
		break;
		
		case 5:
				sprintf((char *)temp,"%.2f",F_AC_PF);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3329);
				strcat((char *)NB_NOTIFY_Value_CMD,",3329,0,5700,4,4,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",0,0\r\n");	
#if DEBUG		
				printf("PF_Processing_Completed\r\n");
#endif	 		
		break;
		
		case 6:
				sprintf((char *)temp,"%d",BAT_Q);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3320);
				strcat((char *)NB_NOTIFY_Value_CMD,",3320,0,5700,4,4,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",1,0\r\n");	
				strcat((char *)NB_NOTIFY_Unit_CMD,message_ID_3320);
				strcat((char *)NB_NOTIFY_Unit_CMD,",3320,0,5701,1,1,\"%\",0,0\r\n");
#if DEBUG		
				printf("BAT_Processing_Completed\r\n");
#endif	 		
		break;
		
		case 7:
				sprintf((char *)temp,"%d",relay_state);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3338);
				strcat((char *)NB_NOTIFY_Value_CMD,",3338,0,5850,5,1,");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,",0,0\r\n");	
#if DEBUG		
				printf("relay_state_Processing_Completed\r\n");
#endif	 		
		break;
		
		case 8:
				sprintf((char *)temp,"%s",timer);
				strcat((char *)NB_NOTIFY_Value_CMD,message_ID_3341);
				strcat((char *)NB_NOTIFY_Value_CMD,",3341,0,5527,1,10,\"");
				strcat((char *)NB_NOTIFY_Value_CMD,(char *)temp);
				strcat((char *)NB_NOTIFY_Value_CMD,"\",0,0\r\n");	
#if DEBUG		
				printf("Timer_Processing_Completed\r\n");
#endif	 		
		break;
		
	}
}

void BC260Y_Notify(uint8_t begin_sign,uint8_t end_sign)
{
	uint8_t i;
	for(i=begin_sign;i<end_sign;i++)
	{	
		Clear_Buf_CMD(NB_NOTIFY_Value_CMD,60);
		Clear_Buf_CMD(NB_NOTIFY_Unit_CMD,60);

		Data_Processing(i);
		
		Send_Cmd(NB_NOTIFY_Value_CMD,"OK");
		
		if(!(i==4||i==5||i==7||i==8))
			Send_Cmd(NB_NOTIFY_Unit_CMD,"OK");

	}
}

void Notify_Control(uint8_t transmit_ID)
{
	switch(transmit_ID)
	{
		case 0:
			BC260Y_Notify(0,1);	
#if DEBUG		
		printf("上传电压数据\r\n");
#endif	 		
		break;
		
		case 1:
			BC260Y_Notify(1,2);	
#if DEBUG		
		printf("上传电流数据\r\n");
#endif	 		
		break;
		
		case 2:
			BC260Y_Notify(2,3);	
#if DEBUG		
		printf("上传功率数据\r\n");
#endif	 		
		break;
		
		case 3:
			BC260Y_Notify(3,4);	
#if DEBUG		
		printf("上传电能数据\r\n");
#endif	 		
		break;
		
		case 4:
			BC260Y_Notify(4,5);	
#if DEBUG		
		printf("上传相角数据\r\n");
#endif	 		
		break;
		
		case 5:
			BC260Y_Notify(5,6);	
#if DEBUG		
		printf("上传功率因素数据\r\n");
#endif	 		
		break;
		
		case 6:
			BC260Y_Notify(6,7);	
#if DEBUG		
		printf("上传电池数据\r\n");
#endif	 		
		break;
		
		case 7:
			BC260Y_Notify(7,8);	
#if DEBUG		
		printf("上传继电器状态数据\r\n");
#endif	 		
		break;
		
		case 8:
			BC260Y_Notify(8,9);	
#if DEBUG		
		printf("上传计时器数据\r\n");
#endif	 		
		break;
			
	}
}

