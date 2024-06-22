#ifndef INC_BL_FLASH
#define INC_BL_FLASH

#include "common-defines.h"

void bl_flash_memory_erase(void);
void bl_flash_memory_write(uint32_t address, const uint8_t *data, const uint32_t length);

#endif