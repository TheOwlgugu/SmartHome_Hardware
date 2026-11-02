#include "led.h"
                                       
sbit LIGHT = P2^0;//灯光的位置

void LED_Init() {

    P0 = 0x00;//负责拉低74HC138译码器电平，避免乱闪

    LIGHT = 1;  /* 默认灭 */
}

void LED_Control(bit state) {
    LIGHT = !state;
	//控制开关灯
}
