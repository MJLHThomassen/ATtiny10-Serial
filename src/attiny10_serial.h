#pragma once

#include <stdint.h>
#include <avr/io.h>

#ifndef F_CPU
    #error F_CPU Needs to be defined!
#endif

#ifndef BAUD
    #define BAUD 2400UL
    #warning BAUD Not defined, defaulting to 2400
#endif

#ifndef DATA_BITS
    #define DATA_BITS 8
#endif
#if DATA_BITS < 5 || DATA_BITS > 9
    #error DATA_BITS should be in the range [5,9]
#endif

#define PARITY_NONE 0
#define PARITY_EVEN 1
#define PARITY_ODD 2
#define PARITY_MARK 3
#define PARITY_SPACE 4

#ifndef PARITY
    #define PARITY PARITY_NONE
#endif
#if PARITY != PARITY_NONE && \
    PARITY != PARITY_EVEN && \
    PARITY != PARITY_ODD && \
    PARITY != PARITY_MARK && \
    PARITY != PARITY_SPACE
    #error Unknown PARITY setting, should be PARITY_NONE, PARITY_EVEN, PARITY_ODD, PARITY_MARK or PARITY_SPACE
#endif
#if PARITY != PARITY_NONE && DATA_BITS == 9
    #error Can not use parity bit if DATA_BITS == 9
#endif

#define STOP_BIT_1 0
#define STOP_BIT_1_5 1
#define STOP_BIT_2 2

#ifndef STOP_BITS
    #define STOP_BITS STOP_BIT_1
#endif
#if STOP_BITS != STOP_BIT_1 && STOP_BITS != STOP_BIT_1_5 && STOP_BITS != STOP_BIT_2
    #error STOP_BITS should be STOP_BIT_1, STOP_BIT_1_5 or STOP_BIT_2
#endif

#if STOP_BITS == STOP_BIT_1
#define STOP_BIT_LOOPS 1
#else
#define STOP_BIT_LOOPS 2
#endif

#define SIGNIFICANT_BIT_LSB 0
#define SIGNIFICANT_BIT_MSB 1

#ifndef SIGNIFICANT_BIT
    #define SIGNIFICANT_BIT SIGNIFICANT_BIT_LSB
#endif
#if SIGNIFICANT_BIT != SIGNIFICANT_BIT_LSB && \
    SIGNIFICANT_BIT != SIGNIFICANT_BIT_MSB
    #error Unknown SIGNIFICANT_BIT setting, should be SIGNIFICANT_BIT_LSB or SIGNIFICANT_BIT_MSB
#endif

#ifndef SIGNAL_INVERSION
    #define SIGNAL_INVERSION false
#endif
#if SIGNAL_INVERSION != false && \
    SIGNAL_INVERSION != true
    #error Unknown SIGNAL_INVERSION setting, should be false or true
#endif

#if SIGNIFICANT_BIT == SIGNIFICANT_BIT_LSB
#define DATA_MASK 0xFF >> (8 - DATA_BITS)
#elif SIGNIFICANT_BIT == SIGNIFICANT_BIT_MSB
#define DATA_MASK 0xFF << (8 - DATA_BITS)
#endif

#if SIGNAL_INVERSION
#define SB0 "sbi"
#define SB1 "cbi"
#else
#define SB0 "cbi"
#define SB1 "sbi"
#endif

#if BAUD == 50
#define BIT_TICKS 20000
#elif BAUD == 75
#define BIT_TICKS 13333
#elif BAUD == 150
#define BIT_TICKS 6666
#elif BAUD == 200
#define BIT_TICKS 5000
#elif BAUD == 300
#define BIT_TICKS 3333
#elif BAUD == 600
#define BIT_TICKS 1666
#elif BAUD == 1200
#define BIT_TICKS 833
#elif BAUD == 1800
#define BIT_TICKS 555
#elif BAUD == 2400
#define BIT_TICKS 416
#elif BAUD == 4800
#define BIT_TICKS 208
#elif BAUD == 9600
#define BIT_TICKS 104
#elif BAUD == 19200
#define BIT_TICKS 52
#define BIT_LOOPS 13
#define HALF_BIT_LOOPS 6
#define BIT_PADDING "nop \n\t" "nop \n\t"
#else
    #error Unsupported BAUD rate
#endif

// By default, we use 4-tick loops with no padding nops
#ifndef BIT_LOOPS
#define BIT_LOOPS ((BIT_TICKS)/4)
#endif
#ifndef HALF_BIT_LOOPS
#define HALF_BIT_LOOPS ((BIT_LOOPS)/2)
#endif
#ifndef BIT_PADDING
#define BIT_PADDING
#endif

#define BIT_COUNT DATA_BITS + 1

[[gnu::always_inline]]
inline void attiny10_serial_begin_tx(const uint8_t pin)
{
    #if SIGNAL_INVERSION
    PUEB &= ~_BV(pin); // Disable Pull-Up for the UART TX pin
    #else
    PUEB |= _BV(pin); // Enable Pull-Up for the UART TX pin
    #endif
    DDRB |= _BV(pin); // Set UART TX pin as output
}

[[gnu::always_inline]]
inline void attiny10_serial_end_tx(const uint8_t pin)
{
    PUEB &= ~_BV(pin); // Disable Pull-Up for the UART TX pin
    DDRB &= ~_BV(pin); // Set UART TX back to input
}

[[gnu::always_inline]]
inline void attiny10_serial_begin_rx(const uint8_t pin)
{
    PUEB &= ~_BV(pin); // Disable Pull-Up for the UART RX pin
    DDRB &= ~_BV(pin); // Set UART RX pin as input
    PCICR |= _BV(PCIE0); // Enable pin change interrupt 0
    PCMSK |= _BV(pin); // Enable pin change interrupt on RX pin
}

[[gnu::always_inline]]
inline void attiny10_serial_end_rx(const uint8_t pin)
{
    PUEB &= ~_BV(pin); // Disable Pull-Up for the UART RX pin
    DDRB &= ~_BV(pin); // Set UART RX back to input
    PCICR &= ~_BV(PCIE0); // Disable pin change interrupt 0
    PCMSK &= ~_BV(pin); // Disable pin change interrupt on RX pin
}

inline uint8_t attiny10_serial_read(const uint8_t pin);

#define attiny10_serial_write_pb(pin, data) attiny10_serial_write_pb##pin(data)
#define attiny10_serial_write(pin, data) attiny10_serial_write_pb(pin, (data))

void attiny10_serial_write_pb0(uint8_t data);
void attiny10_serial_write_pb1(uint8_t data);
void attiny10_serial_write_pb2(uint8_t data);
void attiny10_serial_write_pb3(uint8_t data);