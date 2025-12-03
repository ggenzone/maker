#include <msp430.h>
#include "led.h"

int main(void) {
    volatile unsigned int i;

    WDTCTL = WDTPW + WDTHOLD;

    led_init();

    while(1) {
        led_toggle();
        for(i=10000; i>0; i--);
    }
}
