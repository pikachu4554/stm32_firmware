#ifndef INC_RING_BUFFER_H
#define INC_RING_BUFFER_H

#include "common-defines.h"

typedef struct ring_buffer_t {

    //treat any buffer as a ring buffer
    uint8_t *buffer;

    //computationaaly cheaper than modulo
    uint32_t mask;

    //read_index_number
    uint32_t read_index;

    //write index number
    uint32_t write_index;
} ring_buffer_t;

void ring_buffer_setup(ring_buffer_t* rb, uint8_t* buffer, uint32_t size);
bool ring_buffer_empty(ring_buffer_t* rb);
bool ring_buffer_write(ring_buffer_t* rb, uint8_t byte);
bool ring_buffer_read(ring_buffer_t* rb, uint8_t* byte);

#endif