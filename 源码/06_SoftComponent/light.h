#ifndef  LIGHT_H       
#define  LIGHT_H

#include "gpio.h"

#define Light_1 (PTC_NUM|8) //3号引脚 用来测光敏电阻
#define Light_2 (PTC_NUM|7) //4号引脚 用来测热敏电阻

void Light_Init(void);
void Light_1_On(void);
void Light_1_Off(void);
void Light_2_On(void);
void Light_2_Off(void);

#endif


