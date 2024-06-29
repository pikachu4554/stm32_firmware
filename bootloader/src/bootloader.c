#include "common-defines.h"
#include <libopencm3/stm32/memorymap.h> //for FLASH_BASE
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "core/uart.h"
#include "core/ring_buffer.h"
#include "comms_packet.h"
#include "core/system.h"
#include "bl_flash.h"
#include "core/higher_timer.h"

#define BOOTLOADER_SIZE (0x8000U) //U ensures that the number is always treated as an unsigned number. operations convert this define to a signed number
#define MAIN_FIRMWARE_START_ADDRESS (FLASH_BASE + BOOTLOADER_SIZE)
#define MAX_FW_LENGTH (1024U*512U - BOOTLOADER_SIZE)

#define UART_PORT (GPIOA)
#define UART_TX_PIN (GPIO2)
#define UART_RX_PIN (GPIO3)

#define DEV_ID (0x24)
#define SYNC_SEQ_0 (0xA1)
#define SYNC_SEQ_1 (0xA2)
#define SYNC_SEQ_2 (0xA3)
#define SYNC_SEQ_3 (0xA4)

#define DEFAULT_TIMEOUT (5000)

typedef enum bl_state_t{
    bl_sync_state,
    bl_wait_for_update_req_state,
    bl_devID_req_state,
    bl_devID_res_state,
    bl_fw_length_req_state,
    bl_fw_length_res_state,
    bl_erase_app_state,
    bl_recv_fw_state,
    bl_done_state
} bl_state_t;

static bl_state_t state = bl_sync_state;
static uint32_t fw_length = 0;
static uint32_t bytes_written = 0;
static uint8_t sync_seq[4] = {0};
static h_timer_config_t timer;    
static comms_packet_t temp_packet = {0};

static void gpio_setup(void){
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(UART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, UART_TX_PIN | UART_RX_PIN); 
    gpio_set_af(UART_PORT, GPIO_AF7,UART_TX_PIN | UART_RX_PIN);
}

static void gpio_teardown(void){
    gpio_mode_setup(UART_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, UART_TX_PIN| UART_RX_PIN); //analog is the default pin mode
    rcc_periph_clock_disable(RCC_GPIOA);
    //not removing alternate functions. why?
}

static void execute_main(void){ //static is neede here. why?
    typedef void (*main_fn)(void);
    uint32_t *reset_vector_entry = (uint32_t *)(MAIN_FIRMWARE_START_ADDRESS + 4U);
    uint32_t *reset_vector = (uint32_t *)(*reset_vector_entry);
    main_fn main_fn_reset_vector=(main_fn)reset_vector;

    main_fn_reset_vector();
}

static void bootloader_panic(void){
    comms_create_single_byte_packet(&temp_packet, NACK_APCKET_DATA0);
    comms_write(&temp_packet);
    state = bl_done_state;
}

static void check_for_timeout(void){
    if(h_timer_has_elapsed(&timer)){
        bootloader_panic();
    }
}

static bool is_device_id_packet(const comms_packet_t *packet){
    if(packet->length != 2){
        return false;
    }

    if(packet->data[0] != DEV_ID_RES_PACKET_DATA0){
        return false;
    }

    for(uint8_t i = 2; i<PACKET_DATA_LENGTH;i++){
        if(packet->data[i] != 0xff){
            return false;
        }
    }
    return true;
}

static bool is_fw_length_packet(const comms_packet_t *packet){
    if(packet->length != 5){
        return false;
    }

    if(packet->data[0] != FW_LENGTH_RES_DATA0){
        return false;
    }

    for(uint8_t i = 5; i<PACKET_DATA_LENGTH;i++){
        if(packet->data[i] != 0xff){
            return false;
        }
    }
    return true;
}

int main(void){  //void is needed here. why

    system_setup();
    gpio_setup();
    uart_setup();
    comms_setup();

    
    h_timer_setup(&timer,DEFAULT_TIMEOUT,false);

    while(state != bl_done_state){
        if(state == bl_sync_state){
            if(uart_data_available()){
                sync_seq[0] = sync_seq[1];
                sync_seq[1] = sync_seq[2];
                sync_seq[2] = sync_seq[3];
                sync_seq[3] = uart_read_byte();

                bool is_match = (sync_seq[0] == SYNC_SEQ_0);
                is_match = is_match && (sync_seq[1] == SYNC_SEQ_1);
                is_match = is_match && (sync_seq[1] == SYNC_SEQ_2);
                is_match = is_match && (sync_seq[1] == SYNC_SEQ_3);

                if(is_match){
                    comms_create_single_byte_packet(&temp_packet, SYNC_OBSERVED_PACKET_DATA0);
                    comms_write(&temp_packet);
                    h_timer_reset(&timer);
                    state = bl_wait_for_update_req_state;
                }else {
                    check_for_timeout();
                }
            } else {
                check_for_timeout();
            }

            continue;
        }

        comms_update();

        switch(state){

            case bl_wait_for_update_req_state:{
                if(comms_packets_available()){
                    comms_read(&temp_packet);
                    if(packet_is_single_byte(&temp_packet,FW_UPDATE_REQ_PACKET_DATA0)){
                        h_timer_reset(&timer);
                        comms_create_single_byte_packet(&temp_packet, FW_UPDATE_RES_PACKET_DATA0);
                        comms_write(&temp_packet);
                        state = bl_devID_req_state;
                    }else{
                        bootloader_panic();
                    }

                }else{
                    check_for_timeout();
                }
            } break;

            case bl_devID_req_state:{
                h_timer_reset(&timer);
                comms_create_single_byte_packet(&temp_packet, DEV_ID_REQ_PACKET_DATA0);
                comms_write(&temp_packet);
                state = bl_devID_res_state;
            } break;

            case bl_devID_res_state:{
                if(comms_packets_available()){
                    comms_read(&temp_packet);
                    if(is_device_id_packet(&temp_packet) && temp_packet.data[1] == DEV_ID){
                        h_timer_reset(&timer);
                        state = bl_devID_req_state;
                    }else{
                        bootloader_panic();
                    }

                }else{
                    check_for_timeout();
                }
            } break;

            case bl_fw_length_req_state:{
                h_timer_reset(&timer);
                comms_create_single_byte_packet(&temp_packet, FW_LENGTH_REQ_DATA0);
                comms_write(&temp_packet);
                state = bl_fw_length_res_state;
            } break;

            case bl_fw_length_res_state:{
                 if(comms_packets_available()){
                    comms_read(&temp_packet);

                    //little endian
                    fw_length = (
                        (temp_packet.data[0]) | 
                        (temp_packet.data[1] << 8)  |
                        (temp_packet.data[2] << 16) |
                        (temp_packet.data[3] << 24) 
                    );

                    if(is_fw_length_packet(&temp_packet) && fw_length <= MAX_FW_LENGTH){
                        h_timer_reset(&timer);
                        state = bl_erase_app_state;
                        
                    }else{
                        bootloader_panic();
                    }

                }else{
                    check_for_timeout();
                }
            } break;

            case bl_erase_app_state:{
                bl_flash_memory_erase(); //may take a few seconds
                comms_create_single_byte_packet(&temp_packet, READY_FOR_DATA_DATA0);
                comms_write(&temp_packet);
                h_timer_reset(&timer);
                state = bl_recv_fw_state;
            } break;

            case bl_recv_fw_state:{
                if(comms_packets_available()){
                    comms_read(&temp_packet);

                    const uint8_t packet_content_length = (temp_packet.length & 0x0f) + 1; //prevents buffer overflow
                    bl_flash_memory_write(MAIN_FIRMWARE_START_ADDRESS + bytes_written, temp_packet.data, packet_content_length);
                    bytes_written += packet_content_length;
                    h_timer_reset(&timer);

                    if(bytes_written >= fw_length){
                        comms_create_single_byte_packet(&temp_packet, UPDATE_SUCCESSFUL_DATA0);
                        comms_write(&temp_packet);
                        state = bl_done_state;
                    } else{
                        comms_create_single_byte_packet(&temp_packet, READY_FOR_DATA_DATA0);
                        comms_write(&temp_packet);
                    }
                } else{
                    check_for_timeout();
                }
            } break;

            default: {
                state = bl_sync_state;
            } break;

        }
    }

    //need to disable all things we setup at the start of code
    system_delay(150); //wait for uart data to tramsmit the successful message
    uart_teardown();
    gpio_teardown();
    system_teardown();

    execute_main();

    return 0;
}

