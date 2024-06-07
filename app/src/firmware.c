#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h> //contains dec. for the systick periph
#include <libopencm3/cm3/vector.h> //contains dec. for the IVT and the interrupt handlers


#define LED_PORT (GPIOD)
#define LED_PIN (GPIO15)
#define SYSTICK_FREQ (1000) //counted in ms. Proabably sets the counter value
#define AHB_FREQ (84000000) //AHB operates at CPU freq

// //static means that this function is available only in this translation unit and not outside it
// static void self_rcc_setup(void){ //dont share name prefixes with HAL
//     rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
// }

// static void self_gpio_setup(void){
//     //every peripheral, including GPIOs are off by default, probably as a power saving technique. It is turned off due to the fact that there is no clock
//     //being passed to it. We can turn it on by enabling the clock to it.
//     rcc_periph_clock_enable(RCC_GPIOD); //each port can be manually be turned on or off by clock enabling and disabling
//     //each pin can have a pullup or pulldown resistor, to give a default value rather than be in a floating state
//     //gpio5 is 1 shifted up by 5 positions. can use or to give multpile pins. gpio5 | gpio6
//     gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
// }

// static void delay_cycles(uint32_t cycles){
//     for(uint32_t i = 0;i<cycles;i++){
//         __asm__("nop"); //needed as compiler may remove this code. We may need to add more lines for more advanced compilers.
//     }
// }

// int main(void){

//     self_rcc_setup(); 
//     self_gpio_setup();  

//     while(1){
//         gpio_toggle(LED_PORT, LED_PIN);
//         delay_cycles(84000000/4); //4 as there are approx 4 intructins that need to be executed per loop iteration, including comparion, incremneting etc.
//     }
//     return 0;
// }

// updated firmware to use systick rather than using the cpu to count the time. Basicallhy trying to avoid the busy while loop.

static void self_rcc_setup(void){
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

static void self_gpio_setup(void){
    
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

static void self_systick_setup(void){
    //basically sets up the systick peripheral
    systick_set_frequency(SYSTICK_FREQ,AHB_FREQ);
    systick_counter_enable(); //systick is disabled by default
    //now, when the systick counter reaches its target, it interrupts the CPU. The interrupt vector is stored as
    //a function pointer. There is a very particular layout of IV in the IVT that has to be followed as the 
    //processor expects to find these in that particular location. libopnencm3 follows the rules specified int the ref manual. 
    //Check the refereence manual for more info.
    //also, interuupts are diabled by default, and must be enabled.
    //by default the interrupt handler is set to a null handler that does nothing.
    //it uses something called a pragma to map the function pointer of systick to the null handler. The pragma is weak,
    //which means that we can override the base definition using our own custom implementation in this file.
    systick_interrupt_enable();
}

//volatile to prevent the compiler from doing smart stuff with it that may break it.
volatile uint64_t ticks = 0;
void sys_tick_handler(void){  //prototype available from the nvic.h header
    ticks++; 
} 

static uint64_t get_ticks(void){
    return ticks;
}


int main(void){

    self_rcc_setup(); 
    self_gpio_setup(); 
    self_systick_setup();
    uint64_t start_time = get_ticks(); //define this as we might be using the ticks varible for various different tasks and they might start at differnt times
    while(1){
        if(get_ticks()-start_time >=500){
            gpio_toggle(LED_PORT, LED_PIN);
            start_time = get_ticks(); //without this, the blinking is very fast as it appears to be always on at a lower brightness level.
        }
        //do useful work here.
    }
    return 0;
}