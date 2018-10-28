#ifndef MY_UART_H
#define MY_UART_H

#include "stdint.h"

void rx_one_byte(uint8_t by);
void rx_data_parse(void);
void rx_timer_counter(void);
void my_uart_init(void);
#endif
