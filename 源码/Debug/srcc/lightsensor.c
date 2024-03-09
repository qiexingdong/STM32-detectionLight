#include "lightsensor.h"

void LightSensor_Init(void)
{
	gpio_init(LightSensor_DO,GPIO_INPUT,0);
	gpio_set(LightSensor_DO,0);
}

uint8_t LightSensor_Get(void)
{
	return gpio_get(LightSensor_DO);
}