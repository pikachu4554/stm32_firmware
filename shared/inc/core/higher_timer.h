#ifndef INC_HIGHER_TIMER
#define INC_HIGHER_TIMER

#include "common-defines.h"

typedef struct h_timer_config{
    uint64_t wait_time;
    uint64_t target_time;
    bool auto_reset;
    bool has_elapsed_once;
} h_timer_config_t;

void h_timer_setup(h_timer_config_t *timer, uint64_t wait_time, bool auto_reset);
bool h_timer_has_elapsed(h_timer_config_t *timer);
void h_timer_reset(h_timer_config_t *timer);

#endif 