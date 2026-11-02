#include "key.h"
#include "delay.h"

u8 Key_Scan() {
    u8 key_val = 0;
    
    // K3 -> P3.2 (控制灯)
    if (P3_2 == 0) {
        Delay(10);
        if (P3_2 == 0) {
            key_val = 1;
            while (P3_2 == 0); // 等待松手
        }
    }
    
    // K4 -> P3.3 (控制风扇)
    if (P3_3 == 0) {
        Delay(10);
        if (P3_3 == 0) {
            key_val = 2;
            while (P3_3 == 0); // 等待松手
        }
    }
    
    return key_val;
}
