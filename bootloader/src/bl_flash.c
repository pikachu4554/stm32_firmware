#include <libopencm3/stm32/flash.h>
#include "common-defines.h"
#include "bl_flash.h"

#define MAIN_APP_SECTOR_START (2)
#define MAIN_APP_SECTOR_END (7)

void bl_flash_memory_erase(void){

    //write values into registers
    flash_unlock(); 
    for(uint8_t sector = MAIN_APP_SECTOR_START; sector <= MAIN_APP_SECTOR_END; sector++){
        flash_erase_sector(sector, FLASH_CR_PROGRAM_X32); //32 bit paraallel erase
        //when we clear flash, it goes to logic 1
    }
    flash_lock();
}

void bl_flash_memory_write(uint32_t address, const uint8_t *data, const uint32_t length){
    flash_unlock();
    flash_program(address, data, length); //does not do paralleleism
    flash_lock();
}