#include "timer.h"
//#include "led.h"



uint8_t B_ReadData_HLW8110;
uint8_t BC260Y_NOTIFY_CNT;
extern uint16_t Timer;
extern uint16_t Timer_residue;
extern uint16_t Timer_residue_BAK;

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		    		  			    
	if(TIM3->SR&0X0001)//溢出中断
	{
		B_ReadData_HLW8110=1;
	}				   
	TIM3->SR&=~(1<<0);//清除中断标志位 	    
}
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
//时间中断：Tout(溢出时间,单位s) = ((arr+1)*psc+1)/Tclk
//Tclk(TIM3的时钟输出频率，单位Mhz)
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3时钟使能    
 	TIM3->ARR=arr;  	//设定计数器自动重装值//刚好1ms    
	TIM3->PSC=psc;  	//预分频器7200,得到10Khz的计数时钟		  
	TIM3->DIER|=1<<0;   //允许更新中断	  
	TIM3->CR1|=0x01;    //使能定时器3
  	MY_NVIC_Init(1,3,TIM3_IRQn,2);//抢占1，子优先级3，组2									 
}

void TIM4_IRQHandler(void)
{ 		    		  			    
	if(TIM4->SR&0X0001)//溢出中断
	{
		if(Timer_residue>0)
		{
			Timer_residue--;
		}
		else if(BC260Y_NOTIFY_CNT<8)
		{
			BC260Y_NOTIFY_CNT++;
			Timer_residue=Timer_residue_BAK;
		}
		else
		{
			BC260Y_NOTIFY_CNT=0;
			Timer_residue=Timer_residue_BAK;
		}
	}				   
	TIM4->SR&=~(1<<0);//清除中断标志位 	    
}

void TIM4_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<2;	//TIM3时钟使能    
 	TIM4->ARR=arr;  	//设定计数器自动重装值//刚好1ms    
	TIM4->PSC=psc;  	//预分频器7200,得到10Khz的计数时钟		  
	TIM4->DIER|=1<<0;   //允许更新中断	  
	TIM4->CR1|=0x01;    //使能定时器4
  	MY_NVIC_Init(1,3,TIM4_IRQn,2);//抢占1，子优先级3，组2									 
}














