#include "buzz.h"

void Buzz_Init(void)
{
    gpio_init(Buzz,GPIO_OUTPUT,1);
}
void Buzz_On(void)
{
    gpio_set(Buzz,0);
}

void Buzz_Off(void)
{
    gpio_set(Buzz,1);
}

