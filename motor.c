#include "motor.h"

/* 风扇 = 板载 ULN2003(U14) 的第 1 路。
   原理图：U14 输入 IN1~IN4 = P1.0~P1.3，输出 OUT1~OUT4 接到步进电机座 J47。
   接线：马达正极 -> J47 的 VCC，马达负极 -> J47 的 OUT1。
   P1.0 = 1 时 ULN2003 把 OUT1 拉到地，马达通电转动。
   ULN2003 单路可灌 500mA 且内部自带续流二极管，不用再外接任何元件。

   切勿把马达直接挂在单片机 IO 上：51 的拉电流只有几十 uA，
   且马达断电瞬间的反电动势会击穿 IO 口。                           */
sbit FAN = P1^0;

/* 蜂鸣器挂在 ULN2003 的第 5 路，输入是 P1.5。
   这里必须主动拉低，否则上电后 P1 默认为高，蜂鸣器会一直叫。      */
sbit BEEP = P1^5;

static bit fan_state = 0;

void Motor_Init() {
    BEEP = 0;
    FAN  = 0;
    fan_state = 0;
}

void Motor_Control(bit state) {
    fan_state = state;
    FAN = state;
}

bit Motor_GetState() {
    return fan_state;
}
