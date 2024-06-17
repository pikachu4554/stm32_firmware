#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>

#include "timer.h"

// freq = sys_freq / ((prescaler-1)*(ARR-1))
//aiming at 1000 steps and 1000Hz. based on some cals, we will get approx this target due to the -1 in the formula

#define PRESCALER (84)
#define ARR_VALUE (1000)

void self_timer_setup(void){
    
    //enable timer
    rcc_periph_clock_enable(RCC_TIM2); //TIM4 as it is according to the datasheet

    //set up timer properties like clock division, alignment, counter count direction, 
    timer_set_mode(TIM2,TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    //configure timer for PWM. set the channel and the mode for channel.
    timer_set_oc_mode(TIM2,TIM_OC1, TIM_OCM_PWM1); //channel1 for PD12 according to chip datasheet

    //enable timer counter to start counting
    timer_enable_counter(TIM2);

    //enable output compare for channel
    timer_enable_oc_output(TIM2, TIM_OC1);

    //setup frequency and resolution
    timer_set_prescaler(TIM2, PRESCALER - 1);
    timer_set_period(TIM2, ARR_VALUE - 1);
}

void timer_pwm_set_duty_cycle(float duty_cycle){  //b/w 1 and 100
//duty cycle = ccr/arr * 100

const float raw_value = (float)ARR_VALUE*(duty_cycle/100.0f);
timer_set_oc_value(TIM2, TIM_OC1, (uint32_t)raw_value); //will lose accuracy here tho

}