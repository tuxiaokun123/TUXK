#ifndef __BATTERY_H
#define __BATTERY_H

void battery_EN(void);
void Adc_Init(void);
float AD_GetValue(uint8_t ADC_Channel);
float Get_VBAT_Average(u8 times);

#endif
