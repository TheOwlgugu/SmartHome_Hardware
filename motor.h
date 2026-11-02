#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <REGX52.H>

void Motor_Init();
void Motor_Control(bit state); /* 1¿ª 0¹Ø */
bit  Motor_GetState();

#endif
