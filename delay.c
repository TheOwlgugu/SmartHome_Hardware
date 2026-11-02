#include "delay.h"

void Delay(u16 ms) {
    u8 i, j;
    while (ms--) {
        i = 2;
        j = 239;
        do {
            while (--j);
        } while (--i);
    }
}
