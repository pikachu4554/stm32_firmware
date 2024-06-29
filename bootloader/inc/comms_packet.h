#ifndef COMMS_PACKET_H
#define COMMS_PACKET_H

#include "common-defines.h"

#define PACKET_DATA_LENGTH (16)
#define PACKET_HEADER_LENGTH (1)
#define PACKET_CRC_LENGTH (1)
#define PACKET_LENGTH (PACKET_HEADER_LENGTH + PACKET_DATA_LENGTH + PACKET_CRC_LENGTH)

#define RETX_PACKET_DATA0 (0x19)
#define ACK_PACKET_DATA0 (0x15)

#define SYNC_OBSERVED_PACKET_DATA0 (0x20)
#define FW_UPDATE_REQ_PACKET_DATA0 (0x31)
#define FW_UPDATE_RES_PACKET_DATA0 (0X37)
#define DEV_ID_REQ_PACKET_DATA0 (0X3C)
#define DEV_ID_RES_PACKET_DATA0 (0X3F)
#define FW_LENGTH_REQ_DATA0 (0X42)
#define FW_LENGTH_RES_DATA0 (0X45)
#define READY_FOR_DATA_DATA0 (0X48)
#define UPDATE_SUCCESSFUL_DATA0 (0X45)
#define NACK_APCKET_DATA0 (0x59)

typedef struct comms_packet_t{
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
bool packet_is_single_byte(const comms_packet_t* packet, uint8_t byte);
void comms_create_single_byte_packet(comms_packet_t* packet, uint8_t byte);

#endif