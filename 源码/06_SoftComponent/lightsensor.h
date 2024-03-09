#ifndef  LIGHTSENSOR_H       
#define  LIGHTSENSOR_H

#include "gpio.h"

#define LightSensor_DO (PTC_NUM|9) //2ºÅÒý½Å

void LightSensor_Init(void);
uint8_t LightSensor_Get(void);


#endif



