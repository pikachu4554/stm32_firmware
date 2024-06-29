#include <string.h>
#include "core/uart.h"
#include "comms_packet.h"
#include "core/crc8.h"

#define PACKET_BUFFER_LENGTH (8)

typedef enum state_t{
    state_Length,
    state_Data,
    state_CRC
} state_t;

static state_t state = state_Length;
static uint8_t data_byte_count = 0;

static comms_packet_t temporary_packet = {.length = 0, .data = {0}, .crc = 0};
static comms_packet_t retx_packet = {.length = 0, .data = {0}, .crc = 0};
static comms_packet_t ack_packet = {.length = 0, .data = {0}, .crc = 0};
static comms_packet_t last_transmitted_packet = {.length = 0, .data = {0}, .crc = 0};

static comms_packet_t packet_buffer[PACKET_BUFFER_LENGTH];
static uint32_t packet_buffer_read_index = 0;
static uint32_t packet_buffer_write_index = 0;
static uint32_t packet_buffer_mask = PACKET_BUFFER_LENGTH -1;

bool packet_is_single_byte(const comms_packet_t* packet, uint8_t byte){
    if(packet->length !=1 ){
        return false;
    }

    if(packet->data[0] != byte){
        return false;
    }

    for(int i=1; i<PACKET_DATA_LENGTH; i++){
        if(packet->data[i] != 0xff){
            return false;
        }
    }

    return true;
}

void comms_create_single_byte_packet(comms_packet_t* packet, uint8_t byte){
    memset(packet, 0xff, sizeof(comms_packet_t));
    packet->length = 1;
    packet->data[0] = byte;
    packet->crc = comms_compute_crc(packet);  //this was one source of error
}

//an implementation of memcopy()
static void comms_packet_copy(const comms_packet_t *source, comms_packet_t *destination){
    destination->length = source->length;
    for(int i=0;i<PACKET_DATA_LENGTH;i++){
        destination->data[i] = source->data[i];
    }
    destination->crc = source->crc;
}

void comms_setup(void){
    retx_packet.length=1;
    retx_packet.data[0] = RETX_PACKET_DATA0;
    for(int i =1;i<PACKET_DATA_LENGTH;i++){
        retx_packet.data[i] = 0xff;
    }
    retx_packet.crc = comms_compute_crc(&retx_packet);

    ack_packet.length=1;
    ack_packet.data[0] = ACK_PACKET_DATA0;
    for(int i =1;i<PACKET_DATA_LENGTH;i++){
        ack_packet.data[i] = 0xff;
    }
    ack_packet.crc = comms_compute_crc(&ack_packet);
}
void comms_update(void){
    while( uart_data_available() ){
        switch(state){
            case state_Length:
                temporary_packet.length = uart_read_byte();
                state = state_Data;
                break;
            case state_Data:
                temporary_packet.data[data_byte_count] = uart_read_byte();
                if(data_byte_count >= PACKET_DATA_LENGTH){
                    data_byte_count = 0;
                    state = state_CRC;
                }
                break;
            case state_CRC:
                temporary_packet.crc = uart_read_byte();

                //check for corruption
                if(temporary_packet.crc != comms_compute_crc(&temporary_packet)){
                    comms_write(&retx_packet);
                    state = state_Length;
                    break; 
                }
                
                //check for retransmit request
                if(packet_is_single_byte(&temporary_packet, RETX_PACKET_DATA0)){
                    comms_write(&last_transmitted_packet);
                    state = state_Length;
                    break;
                }

                //ack packet check
                if(packet_is_single_byte(&temporary_packet, ACK_PACKET_DATA0)){
                    state = state_Length;
                    break;
                }
                
                uint8_t next_write_index = (packet_buffer_write_index+1) & packet_buffer_mask;

                if(next_write_index == packet_buffer_read_index){
                    __asm__("BKPT #0");
                }
                //don't have to worry about concurrency as differnt parts will read and write at distinct times, and no interrupts
                comms_packet_copy(&temporary_packet, &packet_buffer[packet_buffer_write_index]);
                packet_buffer_write_index = next_write_index; 
                comms_write(&ack_packet);
                state = state_Length;

                break;
            default:
                state = state_Length;
                break;
        }
    }
}

bool comms_packets_available(void){
    return !(packet_buffer_read_index == packet_buffer_write_index);
}

void comms_write(comms_packet_t* packet){
    uart_write((uint8_t *)packet, PACKET_LENGTH);
    comms_packet_copy(packet, &last_transmitted_packet);
}

void comms_read(comms_packet_t* packet){
    comms_packet_copy(&packet_buffer[packet_buffer_read_index], packet);
    packet_buffer_read_index = (packet_buffer_read_index+1) & packet_buffer_mask;
}

uint8_t comms_compute_crc(comms_packet_t *packet){
    return crc8((uint8_t*)packet, PACKET_LENGTH - PACKET_CRC_LENGTH); //beware of compiler padding 
}


