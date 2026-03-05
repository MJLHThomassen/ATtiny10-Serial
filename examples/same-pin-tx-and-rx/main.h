#pragma once

#define F_CPU 1000000UL
#define UART_TX_PIN PB1
#define UART_RX_PIN PB1

#define BAUD 19200UL
#define DATA_BITS 8
#define PARITY PARITY_ODD
#define STOP_BITS STOP_BIT_1
#define SIGNIFICANT_BIT SIGNIFICANT_BIT_LSB
#define SIGNAL_INVERSION false