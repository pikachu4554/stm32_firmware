#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/scb.h>

#include "core/system.h"
#include "timer.h"

#define LED_PORT (GPIOD)
#define LED_PIN (GPIO12)
#define BOOTLOADER_SIZE (0x8000U)

static void self_gpio_setup(void){
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_mode_setup(LED_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED_PIN); //change mode to alternate function
    gpio_set_af(LED_PORT, GPIO_AF2,LED_PIN);
}

static void vector_setup(void){
    SCB_VTOR = BOOTLOADER_SIZE;
}

int main(void){
    vector_setup(); //offset the vector table for this application
    system_setup(); //rcc and systick config in core/system.c
    self_gpio_setup(); 
    self_timer_setup();
    uint64_t start_time = system_get_ticks();
    float duty_cycle = 0.0f;
    timer_pwm_set_duty_cycle(duty_cycle);
    while(1){
        if(system_get_ticks()-start_time >=10){
            duty_cycle+=1.0f;
            if(duty_cycle>100.0f){
                duty_cycle=0.0f;
            }
            timer_pwm_set_duty_cycle(duty_cycle);
            start_time = system_get_ticks(); 
        }
        
    }
    return 0;
}