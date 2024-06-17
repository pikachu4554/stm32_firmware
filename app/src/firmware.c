#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/scb.h>

#include "core/system.h"
#include "timer.h"
#include "core/uart.h"

#define LED_PORT (GPIOA)
#define LED_PIN (GPIO5)
#define BOOTLOADER_SIZE (0x8000U)
#define UART_PORT (GPIOA)
#define UART_TX_PIN (GPIO2)
#define UART_RX_PIN (GPIO3)

static void self_gpio_setup(void){
    //led setup
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(LED_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED_PIN); //change mode to alternate function
    gpio_set_af(LED_PORT, GPIO_AF1,LED_PIN);

    //uart setup
    gpio_mode_setup(UART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, UART_TX_PIN | UART_RX_PIN); //change mode to alternate function
    gpio_set_af(UART_PORT, GPIO_AF7,UART_TX_PIN | UART_RX_PIN);
}

static void vector_setup(void){
    SCB_VTOR = BOOTLOADER_SIZE;
}

int main(void){
    vector_setup(); //offset the vector table for this application
    system_setup(); //rcc and systick config in core/system.c
    self_gpio_setup(); 
    self_timer_setup();
    uart_setup();

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
        
        if(uart_data_available()){
            uint8_t data = uart_read_byte();
            uart_write_byte(data+1);
        }
    }
    return 0;
}