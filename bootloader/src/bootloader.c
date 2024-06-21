#include "common-defines.h"
#include <libopencm3/stm32/memorymap.h> //for FLASH_BASE
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "core/uart.h"
#include "core/ring_buffer.h"
#include "comms_packet.h"
#include "core/system.h"

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

static void gpio_setup(void){
    //uart setup
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(UART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, UART_TX_PIN | UART_RX_PIN); //change mode to alternate function
    gpio_set_af(UART_PORT, GPIO_AF7,UART_TX_PIN | UART_RX_PIN);
}

//const uint8_t bloater[0x8000] = {0}; //this will get discarded if we don't use it anywhere. check memory map
//volatile doesn't mean that it wont't get removed. Just won't get optimized
//the const is doing something very imp here. without  it, the linker will not give a size overflow error

//volatile uint8_t x = 0; //volatile ensures no compiler optimization
int main(void){  //void is needed here. why

    // for(uint32_t i=0;i<0x8000;i++){
    //     x+=bloater[i];
    // }

    system_setup();
    uart_setup();
    comms_setup();
    gpio_setup();

    comms_packet_t packet = {
        .length = 1,
        //.data = {1,2,3,4,5,6,7,8,9,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
        .data = {RETX_PACKET_DATA0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
        .crc = 0
    };

    packet.crc = comms_compute_crc(&packet);

    while(true){
        comms_update();
        comms_write(&packet);
        system_delay(500);
    }

    //need to disable all things we setup at the start of code
    execute_main();

    return 0;
}

