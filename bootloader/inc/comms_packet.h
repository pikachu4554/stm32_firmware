#ifndef COMMS_PACKET_H
#define COMMS_PACKET_H

#include "common-defines.h"

#define PACKET_DATA_LENGTH (16)
#define PACKET_HEADER_LENGTH (1)
#define PACKET_CRC_LENGTH (1)
#define PACKET_LENGTH (PACKET_HEADER_LENGTH + PACKET_DATA_LENGTH + PACKET_CRC_LENGTH)

#define RETX_PACKET_DATA0 (0x19)
#define ACK_PACKET_DATA0 (0x15)

typedef struct copmms_packet_t{
    uint8_t length;
    uint8_t data[PACKET_DATA_LENGTH];
    uint8_t crc;
} comms_packet_t;

void comms_setup(void);
void comms_update(void);
bool comms_packets_available(void);
void comms_write(comms_packet_t* packet);
void comms_read(comms_packet_t* packet);
uint8_t comms_compute_crc(comms_packet_t* packet);

#endif