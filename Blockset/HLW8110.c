
#include "HLW8110.h"
#include "delay.h"
#include "stdio.h"
#include "test.h"
#include "E_Meter_CRT.h"

#if HLW8110

#define HIGH	1
#define LOW		0

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
union IntData
{
	u16  inte;			
	u8 byte[2];		
};
union LongData
{
    u32  word;		
    u16  inte[2];		
    u8   byte[4];		
};
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

unsigned char	u8_TxBuf[10]; 
unsigned char	u8_RxBuf[10];
unsigned char	u8_TX_Length;
unsigned char	u8_RX_Length;
unsigned char	u8_RX_Index;
//unsigned char	B_ReadReg_Time_EN;			// 串口读取寄存器数据，时间计数器标志位，1--开启计数，0--关闭计数
//unsigned char	B_Tx_Finish;
unsigned char	B_Rx_Finish;
unsigned char	B_Rx_Data_ING;					// 接收数据标志位	,		< 1:接收数据中,0:未接收到数据 >
unsigned char	B_Read_Error;							// UART读取出据校验和出错,< 1:数据读错，0:数据读取正确 >
//unsigned char	u8_ReadReg_Index;
//unsigned char	u8_ReadReg_Time;				// 串口读取寄存器数据的时间
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

unsigned int    U16_TempData;	

unsigned int    U16_IFData;
unsigned int    U16_RIFData;
unsigned int    U16_LineFData;
unsigned int    U16_AngleData;
unsigned int    U16_PFData;
unsigned int 	U16_HFConst_RegData;
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
unsigned int	U16_RMSIAC_RegData; 			// A通道电流转换系数
unsigned int	U16_RMSIBC_RegData; 			// B通道电流转换系数
unsigned int	U16_RMSUC_RegData; 				// 电压通道转换系数
unsigned int	U16_PowerPAC_RegData; 		// A通道功率转换系数
unsigned int	U16_PowerPBC_RegData; 		// B通道功率转换系数
unsigned int	U16_PowerSC_RegData; 			// 视在功率转换系数,如果选择A通道，则是A通道视在功率转换系数。A和B通道只能二者选其一
unsigned int	U16_EnergyAC_RegData; 		// A通道有功电能(量)转换系数 
unsigned int	U16_EnergyBC_RegData; 		// B通道有功电能(量)转换系数
unsigned int	U16_CheckSUM_RegData; 		// 转换系数的CheckSum
unsigned int	U16_CheckSUM_Data; 				// 转换系数计算出来的CheckSum

unsigned int	U16_Check_SysconReg_Data; 						
unsigned int	U16_Check_EmuconReg_Data; 						
unsigned int	U16_Check_Emucon2Reg_Data; 	

unsigned int    U16_OVLVL_RegData;
unsigned int    U16_IE_RegData;
unsigned int    U16_SYSCONData;
unsigned int    U16_EMUCON_RegData;
unsigned int    U16_EMUCON2_RegData;
unsigned int    U16_INT_RegData;
unsigned int    U16_OIALVL_RegData;
unsigned int    REG_RIF_RegData;

/*---------------------------------------------------------------------------------------------------------*/			
/*---------------------------------------------------------------------------------------------------------*/
unsigned long 	U32_RMSIA_RegData;			// A通道电流有效值寄存器
unsigned long 	U32_RMSU_RegData;				// 电压有效值寄存器
unsigned long 	U32_POWERPA_RegData;		// A通道功率有效值寄存器
unsigned long 	U32_ENERGY_PA_RegData;	// A通道有功电能（量）有效值寄存器


unsigned long 	U32_RMSIB_RegData;			// B通道电流有效值寄存器
unsigned long	U32_POWERPB_RegData;		// B通道功率有效值寄存器
unsigned long 	U32_ENERGY_PB_RegData;	// B通道有功电能（量）有效值寄存器
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
float   F_AC_V;													// 电压有效值
float   F_AC_I;													// A通道电流
float   F_AC_P;													// A通道有功功率
float   F_AC_E;													// A通道有功电能(量)
float   F_AC_BACKUP_E;									// A通道电量备份	
float   F_AC_PF;												// 功率因素，A通道和B通道只能选其一 
float	F_Angle;												// 相角，A通道和B通道只能选其一 

float   F_AC_I_B;												// B通道电流有效值
float   F_AC_P_B;												// B通道有功功率
float 	F_AC_E_B;												// B通道有功电能(量)
float   F_AC_BACKUP_E_B;								// B通道电量备份	
float   F_AC_LINE_Freq;     						// 市电线性频率
float   LAST_AC_E;
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/


void USART2_IRQHandler(void)
{
	//接收数据
	if(USART2->SR&(1<<5))	
	{	 
		
		if (u8_RX_Index < u8_RX_Length )
		{
			u8_RxBuf[u8_RX_Index] = USART2->DR;		// 数据接收中
			u8_RX_Index++;
			
			B_Rx_Data_ING = 1;										// 置数据接收标志位
		}
		else
		{
			B_Rx_Finish = TRUE;										// 数据接收完毕
			u8_RX_Index = 0;

		}
	}
	
	//发送数据
	
}

void Start_Send_UartData(unsigned char len)
{
	unsigned char i;
	for(i=0;i<len;i++)
	{
		
		while((USART2->SR&0X40) == 0);	//等待发送结束
		
		//delay_us(50);	//两个字节之间增加延时
		USART2->DR	=	u8_TxBuf[i];
		
	}

}

void Clear_RxBuf(void)
{
	unsigned char i;
	for(i = 0;i<10;i++)
	{
		u8_RxBuf[i] = 0x00;
	}
	
	B_Rx_Data_ING = 0;
	B_Rx_Finish = FALSE;
	u8_RX_Index = 0;
}

unsigned char HLW8110_checkSum_Write(unsigned char u8_Reg_length)
{
	unsigned char i;
	unsigned char Temp_u8_checksum;
	unsigned int	a;

	a = 0x0000;
	Temp_u8_checksum = 0;
	for (i = 0; i< (u8_Reg_length-1); i++)
		{
			a += u8_TxBuf[i];
		}
	
	a = ~a;
	Temp_u8_checksum = a & 0xff;

	return Temp_u8_checksum;
	
}

unsigned char HLW8110_checkSum_Read(unsigned char u8_Reg_length)
{
	unsigned char i;
	unsigned char Temp_u8_checksum;
	unsigned int a;

	a = 0x0000;
	Temp_u8_checksum = 0;
	for (i = 0; i< (u8_Reg_length-1); i++)
		{
			a += u8_RxBuf[i];
		}
		
	a = a + u8_TxBuf[0] + u8_TxBuf[1];
	a = ~a;
		
	Temp_u8_checksum = a & 0xff;

	return Temp_u8_checksum;
	
}

void Uart_HLW8110_WriteREG_EN(void)
{

	u8_TX_Length = 4;
	u8_RX_Length = 0;
	
	u8_TxBuf[0] = 0xa5;
	u8_TxBuf[1] = 0xea;
	u8_TxBuf[2] = 0xe5;
//	u8_TxBuf[3] = 0x8b;  //checksum
	u8_TxBuf[3] = HLW8110_checkSum_Write(u8_TX_Length);

	Start_Send_UartData(u8_TX_Length);
		
}

void Uart_HLW8110_WriteREG_DIS(void)
{
	
	u8_TX_Length = 4;
	u8_RX_Length = 0;
	
	u8_TxBuf[0] = 0xa5;
	u8_TxBuf[1] = 0xea;
	u8_TxBuf[2] = 0xdc;
//	u8_TxBuf[3] = 0x94;  //checksum
	u8_TxBuf[3] = HLW8110_checkSum_Write(u8_TX_Length);
        
	Start_Send_UartData(u8_TX_Length);
}

void Uart_HLW8110_Set_Channel_A(void)
{
	u8_TX_Length = 4;
	u8_RX_Length = 0;
	
	u8_TxBuf[0] = 0xa5;
	u8_TxBuf[1] = 0xea;
	u8_TxBuf[2] = 0x5a;
//	u8_TxBuf[3] = 0x16;  //checksum
	u8_TxBuf[3] = HLW8110_checkSum_Write(u8_TX_Length);

	Start_Send_UartData(u8_TX_Length);
}


void Uart_Read_HLW8110_Reg(unsigned char ADDR_Reg,unsigned char u8_reg_length)
{
	u8_TxBuf[0] = 0xa5;
	u8_TxBuf[1] = ADDR_Reg;
	u8_TX_Length =  2;
	u8_RX_Length = u8_reg_length + 1;	// +1，是因为接收的数据长度，除了REG值，还有一个校验和数据
	
	
	Clear_RxBuf();										//清空接收缓冲区
	Start_Send_UartData(u8_TX_Length);
}

void Uart_Write_HLW8110_Reg(unsigned char ADDR_Reg,unsigned char u8_reg_length,unsigned long u32_data)
{
	unsigned char i;
	union LongData Temp_u32_a;

	
	u8_TxBuf[0] = 0xa5;
	u8_TxBuf[1] = ADDR_Reg|0x80;

	Temp_u32_a.word = u32_data;
	for (i = 0; i< u8_reg_length; i++)
		{
			u8_TxBuf[i+2] = Temp_u32_a.byte[u8_reg_length-1-i];						//STM32，32位MCU，union定义，是低位在前
			//u8_TxBuf[i+2] = Temp_u32_a.byte[4-u8_reg_length + i];				//STM8,STC MCU,   union定义，是高位在前
		}


	u8_TX_Length = 3 + u8_reg_length ;
	u8_RX_Length = 0;
	
	u8_TxBuf[u8_TX_Length-1] = HLW8110_checkSum_Write(u8_TX_Length);


	Start_Send_UartData(u8_TX_Length);
}

void Uart_HLW8110_Reset(void)
{
	
	u8_TX_Length = 4;
	u8_RX_Length = 0;
	
	u8_TxBuf[0] = 0xa5;
	u8_TxBuf[1] = 0xea;
	u8_TxBuf[2] = 0x96;
//	u8_TxBuf[3] = 0xda;  //checksum
	
	u8_TxBuf[3] = HLW8110_checkSum_Write(u8_TX_Length);

	Start_Send_UartData(u8_TX_Length);
}




unsigned char Judge_CheckSum_HLW8110_Calfactor(void)
{
	unsigned long a;
	//unsigned int b;
	//unsigned int c;
	unsigned char d;
 
  //读取RmsIAC、RmsIBC、RmsUC、PowerPAC、PowerPBC、PowerSC、EnergAc、EnergBc的值
	
	Uart_Read_HLW8110_Reg(REG_RMS_IAC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_RMSIAC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG
		printf("A通道电流转换系数:%x\n " ,U16_RMSIAC_RegData);
#endif				
	}
	
	
	Uart_Read_HLW8110_Reg(REG_RMS_IBC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_RMSIBC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG
		printf("B通道电流转换系数:%x\n " ,U16_RMSIBC_RegData);
#endif				
	}
	
	
	Uart_Read_HLW8110_Reg(REG_RMS_UC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_RMSUC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG		
		printf("电压通道转换系数:%x\n " ,U16_RMSUC_RegData);
#endif				
	}
		
	Uart_Read_HLW8110_Reg(REG_POWER_PAC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_PowerPAC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG
		printf("A通道功率转换系数:%x\n " ,U16_PowerPAC_RegData);
#endif		
	}
		
	Uart_Read_HLW8110_Reg(REG_POWER_PBC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_PowerPBC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG		
		printf("B通道功率转换系数:%x\n " ,U16_PowerPAC_RegData);
#endif				
	}
	
	Uart_Read_HLW8110_Reg(REG_POWER_SC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_PowerSC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG		
		printf("视在功率转换系数:%x\n " ,U16_PowerSC_RegData);
#endif			
	}
	
	Uart_Read_HLW8110_Reg(REG_ENERGY_AC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_EnergyAC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG		
		printf("A通道电量转换系数:%x\n " ,U16_EnergyAC_RegData);
#endif				
	
	}
	Uart_Read_HLW8110_Reg(REG_ENERGY_BC_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_EnergyBC_RegData = (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG		
		printf("B通道电量转换系数:%x\n " ,U16_EnergyBC_RegData);
#endif				
	}
	
 
	Uart_Read_HLW8110_Reg(REG_CHECKSUM_ADDR,2);
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_CheckSUM_RegData= (u8_RxBuf[0]<<8) + u8_RxBuf[1] ;
#if DEBUG		
		printf("系数校验和:%x\n " ,U16_CheckSUM_RegData);
#endif				
	}
	
	
	a = 0;
	a = ~(0xffff+U16_RMSIAC_RegData + U16_RMSIBC_RegData + U16_RMSUC_RegData + 
        U16_PowerPAC_RegData + U16_PowerPBC_RegData + U16_PowerSC_RegData + 
          U16_EnergyAC_RegData + U16_EnergyBC_RegData  );
  
	U16_CheckSUM_Data = a & 0xffff;
#if DEBUG	
		printf("计算系数校验和:%x\n " ,U16_CheckSUM_Data);
#endif		  
	if ( U16_CheckSUM_Data == U16_CheckSUM_RegData)
	{
		d = 1;
#if DEBUG	
		printf("校验和正确\r\n ");
#endif		    
	}
	else
	{
		d = 0;
#if DEBUG
		printf("校验和出错\r\n ");
#endif				
	}
  
	return d;
  
}

void Init_HLW8110(void)
{
	RCC->APB2ENR|=1<<2;       	 
	RCC->APB2ENR|=1<<3;  	
	   	 
	GPIOA->CRL&=0XF0FFFFFF;  //IO状态设置,设置PA7、PA6、PA5、PA3、PA2
	GPIOA->CRL|=0X08000000;  //IO状态设置，PA7、PA6、PA5、PA2-输出、PA3-输入	 
    GPIOB->CRL&=0XFFFFFFF0;
	GPIOB->CRL|=0X00000008;	
	
  //9600 bps,1S传输9600/11bit = 872byte
//  IO_HLW8112_EN = LOW;
//  IO_HLW8112_SCLK = HIGH;
 // IO_HLW8112_CS = LOW;
	
	
	Uart_HLW8110_WriteREG_EN();
	delay_ms(10);
	
//电流通道A设置命令，指定当前用于计算视在功率、功率因数、相角、瞬时有功功率、瞬时视在功率和有功功率过载的信号指示 的通道为通道A	
	Uart_HLW8110_Set_Channel_A();			
	delay_ms(10);
	Uart_Write_HLW8110_Reg(REG_SYSCON_ADDR,2,0x0A00);	//开启A通道，关闭B通道，电压通道PGA = 1，电流通道PGA = 16
	delay_ms(10);
	

	Uart_Write_HLW8110_Reg(REG_EMUCON_ADDR,2,0x0801);	//1，使能PFA 脉冲输出和有功电能寄存器累加；
//	Uart_Write_HLW8110_Reg(REG_EMUCON_ADDR,2,0x0018);	//正向和负向过零点均发生变化，ZXD0 = 1，ZXD1 = 1
	Uart_Write_HLW8110_Reg(REG_EMUCON2_ADDR,2,0x057F);	//0x0001是EMUCON2的默认值，waveEn = 1,zxEn = 1，A通道电量寄存器，读后不清0，EPA_CB = 1；打开功率因素检测
	delay_ms(10);
	

	//关闭所有中断
	Uart_Write_HLW8110_Reg(REG_IE_ADDR,2,0x0000);

	
	Uart_HLW8110_WriteREG_DIS();//关闭写8112 Reg
//	delay_ms(10);	
  //读取地址是0x6F至0x77的寄存器，验证系数是否正确
	Judge_CheckSum_HLW8110_Calfactor();	
//    Set_V_Zero();         //设置INT1
//	Set_Leakage();
// Set_OVLVL();          //设置INT2
//	Set_underVoltage();
	LAST_AC_E=0;
	LAST_AC_E=*E_Data_Read();
	F_AC_BACKUP_E=LAST_AC_E;
}


void Check_WriteReg_Success(void)
{
	Uart_Read_HLW8110_Reg(REG_SYSCON_ADDR,2);
	delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_Check_SysconReg_Data =  (u8_RxBuf[0]<<8) + (u8_RxBuf[1]); 
#if DEBUG		
		printf("写入的SysconReg寄存器:%x\n " ,U16_Check_SysconReg_Data);
#endif				
	}
	else
	{
#if DEBUG		
		printf("SysconReg寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	
	Uart_Read_HLW8110_Reg(REG_EMUCON_ADDR,2);
	delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_Check_EmuconReg_Data =  (u8_RxBuf[0]<<8) + (u8_RxBuf[1]); 
#if DEBUG		
		printf("写入的EmuconReg寄存器:%x\n " ,U16_Check_EmuconReg_Data);
#endif				
	}
	else
	{
#if DEBUG		
		printf("EmuconReg寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	
	
	Uart_Read_HLW8110_Reg(REG_EMUCON2_ADDR,2);
		delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_Check_Emucon2Reg_Data =  (u8_RxBuf[0]<<8) + (u8_RxBuf[1]); 
#if DEBUG		
		printf("写入的Emucon2Reg寄存器寄存器:%x\n " ,U16_Check_Emucon2Reg_Data);
#endif				
	}
	else
	{
#if DEBUG		
		printf("Emucon2Reg寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	
}

void Read_HLW8110_IA(void)
{	
	float a;
	
	Uart_Read_HLW8110_Reg(REG_RMSIA_ADDR,3);
		delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U32_RMSIA_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]); 
#if DEBUG		
		printf("A通道电流寄存器:%lx\n " ,U32_RMSIA_RegData);
#endif				
	}
	else
	{
#if DEBUG		
		printf("A通道电流寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	
	

  //U16_AC_I = (U32_RMSIA_RegData * U16_RMSIAC_RegData)/(电流系数* 2^23）
	if ((U32_RMSIA_RegData & 0x800000) == 0x800000)
	{
			F_AC_I = 0;
	}
	else
	{
		a = (float)U32_RMSIA_RegData;
		a = a * U16_RMSIAC_RegData;
		a  = a/0x800000;                     //电流计算出来的浮点数单位是mA,比如5003.12 
		a = a/1;  														// 1 = 电流系数,系数计算可以参考excel资料,1 = current coefficient, coefficient calculation can refer to excel data
		a = a/1000;              //a= 5003ma,a/1000 = 5.003A,单位转换成A(mA->A)
		a = (a-0.01) * D_CAL_A_I;				//D_CAL_A_I = 0.1 是校正系数，默认是1(D_ CAL_ A_ I is the correction factor. The default value is 1)
//		if(a<0)
//		F_AC_I = 0;
//		else 		
//		F_AC_I = a;		
		F_AC_I=F_AC_P/F_AC_V;		
	}
}

void Read_HLW8110_U(void)
{
	float a;
	
	Uart_Read_HLW8110_Reg(REG_RMSU_ADDR,3);
		delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U32_RMSU_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]);
#if DEBUG		
		printf("电压通道寄存器:%lx\n " ,U32_RMSU_RegData);
#endif				
	}
	else
	{
#if DEBUG		
		printf("电压通道寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	

	//U16_AC_V = (U32_RMSU_RegData * U16_RMSUC_RegData)/2^23
	
	 if ((U32_RMSU_RegData &0x800000) == 0x800000)
	 {
			F_AC_V = 0;
	 }
  else
	{
  a =  (float)U32_RMSU_RegData;
  a = a*U16_RMSUC_RegData;  
	a = a/0x400000;     
	a = a/1;  							// 1 = 电压系数,系数计算可以参考excel资料,1 = voltage coefficient, coefficient calculation can refer to excel data
	a = a/100; 				 		//计算出a = 22083.12mV,a/100表示220.8312V，电压转换成V,(Calculated a = 22083.12mv, a / 100 means 220.8312v, voltage converted into v)
	a = a*D_CAL_U;				//D_CAL_U是校正系数，默认是1(D_ CAL_ U is the correction factor. The default value is 1)		
	F_AC_V = a;
	}
}

void Read_HLW8110_PA(void)
{
	float a;
	float b;
	
	Uart_Read_HLW8110_Reg(REG_POWER_PA_ADDR,4);
		delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U32_POWERPA_RegData = (unsigned long)(u8_RxBuf[0]<<24) + (unsigned long)(u8_RxBuf[1]<<16) + (unsigned long)(u8_RxBuf[2]<<8) + (unsigned long)(u8_RxBuf[3]);
#if DEBUG		
		printf("A通道功率寄存器:%lx\n " ,U32_POWERPA_RegData);
#endif				
	}
	else
	{
#if DEBUG
		printf("A通道功率寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	
	
	 if (U32_POWERPA_RegData > 0x80000000)
	{
     b = ~U32_POWERPA_RegData;
     a = (float)b;
	}
	else
     a =  (float)U32_POWERPA_RegData;
     
   
	//功率需要分正功和负功
	//计算,U16_AC_P = (U32_POWERPA_RegData * U16_PowerPAC_RegData)/(2^31*电压系数*电流系数)
	//单位为W
	
	a = a*U16_PowerPAC_RegData;
    a = a/0x80000000;            
		a = a/1;  										// 1 = 电流系数,系数计算可以参考excel资料,1 = current coefficient, coefficient calculation can refer to excel data
		a = a/1;  										// 1 = 电压系数,系数计算可以参考excel资料,1 = voltage coefficient, coefficient calculation can refer to excel data
   	a = (a-0.08) * D_CAL_A_P;						// D_CAL_A_P是校正系数，默认是1((D_ CAL_ A_ P is the correction factor, which is 1 by default) )   
    if(a<0)
	F_AC_P = 0;
	else
	F_AC_P = a;									 	// 单位为W,比如算出来5000.123，表示5000.123W (The unit is w, for example, 5000.123 means 5000.123w)

}

void Read_HLW8110_EA(void)
{
	float a;
	Uart_Read_HLW8110_Reg(REG_ENERGY_PA_ADDR,3); 
	delay_ms(10);
	
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U32_ENERGY_PA_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]);
#if DEBUG		
		printf("A通道有功电量寄存器:%lx\n " ,U32_ENERGY_PA_RegData);
#endif				
	}
	else
	{
#if DEBUG		
		printf("A通道有功电量寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	
	Uart_Read_HLW8110_Reg(REG_HFCONST_ADDR,2); 
	delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_HFConst_RegData = (unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]);
#if DEBUG		
		printf("HFCONST常数 = :%d\n " ,U16_HFConst_RegData);
#endif				
	}
	else
	{
#if DEBUG		
		printf("HFCONST常数寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}

	//电量计算,电量((Electricity quantity)) = (U32_ENERGY_PA_RegData * U16_EnergyAC_RegData * HFCONST) /(K1*K2 * 2^29 * 4096)
	//HFCONST:默认值是0x1000, HFCONST/(2^29 * 4096) = 0x20000000
    a =  (float)U32_ENERGY_PA_RegData;	
   
    a = a*U16_EnergyAC_RegData;
    a = a/0x20000000;             //电量单位是0.001KWH
     
    a = a/1;  										// 1 = 电流系数,系数计算可以参考excel资料,1 = current coefficient, coefficient calculation can refer to excel data
    a = a/1;  										// 1 = 电压系数,系数计算可以参考excel资料,1 = voltage coefficient, coefficient calculation can refer to excel data
	a = a * D_CAL_A_E;     				// D_CAL_A_E是校正系数，免校准应用默认是1 ,D_ CAL_ B_ E is the correction factor. The default value for calibration free applications is 1
    F_AC_E = a+LAST_AC_E;
  
	F_AC_BACKUP_E = F_AC_E;
		
}

void Read_HLW8110_LineFreq(void)
{
	float a;
	unsigned long b;
	Uart_Read_HLW8110_Reg(REG_UFREQ_ADDR,2);
	delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		b = (unsigned long)(u8_RxBuf[0]<<8) + (unsigned long)(u8_RxBuf[1]);
#if DEBUG		
		printf("A通道线性频率寄存器:%ld\n " ,b);
#endif				
	}
	else
	{
#if DEBUG		
		printf("A通道线性频率寄存器读取出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	a = (float)b;
	a = 3579545/(8*a);    
	F_AC_LINE_Freq = a;
}

void Read_HLW8110_PF(void)
{
	float a;
	unsigned long b;
	
	Uart_Read_HLW8110_Reg(REG_PF_ADDR,3);
	delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		b = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]);
#if DEBUG		
		printf("A通道功率因素寄存器:%ld\n " ,b);
#endif				
	}
	else
	{
#if DEBUG		
		printf("读取A通道功率因素寄存器出错\r\n");
#endif				
		B_Read_Error = 1;
	}

	if (b>0x800000)      
	{
		a = (float)(0xffffff-b + 1)/0x7fffff;
	}
	else
	{
		a = (float)b/0x7fffff;
	}
	  
	if (F_AC_P < 0.3) //小于0.3W(Less than 0.3w),Power factor data is inaccurate
	    a = 0; 
	  
	F_AC_PF = a;
  
}

void Read_HLW8110_Angle(void)
{
	float a;	
	unsigned long b;
	Uart_Read_HLW8110_Reg(REG_ANGLE_ADDR,2);
	delay_ms(10);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		b =(unsigned long)(u8_RxBuf[0]<<8) + (unsigned long)(u8_RxBuf[1]);
#if DEBUG		
		printf("A通道线相角寄存器:%ld\n " ,b);
#endif				
	}
	else
	{
#if DEBUG		
		printf("A通道线相角寄存器出错\r\n");
#endif				
		B_Read_Error = 1;
	}
	
	if ( F_AC_PF < 55)	//Linear frequency: 50 Hz,(线性频率50HZ)
	{
		a = b;
		a = a * 0.0805;
		F_Angle = a;
	}
	else
	{
		//Linear frequency: 60 Hz,(线性频率60HZ)
		a = b;
		a = a * 0.0965;
		F_Angle = a;
	}
	
	
	if (F_AC_P < 0.5)		//功率小于0.5时，说明没有负载，相角为0,(When the power is less than 0.5, there is no load and the phase angle is 0)
	{
		F_Angle = 0;
	}
	
	if (F_Angle < 90)
	{
		a = F_Angle;
#if DEBUG		
		printf("电流超前电压:%f\n " ,a);	//Current lead voltage
#endif				
	}
	else if (F_Angle < 180)
	{
		a = 180-F_Angle;
#if DEBUG		
		printf("电流滞后电压:%f\n " ,a);	//Current hysteresis voltage
#endif				
	}
	else if (F_Angle < 360)
	{
		a = 360 - F_Angle;
#if DEBUG		
		printf("电流滞后电压:%f\n " ,a);	//Current hysteresis voltage
#endif				
	}
	else
	{
		a = F_Angle -360;
#if DEBUG		
		printf("电流超前电压:%f\n " ,a);	//Current lead voltage
#endif				
	}
}

void Calculate_HLW8110_MeterData(void)
{

	
	Check_WriteReg_Success();
	
	Read_HLW8110_U();
	Read_HLW8110_PA();
	Read_HLW8110_IA();
	Read_HLW8110_EA();
	
	Read_HLW8110_LineFreq();
	Read_HLW8110_Angle();
	Read_HLW8110_PF();
	Read_HLW8112_State();
	
#if DEBUG	
	printf("\r\n");	//(Insert line feed)
	printf("\r\n");	//(Insert line feed)
	printf("交流测量,uart通讯方式\r\n");
	printf("A通道电流转换系数:%x\n " ,U16_RMSIAC_RegData);
	printf("B通道电流转换系数:%x\n " ,U16_RMSIBC_RegData);
	printf("电压通道转换系数:%x\n " ,U16_RMSUC_RegData);
	printf("A通道功率转换系数:%x\n " ,U16_PowerPAC_RegData);
	printf("B通道功率转换系数:%x\n " ,U16_PowerPBC_RegData);
	printf("视在功率转换系数:%x\n " ,U16_PowerSC_RegData);
	printf("A通道电量转换系数:%x\n " ,U16_EnergyAC_RegData);
	printf("B通道电量转换系数:%x\n " ,U16_EnergyBC_RegData);
	printf("转换系数校验和:%x\n " ,U16_CheckSUM_RegData);
	printf("转换系数计算出的校验和:%x\n " ,U16_CheckSUM_Data);
	
	printf("\r\n");//插入换行(Insert line feed)
	printf("A通道电流寄存器:%lx\n " ,U32_RMSIA_RegData);
	printf("电压寄存器:%lx\n " ,U32_RMSU_RegData);
	printf("A通道功率寄存器:%lx\n " ,U32_POWERPA_RegData);
	printf("A通道电量寄存器:%lx\n " ,U32_ENERGY_PA_RegData);

	printf("\r\n");//插入换行(Insert line feed)
	printf("F_AC_I = %f A \n " ,F_AC_I);						//电流(A)
	printf("F_AC_V = %f V	\n " ,F_AC_V);							//电压(V)
	printf("F_AC_P = %f W	\n " ,F_AC_P);						//功率(P)
	printf("F_AC_BACKUP_E = %f KWH \n " ,F_AC_BACKUP_E);			//电量(Electricity quantity)
	printf("F_AC_LINE_Freq = %f Hz \n " ,F_AC_LINE_Freq);		//市电线性频率(Linear frequency)	
	printf("F_Angle = %f\n " ,F_Angle);					//相角(phase angle)
	printf("F_AC_PF = %f\n " ,F_AC_PF);		//功率因素(Power factor)
	
	printf("\r\n");//插入换行(Insert line feed)
	printf("\r\n");//插入换行(Insert line feed)
	printf("----------------------------------------------\r\n");	
	printf("----------------------------------------------\r\n");
#endif		
}


//*************************************************************************************
//*************************************************************************************
void Set_OVLVL(void)
{
  
  //设置方法,0x5b21,设置210V过压,OVLVL = 0x5a8b
  
  
	Uart_HLW8110_WriteREG_EN();	//打开写8110 Reg
	Uart_Write_HLW8110_Reg (REG_EMUCON2_ADDR,2,0x0fff);
  //打开过压、过流等功能
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x0f);          //电量寄存器读后不清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;
*/
	Uart_Write_HLW8110_Reg (REG_OVLVL_ADDR,2,0x5a8b);
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_OVLVL_ADDR);
  HLW8112_SPI_WriteByte(0x5a);
  HLW8112_SPI_WriteByte(0x8b);
  IO_HLW8112_CS = HIGH;
*/
/*	
	Uart_Read_HLW8110_Reg(REG_OVLVL_ADDR,2);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U32_OVLVL_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]); 
		printf("A通道电流寄存器:%lx\n " ,U32_RMSIA_RegData);
	}
	else
	{
		printf("A通道电流寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
*/
	Uart_Write_HLW8110_Reg (REG_INT_ADDR,2,0x32c9);
  //设置INT寄存器, 电压通道过零输出，INT = 3219，INT2过压输出 
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte(0x32);        
  HLW8112_SPI_WriteByte(0xc9);               
  IO_HLW8112_CS = HIGH;
*/ 
  
  //设置IE寄存器, IE
	Uart_Read_HLW8110_Reg(REG_IE_ADDR,2); 
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_IE_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]); 
		printf("中断配置允许寄存器:%lx\n " ,U16_IE_RegData);
	}
	else
	{
		printf("中断配置允许寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	
	Uart_Write_HLW8110_Reg (REG_IE_ADDR,2,(U16_IE_RegData|0x0200)&0xff);
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x02); //电压过压中断使能，OVIE= 1      
 // HLW8112_SPI_WriteByte(0x02); //电压过压中断使能，OVIE= 1   
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
*/ 
//	U16_TempData = Read_HLW8112_RegData(REG_IE_ADDR,2);
  
	Uart_HLW8110_WriteREG_DIS();	//关闭写8112 Reg
  
  
  
 
}


void Set_underVoltage(void)
{
  
  //设置方法,0x5b21,设置210V过压,OVLVL = 0x5a8b
  
  
	Uart_HLW8110_WriteREG_EN();	//打开写8112 Reg
  	Uart_Write_HLW8110_Reg (REG_SAGCYC_ADDR,2,0x0001);
/*	
	IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_SAGCYC_ADDR);		//欠压设置
  HLW8112_SPI_WriteByte(0x00);
//  HLW8112_SPI_WriteByte(0x05);
	HLW8112_SPI_WriteByte(0x01);
  IO_HLW8112_CS = HIGH;
   U16_TempData = Read_HLW8112_RegData(REG_SAGCYC_ADDR,2);
*/
	Uart_Write_HLW8110_Reg (REG_SAGLVL_ADDR,2,0x4e1c);
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_SAGLVL_ADDR);		//欠压设置180V
  HLW8112_SPI_WriteByte(0x4E);
  HLW8112_SPI_WriteByte(0x1C);
*/	
//	HLW8112_SPI_WriteByte(0x6C);		//欠压设置280V
//	HLW8112_SPI_WriteByte(0x7C);
//  IO_HLW8112_CS = HIGH;
  
//   U16_TempData = Read_HLW8112_RegData(REG_OVLVL_ADDR,2);

  
  //设置INT寄存器, 电压通道过零输出，INT = 3219，INT2欠压输出 
	Uart_Write_HLW8110_Reg (REG_INT_ADDR,2,0x32d9);
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte(0x32);        
  HLW8112_SPI_WriteByte(0xD9);               
  IO_HLW8112_CS = HIGH;
*/  
  
  //设置IE寄存器, IE
  
	Uart_Read_HLW8110_Reg(REG_IE_ADDR,2); 
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_IE_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]); 
		printf("中断配置允许寄存器:%lx\n " ,U16_IE_RegData);
	}
	else
	{
		printf("中断配置允许寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	
	Uart_Write_HLW8110_Reg (REG_IE_ADDR,2,(U16_IE_RegData|0x0800)&0xff);
	
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x08); //电压欠压中断使能，OVIE= 1      
 // HLW8112_SPI_WriteByte(0x02); //电压过压中断使能，OVIE= 1   
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
  
  U16_TempData = Read_HLW8112_RegData(REG_IE_ADDR,2);
*/  
	Uart_HLW8110_WriteREG_DIS();	//关闭写8112 Reg
  
  
  
 
}


void Set_V_Zero(void)
{
	Uart_HLW8110_WriteREG_EN();	//打开写8112 Reg
  	Uart_Write_HLW8110_Reg (REG_SAGCYC_ADDR,2,0x0001);        //打开写8112 Reg
	Uart_Read_HLW8110_Reg(REG_EMUCON_ADDR,2); 
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_EMUCON_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]); 
		printf("计量控制寄存器:%lx\n " ,U16_EMUCON_RegData);
	}
	else
	{
		printf("计量控制寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	
	Uart_Write_HLW8110_Reg (REG_SAGCYC_ADDR,2,(U16_EMUCON_RegData|0x0100)&0xff|0x80);
  //设置EMUCON寄存器,REG_EMUCON_ADDR = REG_EMUCON_ADDR | 0x0180
/*  a = Read_HLW8112_RegData(REG_EMUCON_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x01);          
  HLW8112_SPI_WriteByte((a&0xff)|0x80); // 正向和负向过零点均发生变化，ZXD0 = 1，ZXD1 = 1
  IO_HLW8112_CS = HIGH;
*/ 
  
  //设置EMUCON2寄存器, REG_EMUCON2_ADDR = REG_EMUCON2_ADDR | 0x0024
	Uart_Read_HLW8110_Reg(REG_EMUCON2_ADDR,2); 
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_EMUCON2_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]); 
		printf("计量控制寄存器2:%x\n " ,U16_EMUCON2_RegData);
	}
	else
	{
		printf("计量控制寄存器2读取出错\r\n");
		B_Read_Error = 1;
	}
	
	Uart_Write_HLW8110_Reg (REG_SAGCYC_ADDR,2,U16_EMUCON_RegData&0xffff|0x24);
/*  a = Read_HLW8112_RegData(REG_EMUCON2_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(a>>8);          
  HLW8112_SPI_WriteByte((a&0xff)|0x24); // ZxEN = 1,WaveEn = 1;
  IO_HLW8112_CS = HIGH;
*/    
  

  //设置IE寄存器
	Uart_Read_HLW8110_Reg(REG_IE_ADDR,2); 
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_IE_RegData = (unsigned long)(u8_RxBuf[0]<<16) + (unsigned long)(u8_RxBuf[1]<<8) + (unsigned long)(u8_RxBuf[2]); 
		printf("中断配置允许寄存器:%x\n " ,U16_IE_RegData);
	}
	else
	{
		printf("中断配置允许寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	
	Uart_Write_HLW8110_Reg (REG_IE_ADDR,2,(U16_IE_RegData|0x4000)&0xff);
/*  a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x40); //电压过零中断使能，ZX_UIE = 1         
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
*/
  //设置INT寄存器, 电压通道过零输出，INT = 3219,INT1输出电压过零
  //a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
	Uart_Write_HLW8110_Reg (REG_INT_ADDR,2,0x3219);
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte(0x32);        
  HLW8112_SPI_WriteByte(0x19);               
  IO_HLW8112_CS = HIGH;
*/  
  
  
  Uart_HLW8110_WriteREG_DIS();	//关闭写8112 Reg
}


void Set_Leakage(void)
{
	Uart_Read_HLW8110_Reg(REG_RIF_ADDR,2); 
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		REG_RIF_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
		printf("复位中断状态寄存器:%x\n " ,REG_RIF_RegData);
	}
	else
	{
		printf("复位中断状态寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	
	Uart_HLW8110_WriteREG_EN();	//打开写8112 Reg
	
//	Uart_Read_HLW8110_Reg(REG_EMUCON2_ADDR,2);
//	delay_ms(50);	
//	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
//	{
//		U16_EMUCON2_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
//		printf("计量控制寄存器2:%x\n " ,U16_EMUCON2_RegData);
//	}
//	else
//	{
//		printf("计量控制寄存器2读取出错\r\n");
//		B_Read_Error = 1;
//	}
//	
//	Uart_Write_HLW8110_Reg (REG_EMUCON2_ADDR,2,U16_EMUCON2_RegData|0x002A);
//	
//	Uart_Read_HLW8110_Reg(REG_EMUCON2_ADDR,2);
//	delay_ms(50);
//	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
//	{
//		U16_EMUCON2_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
//		printf("计量控制寄存器2:%x\n " ,U16_EMUCON2_RegData);
//	}
//	else
//	{
//		printf("计量控制寄存器2读取出错\r\n");
//		B_Read_Error = 1;
//	}
//  	Uart_Write_HLW8110_Reg (REG_SYSCON_ADDR,2,0x0a04);
	
	Uart_Write_HLW8110_Reg (REG_OIALVL_ADDR,2,0xADC0);//0xADC0
	Uart_Read_HLW8110_Reg(REG_OIALVL_ADDR,2);
	delay_ms(50);	
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_OIALVL_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
		printf("过流阈值寄存器:%x\n " ,U16_OIALVL_RegData);
	}
	else
	{
		printf("过流阈值寄存器读取出错\r\n");
		B_Read_Error = 1;
	}


/*
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_SYSCON_ADDR);
  HLW8112_SPI_WriteByte(0x0a);          //-------------高8bit，关闭ADC电流通道B
  HLW8112_SPI_WriteByte(0x04);          //-------------低8bit，
  IO_HLW8112_CS = HIGH;
*/
  //设置comp_off = 1,打开B通道比较器
//	Uart_Read_HLW8110_Reg(REG_EMUCON_ADDR,2);  
//	delay_ms(50);
//	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
//	{
//		U16_EMUCON_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
//		printf("计量控制寄存器:%x\n " ,U16_EMUCON_RegData);
//	}
//	else
//	{
//		printf("计量控制寄存器读取出错\r\n");
//		B_Read_Error = 1;
//	}
//	
//	Uart_Write_HLW8110_Reg (REG_EMUCON_ADDR,2,U16_EMUCON_RegData&0xCFFD);
//	
//	Uart_Read_HLW8110_Reg(REG_EMUCON_ADDR,2);  
//	delay_ms(50);
//	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
//	{
//		U16_EMUCON_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
//		printf("计量控制寄存器:%x\n " ,U16_EMUCON_RegData);
//	}
//	else
//	{
//		printf("计量控制寄存器读取出错\r\n");
//		B_Read_Error = 1;
//	}
	
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON_ADDR);  
  HLW8112_SPI_WriteByte((a>>8)&0xcf);   //   打开比较器    
  HLW8112_SPI_WriteByte(a&0xfd);  
  IO_HLW8112_CS = HIGH;
*/  
 
/*	a = Read_HLW8112_RegData(REG_INT_ADDR,2); 
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte((a>>8));        
  HLW8112_SPI_WriteByte(0x29);               
  IO_HLW8112_CS = HIGH;
*/  
  
  //设置IE寄存器, IE
	Uart_Write_HLW8110_Reg (REG_IE_ADDR,2,0x8080);
	
	Uart_Read_HLW8110_Reg(REG_IE_ADDR,2); 
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_IE_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
		printf("中断配置允许寄存器:%x\n " ,U16_IE_RegData);
	}
	else
	{
		printf("中断配置允许寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	
	  
  //设置INT寄存器, INT2比较器漏电输出
//	Uart_Write_HLW8110_Reg (REG_INT_ADDR,2,0x2E);
	Uart_Write_HLW8110_Reg (REG_INT_ADDR,2,0x322E);
	Uart_Read_HLW8110_Reg(REG_INT_ADDR,2);  
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		U16_INT_RegData = (unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
		printf("中断输出寄存器:%x\n " ,U16_INT_RegData);
	}
	else
	{
		printf("中断输出寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	

	
/*	
  a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x80); //漏电中断使能，LeakageIE= 1        
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
  
  U16_TempData = Read_HLW8112_RegData(REG_IE_ADDR,2);
*/  
	
	Uart_Read_HLW8110_Reg(REG_RIF_ADDR,2); 
	delay_ms(50);
	if ( u8_RxBuf[u8_RX_Length-1] == HLW8110_checkSum_Read(u8_RX_Length) )
	{
		REG_RIF_RegData =(unsigned int)(u8_RxBuf[0]<<8) + (unsigned int)(u8_RxBuf[1]); 
		printf("复位中断状态寄存器:%x\n " ,REG_RIF_RegData);
	}
	else
	{
		printf("复位中断状态寄存器读取出错\r\n");
		B_Read_Error = 1;
	}
	Uart_HLW8110_WriteREG_DIS();	//关闭写8112 Reg

}


void Read_HLW8112_State(void)
{
	Uart_Read_HLW8110_Reg(REG_IF_ADDR,2);
	Uart_Read_HLW8110_Reg(REG_RIF_ADDR,2);

}


#endif
