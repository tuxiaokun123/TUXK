#include "air32f10x.h"
#include "stmflash.h"
#include "Power_EN.h"

extern float F_AC_BACKUP_E;

void E_Clear_Init(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;
	      
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}


void E_Data_Clear(void)
{
	uint16_t resetbuffer[2]={0};
	STMFLASH_Write(0X08018000,(u16*)resetbuffer,2);
}

void E_Data_Write(void)
{
	/*1 拆分*/
	uint16_t *pa1 = (uint16_t *)(&F_AC_BACKUP_E);		//获得a前16位地址
	uint16_t *pa2 = pa1+1;		//获得a后16位地址
		
	u16 pbuffer[2];
	/*定义一个同类型对象，用来接收组合后的结果*/
	pbuffer[0] = *pa1;
	pbuffer[1] = *pa2;

	STMFLASH_Write(0X08018000,(u16*)pbuffer,2);
	
//	delay_ms(10);
	
}

float * E_Data_Read(void)
{
	u16 pbuffer[2]={0};
	STMFLASH_Read(0X08018000,(u16*)pbuffer,2);
	float *LAST_AC_E=(float *)pbuffer;
	return LAST_AC_E;
}

void PWR_CNT_Clear(void)
{
	uint16_t Clearbuffer[1]={0};
	STMFLASH_Write(0X08019000,(u16*)Clearbuffer,1);
}

void PWR_CNT_Write(void)
{
	/*1 拆分*/
	uint16_t a=1;
	uint16_t *pa1 = (uint16_t *)(&a);		//获得a前16位地址
		
	u16 pbuffer[1];
	/*定义一个同类型对象，用来接收组合后的结果*/
	pbuffer[0] = *pa1;

	STMFLASH_Write(0X08019000,(u16*)pbuffer,1);
	
//	delay_ms(10);
	
}

uint16_t * PWR_CNT_Read(void)
{
	u16 pbuffer[1]={0};
	STMFLASH_Read(0X08019000,(u16*)pbuffer,1);
	uint16_t *PWR_CNT=(uint16_t *)pbuffer;
	return PWR_CNT;
}
