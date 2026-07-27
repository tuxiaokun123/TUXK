#ifndef __E_METER_H
#define __E_METER_H

void E_Clear_Init(void);
void E_Data_Clear(void);
void E_Data_Write(void);
float * E_Data_Read(void);
void PWR_CNT_Clear(void);
void PWR_CNT_Write(void);
uint16_t * PWR_CNT_Read(void);
	
#endif
