#include "common-defines.h"
#include <libopencm3/stm32/memorymap.h> //for FLASH_BASE

#define BOOTLOADER_SIZE (0x8000U) //U ensures that the number is always treated as an unsigned number. operations convert this define to a signed number
#define MAIN_FIRMWARE_START_ADDRESS (FLASH_BASE + BOOTLOADER_SIZE)

static void execute_main(void){ //static is neede here. why?
    typedef void (*main_fn)(void);
    uint32_t *reset_vector_entry = (uint32_t *)(MAIN_FIRMWARE_START_ADDRESS + 4U);
    uint32_t *reset_vector = (uint32_t *)(*reset_vector_entry);
    main_fn main_fn_reset_vector=(main_fn)reset_vector;

    main_fn_reset_vector();
}

//const uint8_t bloater[0x8000] = {0}; //this will get discarded if we don't use it anywhere. check memory map
//volatile doesn't mean that it wont't get removed. Just won't get optimized
//the const is doing something very imp here. without  it, the linker will not give a size overflow error

//volatile uint8_t x = 0; //volatile ensures no compiler optimization
int main(void){  //void is needed here. why

    // for(uint32_t i=0;i<0x8000;i++){
    //     x+=bloater[i];
    // }
    execute_main();

    return 0;
}