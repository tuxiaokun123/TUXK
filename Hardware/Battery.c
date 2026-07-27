#include "air32f10x.h"
#include "delay.h"
#include "E_Meter_CRT.h"
#include "Power_EN.h"
#include "Relay.h"
#include "BC260Y.h"

float ADC_BAT_Value;
float ADC_VREFIN_Value;
float VBAT_AVG;
uint8_t BAT_Q;


void battery_EN(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	      
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);	
}


void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值
	
	ADC_TempSensorVrefintCmd(ENABLE);  //使能内部参考电压ADC	
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
//	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);

}

float AD_GetValue(uint8_t ADC_Channel)
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}

float Get_VBAT_Average(u8 times)
{
	//设置指定ADC的规则组通道，一个序列，采样时间
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 2, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期
	
	
	float temp_val=0;
	u8 t;
	static u8 shutdown=0;
	float VBAT;
	for(t=0;t<times;t++)
	{
/*		ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	
		ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		return ADC_GetConversionValue(ADC1);
*/		
		ADC_BAT_Value=AD_GetValue(ADC_Channel_1);
//		ADC_BAT_Value=ADC_GetConversionValue(ADC1);
		
//		ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
	
//		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
		delay_ms(5);
		ADC_VREFIN_Value=AD_GetValue(ADC_Channel_Vrefint);	//返回最近一次ADC1规则组的转换结果
	
		VBAT = 1.2*ADC_BAT_Value/ADC_VREFIN_Value;
		temp_val+=VBAT;
		delay_ms(5);
		
	}
	VBAT_AVG=temp_val/times*2;
	
	if(VBAT_AVG>3.9)
		BAT_Q=100;
	else if(VBAT_AVG<3.1)
	{
		BAT_Q=0;
		shutdown++;
		
		E_Data_Write();
		
		if(shutdown>2)
		{
			Shut_Down();
		}
	}
	
	else
	{
		BAT_Q=100-(3.9-VBAT_AVG)*100/0.8;
		
		// relay_OFF();
		
		shutdown++;
		
		E_Data_Write();
		
		if(shutdown>2)
		{
//			Log_Out();
			Shut_Down();
		}
		
	}
	
	return VBAT_AVG;
} 	 

