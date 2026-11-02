#include "device.h"
#include "led.h"
#include "motor.h"
#include "esp8266.h"

static bit light_state = 0;

void Device_Init() {
    LED_Init();
    Motor_Init();
    light_state = 0;
}

/* 统一向手机上报当前状态 */
void Device_Report() {
    ESP_SendStatus(light_state, Motor_GetState());
}

/* 翻转状态 -> 驱动硬件 -> 上报。
   板上按键和手机指令都走这里，所以两边永远同步。

   dev_id: 1=灯  2=风扇  3=只查询（不改状态，仅回一条 sXX）

   3 是给手机心跳用的：App 每 5 秒发一次 i0，51 回一条当前状态。
   这样 App 刚连上就能立刻拿到真实状态，丢包也能自动校正。      */
void Device_Toggle(u8 dev_id) {
    if (dev_id == 1) {
        light_state = !light_state;
        LED_Control(light_state);
    }
    else if (dev_id == 2) {
        Motor_Control(!Motor_GetState());
    }
    else if (dev_id == 3) {
        /* 查询：什么都不改，直接往下走去上报 */
    }
    else {
        return;             /* 非法编号，不上报 */
    }

    Device_Report();
}
