#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H

#include "common-defines.h"
#define SYSTICK_FREQ (1000) //counted in ms. Proabably sets the counter value
#define CPU_FREQ (84000000) //AHB operates at CPU freq

void system_setup(void);
uint64_t system_get_ticks(void);

#endif