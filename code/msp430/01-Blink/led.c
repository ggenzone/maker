#include <msp430.h>
#include "led.h"

void led_init()
{
    P1DIR |= 0x01;
}

void led_toggle()
{
    P1OUT ^= 0x01;
}
