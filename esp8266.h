#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "common.h"

/* ====== WiFi 配置方式 ======

   注释掉（默认）：51 只发 CIPMUX / CIPSERVER，WiFi 靠 ESP 自己 flash 里
       存着的配置自动重连。AT 1.7.4 的 CWMODE/CWJAP 默认就是写 flash 的，
       所以你用串口助手手动配过一次之后就一直有效。
       好处：开机快，热点密码不进固件。
       换 WiFi 时：用串口助手重发一次 AT+CWJAP 即可，不用重烧 51。

   打开 ESP_CONFIG_WIFI：51 开机时自己发 CWMODE/CWJAP 连 WiFi。
       好处：自包含，ESP 换一个也能直接用。
       代价：开机要多等十几秒，而且密码明文躺在固件里。     */
/* #define ESP_CONFIG_WIFI */

#define ESP_WIFI_SSID   "ggjdsj"
#define ESP_WIFI_PASS   "ggjdsj233"
#define ESP_TCP_PORT    "8080"
/* ============================ */

void ESP_Init();        /* 挂接收回调，必须在 UART_Init 之后调用 */
void ESP_ATSetup();     /* 发 AT 指令开 TCP Server，开机调一次 */

void ESP_SendStatus(u8 light, u8 fan);  /* 上报 sXX */
u8   ESP_GetCommand();   /* 取手机下发的指令：0无 / 1灯 / 2风扇，取走即清 */

void ESP_ReceiveHandler(u8 rx_data);    /* 内部用，串口中断回调 */

#endif
