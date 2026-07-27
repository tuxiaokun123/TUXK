#ifndef __TEST_H
#define __TEST_H	

#include "sys.h"


/*====================================================================================
HLW8110:(HLW8110 = 1,HLW8112 = 0)
HLW8112:(HLW8110 = 0,HLW8112 = 1)


//uart
#define HLW8110		1		//使能1--使用UART通讯(HLW8110)，使能0--使用SPI通讯(HLW8112)	
#define HLW8112		0		//使能1--使用UART通讯(HLW8112)，使能0--使用SPI通讯(HLW8110)	


//spi
//#define HLW8110	0		//使能1--使用UART通讯(HLW8110)，使能0--使用SPI通讯(HLW8112)	
//#define HLW8112	1		//使能1--使用UART通讯(HLW8112)，使能0--使用SPI通讯(HLW8110)	


====================================================================================*/
//选择通讯方式
#define HLW8110		1		//使能1--使用UART通讯(HLW8110)，使能0--使用SPI通讯(HLW8112)	
#define HLW8112		0		//使能1--使用UART通讯(HLW8112)，使能0--使用SPI通讯(HLW8110)	

#define DEBUG  0
//#define Debug 1


//直流校正系数

//8112A通道或8110通道校正系数
#define D_CAL_U		1000/1000		//电压校正系数
#define D_CAL_A_I	18500/1000		//A通道电流校正系数
#define D_CAL_A_P	18500/1000		//A通道功率校正系数
#define D_CAL_A_E	18500/1000		//A通道电能校正系数


//8112 B通道校正系数
#define D_CAL_B_P	1000/1000		//B通道功率校正系数
#define D_CAL_B_I	1000/1000		//B通道电流校正系数
#define D_CAL_B_E	1000/1000		//B通道电能校正系数


/*
#if HLW8110

#define IO_HLW8112_EN	PAout(5)

#define IO_HLW8112_CS	PAout(6)
#define IO_HLW8112_SCLK	PAout(7)

#endif




#if HLW8112

#define IO_HLW8112_EN	PAout(5)

#define IO_HLW8112_CS	PAout(6)
#define IO_HLW8112_SCLK	PAout(7)

#define IO_HLW8112_SDI	PAout(2)
#define IO_HLW8112_SDO	PAin(3)

#define IO_HLW8112_INT1	PDin(11)
#define IO_HLW8112_INT2	PDin(12)

#endif
*/	



#endif
