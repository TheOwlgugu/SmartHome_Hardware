#include <REGX52.H>
#include "common.h"
#include "uart.h"
#include "esp8266.h"
#include "device.h"
#include "key.h"
#include "delay.h"

/* ====== 硬件验证开关 ======
   打开这一行：跳过 ESP8266 的 AT 配置（省掉十几秒开机等待），
               只验证 K3->D3、K4->风扇。此时串口照常输出 sXX，
               可以直接用板载 USB 转 TTL 在电脑串口助手里看（9600）。
   注释掉：    正常模式，配置 ESP 并开 TCP Server。

   注意：这一行打开时，Keil 会报 "WARNING L16: UNCALLED SEGMENT
   ... ESP_ATSETUP"。那是正常的，正好提醒你现在是测试模式。    */
/* #define HW_TEST_ONLY */
/* ========================== */

void main() {
    u8 key = 0;
    u8 cmd = 0;

    /* 1. 初始化。先关外设再开串口，避免上电瞬间蜂鸣器/风扇乱动 */
    Device_Init();
    UART_Init();
    ESP_Init();

#ifndef HW_TEST_ONLY
    /* 2. 配置 ESP8266：开 TCP Server */
    ESP_ATSetup();
#endif

    /* 3. 上报一次初始状态 s00 */
    Device_Report();

    /* 4. 主循环：按键和手机指令是完全对等的两个输入源 */
    while (1) {
        key = Key_Scan();
        if (key != 0) {
            Device_Toggle(key);
        }

        cmd = ESP_GetCommand();
        if (cmd != 0) {
            Device_Toggle(cmd);
        }
    }
}
