#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#include "core/uart.h"
#include "core/ring_buffer.h"

//lenght calculation  => (actual_baud_rate/program_latenecy) here: 11520/1000 = 11.52
//for a 10ms latency, then 115.2 size buffer
//actual baud rate is the number of bytes other than overhead bytes in data transfer
#define RING_BUFFER_SIZE (128)

static ring_buffer_t ring_buffer = {0U};
static uint8_t data_buffer[RING_BUFFER_SIZE] = {0U};

void usart2_isr(void){
    const bool overrun_occured = usart_get_flag(USART2, USART_FLAG_ORE) == 1;
    const bool received_data = usart_get_flag(USART2, USART_FLAG_RXNE) == 1;

    if(received_data || overrun_occured){

        if( ring_buffer_write(&ring_buffer, (uint8_t)usart_recv(USART2)) ){
            //handle failuire??
        }

    }
}

void uart_setup(void){

    //setup ring buffer
    ring_buffer_setup(&ring_buffer, data_buffer, RING_BUFFER_SIZE);

    //clock setup
    rcc_periph_clock_enable(RCC_USART2);

    //usart configuration
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE); //no hw enabled flow control
    usart_set_databits(USART2,8);
    usart_set_baudrate(USART2, 115200);
    usart_set_parity(USART2,0); //no parity bits
    usart_set_stopbits(USART2,1); //1 stop bit
    usart_set_mode(USART2, USART_MODE_TX_RX);

    //interrupt setup
    usart_enable_rx_interrupt(USART2);
    nvic_enable_irq(NVIC_USART2_IRQ);

    //enable peripheral
    usart_enable(USART2);

}

void uart_teardown(void){
    usart_disable_rx_interrupt(USART2);
    usart_disable(USART2);
    nvic_disable_irq(NVIC_USART2_IRQ);
    rcc_periph_clock_disable(RCC_USART2);
}

void uart_write(uint8_t *data, const uint32_t length){
    for(uint32_t i =0; i<length; i++){
        uart_write_byte(data[i]); //compiler will most likely inline this
    }
}

void uart_write_byte(uint8_t data){
    usart_send_blocking(USART2, (uint16_t)data); //casting may happen automatically
}

uint32_t uart_read(uint8_t *data, const uint32_t length){
    if(length == 0){ //if this was >, then the this function gets discarded
        return 0;
    }

    for(uint32_t i = 0;i<length;i++){
        if(  !(  ring_buffer_read(&ring_buffer,&data[i])  )  ){
            return i; //number of bytes actually read
        }
    } 

    return length;
}

uint8_t uart_read_byte(void){
    uint8_t byte =0;
    (void)uart_read(&byte, 1);
    return byte;
}

bool uart_data_available(void){
    return !(ring_buffer_empty(&ring_buffer));
}

