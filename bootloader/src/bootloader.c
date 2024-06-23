#include "common-defines.h"
#include <libopencm3/stm32/memorymap.h> //for FLASH_BASE
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "core/uart.h"
#include "core/ring_buffer.h"
#include "comms_packet.h"
#include "core/system.h"
#include "bl_flash.h"
#include "core/higher_timer.h"

#define BOOTLOADER_SIZE (0x8000U) //U ensures that the number is always treated as an unsigned number. operations convert this define to a signed number
#define MAIN_FIRMWARE_START_ADDRESS (FLASH_BASE + BOOTLOADER_SIZE)

#define UART_PORT (GPIOA)
#define UART_TX_PIN (GPIO2)
#define UART_RX_PIN (GPIO3)

static void execute_main(void){ //static is neede here. why?
    typedef void (*main_fn)(void);
    uint32_t *reset_vector_entry = (uint32_t *)(MAIN_FIRMWARE_START_ADDRESS + 4U);
    uint32_t *reset_vector = (uint32_t *)(*reset_vector_entry);
    main_fn main_fn_reset_vector=(main_fn)reset_vector;

    main_fn_reset_vector();
}

int main(void){  //void is needed here. why

    system_setup();
    h_timer_config_t timer;
    h_timer_config_t timer2;
    h_timer_setup(&timer, 1000, false);
    h_timer_setup(&timer2, 2000, true);

    while(true){
        if(h_timer_has_elapsed(&timer)){
            volatile uint32_t x =0;
            x++;
        }

        if(h_timer_has_elapsed(&timer2)){
            h_timer_reset(&timer);
        }
    }

    //need to disable all things we setup at the start of code
    execute_main();

    return 0;
}

