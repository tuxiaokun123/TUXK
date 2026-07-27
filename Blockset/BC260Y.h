#ifndef __BC260Y_H
#define __BC260Y_H

uint8_t Send_Cmd(uint8_t *cmd, char *recdata);
void Serial_SendByte(uint8_t Byte);
void Serial_SendString(unsigned char *String);
uint8_t Send_Cmd(uint8_t *cmd, char *recdata);
void NB_Rec_Handle(void);
void BC260Y_Init(void);
void Data_Processing(uint8_t Pr_ID);
void Clear_Buf_CMD(uint8_t *Buf,uint8_t length);
void BC260Y_Notify(uint8_t begin_sign,uint8_t end_sign);
void Wait_Onenet_Response(uint16_t time);
void Connect_Onenet_Init(void);
void Notify_Control(uint8_t state);
void Signal_Value_Check(void);

#endif
