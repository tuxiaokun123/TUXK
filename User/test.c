#include "sys.h"
#include "usart.h"		
#include "Delay.h"	 
#include "led.h"
#include "key.h"
#include "timer.h"
#include "HLW8112.h"
#include "test.h"



extern unsigned char B_ReadData_HLW8110;


int main(void)
{	

	//u8 key;	
//	u16 t; 
//	u16 len;	
//	u16 times=0;   
	Stm32_Clock_Init(9);		//系统时钟设置
	//uart_init(72,115200);  		//72M,115200bps,串口1设置，与PCB通讯
	uart_init(36,115200); 		//延时初始化
	
	LED_Init();		  	//初始化与LED连接的硬件接口
//	TIM3_Int_Init(4999,7199);//10Khz的计数频率，计数5K次为500ms,定时为500ms 
	
	TIM3_Int_Init(999,7199);//10Khz的计数频率，计数5K次为500ms,定时为500ms  
	
/*	delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
		delay_init(200);	  			//延时初始化
*/	
#if HLW8112	
	uart2_init(18,9600);		//36M（最大36M）,PCLK1分频后18M，9600，偶校验，串口2设置，与8110通讯
	Init_HLW8110();
	B_ReadData_HLW8110 = 0;
#endif
	
#if HLW8112	
	Init_HLW8112();
	B_ReadData_HLW8110 = 0;
#endif	
	



	
	while(1)
	{
		
		//每3HZ读取一次HLW8110芯片内的寄存器数据,每100ms读取一次数据
		//读取数据的频率视
		#if HLW8112
			if ( B_ReadData_HLW8110 == 1 )
			{

				Calculate_HLW8110_MeterData();
				B_ReadData_HLW8110 = 0;

				

			}
		#endif
		
		#if HLW8112
			if ( B_ReadData_HLW8110 == 1 )
			{
				B_ReadData_HLW8110 = 0;
				HLW8112_Measure();
			}
		#endif
		
		
//		LED0=!LED0;
//		delay_ms(200);
		
		
//		if(USART_RX_STA&0x8000)
//				{					   
//					len=USART_RX_STA&0x3FFF;//得到此次接收到的数据长度					
//					printf("\r\n您发送的消息为:\r\n\r\n");
//					for(t=0;t<len;t++)
//					{
//						USART1->DR=USART_RX_BUF[t];
//						while((USART1->SR&0X40)==0);//等待发送结束
//					}
//					printf("\r\n\r\n");//插入换行
//					USART_RX_STA=0;
//				}else
//				{
//					times++;
//					if(times%5000==0)
//					{
//						printf("\r\n精英STM32F103开发板 串口实验\r\n");
//						printf("正点原子@ALIENTEK\r\n\r\n");
//					}
//					if(times%200==0)printf("请输入数据,以回车键结束\r\n");  
//					if(times%30==0)LED0=!LED0;//闪烁LED,提示系统正在运行.
//					delay_ms(10);   
//				}
	}
	 
} 






