#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "core/system.h"

#define LED_PORT (GPIOD)
#define LED_PIN (GPIO12)

static void self_gpio_setup(void){
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

int main(void){
    system_setup(); //rcc and systick config in core/system.c
    self_gpio_setup(); 
    uint64_t start_time = system_get_ticks(); //define this as we might be using the ticks varible for various different tasks and they might start at differnt times
    while(1){
        if(system_get_ticks()-start_time >=500){
            gpio_toggle(LED_PORT, LED_PIN);
            start_time = system_get_ticks(); //without this, the blinking is very fast as it appears to be always on at a lower brightness level.
        }
        //do useful work here.
    }
    return 0;
}