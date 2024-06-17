#include "common-defines.h"
#include "core/ring_buffer.h"

void ring_buffer_setup(ring_buffer_t* rb, uint8_t* buffer, uint32_t size){

    rb->buffer = buffer;
    rb->read_index = 0;
    rb->write_index = 0;
    rb->mask = size - 1; //assuming size is always a power of 2;

}

bool ring_buffer_empty(ring_buffer_t* rb){

    return (rb->read_index == rb->write_index);

}

bool ring_buffer_read(ring_buffer_t* rb, uint8_t* byte){

    //make copies to ensure that even if other functions are using
    //the same buffer, this scope's values remain same
    uint32_t local_read_index = rb->read_index;
    uint32_t local_write_index = rb->write_index;

    if(local_read_index == local_write_index){

        return false; //buffer empty

    }

    *byte = (rb->buffer)[local_read_index];
    local_read_index = (local_read_index+1) & rb->mask;
    rb->read_index = local_read_index;

    return true;

}

bool ring_buffer_write(ring_buffer_t* rb, uint8_t byte){

    uint32_t local_read_index = rb->read_index;
    uint32_t local_write_index = rb->write_index;

    if( ((local_write_index+1) & rb->mask) == local_read_index ){

        //drop the data as the buffer will become full. We will lose 1 space becaus of this method of implementation.
        return false;

    }

    (rb->buffer)[local_write_index] = byte;
    local_write_index = (local_write_index+1) & rb->mask;
    rb->write_index = local_write_index;

    return true; //without this, we wont  get output

}

