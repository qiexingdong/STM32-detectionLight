#include "light.h"


void Light_Init(void)
{
    gpio_init(Light_1,GPIO_OUTPUT,1);
    gpio_init(Light_2,GPIO_OUTPUT,1);
}
void Light_1_On(void)
{
    gpio_set(Light_1,0);
}
void Light_1_Off(void)
{
    gpio_set(Light_1,1); 
}
void Light_2_On(void)
{
    gpio_set(Light_2,0);
}
void Light_2_Off(void)
{
    gpio_set(Light_2,1); 
}
