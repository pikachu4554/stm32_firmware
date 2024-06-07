#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h> //contains dec. for the systick periph
#include <libopencm3/cm3/vector.h> //contains dec. for the IVT and the interrupt handlers

#include "core/system.h"
#include "common-defines.h"

static void self_rcc_setup(void){  //self defined funcition
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

//volatile to prevent the compiler from doing smart stuff with it that may break it.
static volatile uint64_t ticks = 0;

void sys_tick_handler(void){  //prototype available from the nvic.h header
    ticks++; 
} 

static void self_systick_setup(void){
    //basically sets up the systick peripheral
    systick_set_frequency(SYSTICK_FREQ,CPU_FREQ);
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

uint64_t system_get_ticks(void){
    return ticks;
}

void system_setup(void){
    self_rcc_setup();
    self_systick_setup();
}