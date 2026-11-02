#ifndef __UART_H__
#define __UART_H__

#include <REGX52.H>
#include "common.h"

void UART_Init();
void UART_SendByte(u8 dat);
void UART_SendString(char *s);

/* 接收回调：串口中断里每收到一个字节就调一次，必须写得很短 */
extern void (*UART_RxCallback)(u8 dat);

#endif
