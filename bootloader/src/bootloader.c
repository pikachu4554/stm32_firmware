#include "common-defines.h"
#include <libopencm3/stm32/memorymap.h> //for FLASH_BASE
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "core/uart.h"
#include "core/ring_buffer.h"
#include "comms_packet.h"
#include "core/system.h"
#include "bl_flash.h"

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

    uint8_t data[1024];

    for(uint16_t i =0; i<1024;i++){
        data[i] = i & 0xff;
    }

    //if this is in the while loop, then there will always be a memoery operation in progress and we cannot connect the 
    //debugger, stmcubeprogrammer, or any other software that requires to read memory

    //at a time, only one software can access the flash of the stm32. So, when we run debugger, then st-info --probe and stmCubeProgrammer cannot get st-link
    //parameters. Is this a limitaiotn of mempry access mechanism or the st-link builtin to the chip?
    bl_flash_memory_erase();
    bl_flash_memory_write(0x08008000, data, 1024);
    bl_flash_memory_write(0x0800C000, data, 1024);
    bl_flash_memory_write(0x08010000, data, 1024);
    bl_flash_memory_write(0x08020000, data, 1024);
    bl_flash_memory_write(0x08040000, data, 1024);
    bl_flash_memory_write(0x08060000, data, 1024);

    while(true){
        
    }

    //need to disable all things we setup at the start of code
    execute_main();

    return 0;
}

