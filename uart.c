#include "uart.h"

void (*UART_RxCallback)(u8 dat) = 0;

/* 发送完成标志。由串口中断置位，UART_SendByte 等它。
   这样发送时不必关掉接收中断，收发互不干扰。        */
static volatile bit tx_done = 0;

void UART_Init() {
    SCON = 0x50;  /* 方式1，8位数据，允许接收 */
    TMOD &= 0x0F;
    TMOD |= 0x20; /* 定时器1 方式2 自动重装，做波特率发生器 */
    TH1 = 0xFD;   /* 9600bps @ 11.0592MHz */
    TL1 = 0xFD;
    TR1 = 1;
    ES  = 1;
    EA  = 1;
}

void UART_SendByte(u8 dat) {
    tx_done = 0;
    SBUF = dat;         //串口数据缓冲器：写它 = 发送这个字节；读它 = 取出刚收到的字节
    while (!tx_done);   /* TI 由中断清除并置 tx_done */
}

void UART_SendString(char *s) {
    while (*s != '\0') {
        UART_SendByte(*s++);
    }
}

void UART_ISR() interrupt 4 {
    if (RI) {
        RI = 0;
        if (UART_RxCallback != 0) {
            UART_RxCallback(SBUF);
        }
    }
    if (TI) {
        TI = 0;         /* 必须在中断里清，否则 TI 一直挂着反复触发中断 */
        tx_done = 1;
    }
}
