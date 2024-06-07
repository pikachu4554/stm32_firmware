#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define LED_PORT (GPIOD)
#define LED_PIN (GPIO15)


//static means that this function is available only in this translation unit and not outside it
static void self_rcc_setup(void){ //dont share name prefixes with HAL
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

static void self_gpio_setup(void){
    //every peripheral, including GPIOs are off by default, probably as a power saving technique. It is turned off due to the fact that there is no clock
    //being passed to it. We can turn it on by enabling the clock to it.
    rcc_periph_clock_enable(RCC_GPIOD); //each port can be manually be turned on or off by clock enabling and disabling
    //each pin can have a pullup or pulldown resistor, to give a default value rather than be in a floating state
    //gpio5 is 1 shifted up by 5 positions. can use or to give multpile pins. gpio5 | gpio6
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

static void delay_cycles(uint32_t cycles){
    for(uint32_t i = 0;i<cycles;i++){
        __asm__("nop"); //needed as compiler may remove this code. We may need to add more lines for more advanced compilers.
    }
}

int main(void){

    self_rcc_setup(); 
    self_gpio_setup();  

    while(1){
        gpio_toggle(LED_PORT, LED_PIN);
        delay_cycles(84000000/4); //4 as there are approx 4 intructins that need to be executed per loop iteration, including comparion, incremneting etc.
    }
    return 0;
}