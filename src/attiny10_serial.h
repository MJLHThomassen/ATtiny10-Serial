#pragma once

#include <stdint.h>
#include <avr/io.h>
#include <util/parity.h>

#ifndef F_CPU
    #error F_CPU Needs to be defined!
#endif

#ifndef UART_TX_PIN
    #error UART_TX_PIN Needs to be defined!
#endif

#ifndef UART_RX_PIN
    #error UART_RX_PIN Needs to be defined!
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

#ifndef PARITY
    #define PARITY PARITY_NONE
#endif
#if PARITY != PARITY_NONE && \
    PARITY != PARITY_EVEN && \
    PARITY != PARITY_ODD
    #error Unknown PARITY setting, should be PARITY_NONE, PARITY_EVEN or PARITY_ODD
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
#else
    #error Unsupported BAUD rate
#endif

#define BIT_COUNT DATA_BITS + 1


#if UART_RX_PIN != UART_TX_PIN
void attiny10_serial_begin()
{
    DDRB &= ~_BV(UART_RX_PIN); // Set UART RX pin as input
    PUEB |= _BV(UART_TX_PIN); // Enable Pull-Up for the UART TX pin
    DDRB |= _BV(UART_TX_PIN); // Set UART TX pin as output
}

void attiny10_serial_end()
{
    PUEB &= ~_BV(UART_TX_PIN); // Disable Pull-Up for the UART TX pin
    DDRB &= ~_BV(UART_TX_PIN); // Set UART TX back to input
}
#else
void attiny10_serial_begin_tx()
{
    #if SIGNAL_INVERSION
    PUEB &= ~_BV(UART_TX_PIN); // Disable Pull-Up for the UART TX pin
    #else
    PUEB |= _BV(UART_TX_PIN); // Enable Pull-Up for the UART TX pin
    #endif
    DDRB |= _BV(UART_TX_PIN); // Set UART TX pin as output
}
void attiny10_serial_begin_rx()
{
    PUEB &= ~_BV(UART_RX_PIN); // Disable Pull-Up for the UART RX pin
    DDRB &= ~_BV(UART_RX_PIN); // Set UART RX pin as input
}
void attiny10_serial_end()
{
    PUEB &= ~_BV(UART_TX_PIN); // Disable Pull-Up for the UART TX pin
    DDRB &= ~_BV(UART_TX_PIN); // Set UART TX back to input
}
#endif

void attiny10_serial_write(uint8_t data)
{
    volatile uint8_t i;
    volatile uint8_t bit_counter;

    #if DATA_MASK != 0xFF
    // DATA_MASK ensures data contains only 0's after BIT_COUNT bits are send.
    // This ensures the brts ONE__%= instruction is always skipped and,
    // subsequently, breq PAR_%= / STOP_%= instruction is executed (and taken).
    data &= DATA_MASK;
    #endif

    #if PARITY == PARITY_EVEN
    volatile uint8_t parity = parity_even_bit(data);
    #elif PARITY == PARITY_ODD
    volatile uint8_t parity = 0x01 ^ parity_even_bit(data);
    #endif

    asm volatile(
    //Label:    Instruction                 // Relative PC's: Comment (# cycles)
                // Start and Data bits
                SB0" %[port], %[tx] \n\t"   // 0: Start bit (1c)
                "ldi %[bit], %[bits] \n\t"  // 1: Initialize bit counter (1c)
                "nop \n\t"                  // 2: Delay to get correct timing for start bit loop (1c)
    "BIT_%=:"   "ldi %[i], %[bp_loops] \n\t"// 3, 419: Initialize loop counter for inter-bit busy-wait loop (1c)
    "B_%=:"     "dec %[i] \n\t"             // 4, 8 ...408, 420...824: Decrement loop counter (1c)                  |
                "nop \n\t"                  // 5, 9 ...409, 421...825: busy-wait (1c)                               | 4 cycle loop
                "brne B_%= \n\t"            // 6, 10...410, 422...826: If i == 0, continue (1c), else goto B_%= (2c)|
                #if SIGNIFICANT_BIT == SIGNIFICANT_BIT_LSB
                "bst %[data], 0 \n\t"       // 411, 827: Load next bit (LSB of input byte) to send into T flag (1c)
                "lsr %[data] \n\t"          // 412, 828: Shift next bit into LSB location of input byte (1c)
                #elif SIGNIFICANT_BIT == SIGNIFICANT_BIT_MSB
                "bst %[data], 7 \n\t"       // 411, 827: Load next bit (MSB of input byte) to send into T flag (1c)
                "lsl %[data] \n\t"          // 412, 828: Shift next bit into MSB location of input byte (1c)
                #endif
                "dec %[bit] \n\t"           // 412, 829: Decrement bit counter (1c)
                "brts ONE__%= \n\t"         // 414, 830: Test T flag, if we need to send a 1, goto ONE__%= (2c), else continue (1c)
                #if PARITY != PARITY_NONE
                "breq PAR_%= \n\t"          // 415, 831: If zero flag is set (from previous dec instruction), jump to PAR_%= (2c), else continue (1c)
                #else
                "breq STOP_%= \n\t"         // 415, 831: If zero flag is set (from previous dec instruction), jump to STOP_%= (2c), else continue (1c)
                #endif
    "ZERO_%=:"  SB0 " %[port], %[tx] \n\t"  // 416, 832: Write 0 (1c)
                "rjmp BIT_%= \n\t"          // 417, 833: Jump to BIT_%= (2c)
    "ONE__%=:"  SB1 " %[port], %[tx] \n\t"  // 416, 832: Write 1 (1c)
                "rjmp BIT_%= \n\t"          // 417, 833: Jump to BIT_%= (2c)
                // Parity bit
                #if PARITY != PARITY_NONE
    "PAR_%=:"   "bst %[par], 0 \n\t"        // 1: Load parity bit into T flag (1c)
                "brts P1_%= \n\t"           // 2: Test T flag, if we need to send a 1, goto P1__%= (2c), else continue (1c)
                "nop \n\t"                  // 3: Delay to get correct timing for parity bit loop (1c)
                SB0 " %[port], %[tx] \n\t"  // 4: Parity bit even (this is 4 ticks too late because of the 2c the breq PAR_%= instruction needs and the 3c the parity check needs) (1c)
                "rjmp PLD_%= \n\t"          // 5: Jump to PLD_%= (2c)
    "P1_%=:"    SB1 " %[port], %[tx] \n\t"  // 4: Parity bit odd (this is 4 ticks too late because of the 2c the breq PAR_%= instruction needs and the 3c the parity check needs) (1c)
                "nop \n\t"                  // 6: Delay to get correct timing for parity bit loop (1c)
    "PLD_%=:"   "ldi %[i], %[bp_loops] \n\t"// 7: Initialize loop counter for parity-bit busy-wait loop (1c)
    "P_%=:"     "dec %[i] \n\t"             // 8,  12...412: Decrement loop counter (1c)                    |
                "nop \n\t"                  // 9,  13...413: busy-wait (1c)                                 | 4 cycle loop
                "brne P_%= \n\t"            // 10, 14...414: If i == 0, continue (1c), else goto P_%= (2c) |
                "nop \n\t"                  // 415: Delay to get correct timing for parity bit loop (1c)
                "nop \n\t"                  // 416: Delay to get same timing for stop bit  as when parity bit is not send (1c)
                #endif
                // Stop bit
    "STOP_%=:"  SB1 " %[port], %[tx] \n\t"  // 1: Stop bit (this is 1 tick too late because of the 2c the breq STOP_%= instruction needs, and, if there is a parity bit, we tuned the duration of the parity bit so the stop bit also starts 1 tick too late) (1c)
                "ldi %[bit], %[sbits] \n\t" // 2: Initialize bit counter (1c)
                "ldi %[i], %[s_loops] \n\t" // 3: Initialize loop counter for stop-bit busy-wait loop (1c)
    "S_%=:"     "dec %[i] \n\t"             // 4, 8 ...408, 415...617/823: Decrement loop counter (1c)                    |
                "nop \n\t"                  // 5, 9 ...409, 416...618/824: busy-wait (1c)                                 | 4 cycle loop
                "brne S_%= \n\t"            // 6, 10...410, 417...619/825: If i == 0, continue (1c), else goto S_%= (2c)  |
                "dec %[bit] \n\t"           // 411, 620/826: Decrement bit counter (1c)
                #if STOP_BITS == STOP_BIT_1
                "nop \n\t"                  // 412 Delay to get correct timing for stop bit loop (1c)
                "nop \n\t"                  // 413 Delay to get correct timing for stop bit loop (1c)
                "nop \n\t"                  // 414 Delay to get correct timing for stop bit loop (1c)
                "nop \n\t"                  // 415 Delay to get correct timing for stop bit loop (1c)
                #elif STOP_BITS == STOP_BIT_1_5
                "ldi %[i], %[hs_loops] \n\t"// 412, 621: Initialize loop counter for next stop-bit busy-wait loop (1c)
                "brne S_%= \n\t"            // 413, 622: If zero flag is set (from previous dec instruction), jump to S_%= (2c), else continue (1c)
                "nop \n\t"                  // 623 Delay to get correct timing for 2nd half stop bit loop (1c)
                #else
                "ldi %[i], %[s_loops] \n\t" // 412, 827: Initialize loop counter for next stop-bit busy-wait loop (1c)
                "brne S_%= \n\t"            // 413, 828: If zero flag is set (from previous dec instruction), jump to S_%= (2c), else continue (1c)
                "nop \n\t"                  // 829 Delay to get correct timing for 2nd stop bit loop (1c)
                "nop \n\t"                  // 830 Delay to get correct timing for 2nd stop bit loop (1c)
                "nop \n\t"                  // 831 Delay to get correct timing for 2nd stop bit loop (1c)
                #endif
                // Done
            : // No outputs
            : [port] "I" (_SFR_IO_ADDR(PORTB)),
                [tx] "I" (UART_TX_PIN),
                [i] "a" (i),
                [bit] "a" (bit_counter),
                [data] "a" (data),
                #if PARITY != PARITY_NONE
                [par] "a" (parity),
                #endif
                [bits] "M" (BIT_COUNT),
                [sbits] "M" (STOP_BIT_LOOPS), // Nr of loops for stop bit(s)
                [bp_loops] "M" ((BIT_TICKS / 4) - 2), // Nr of N-cycle-loops for data bits and parity bit
                #if STOP_BITS == STOP_BIT_1_5
                [hs_loops] "M" (((BIT_TICKS / 4) - 2) / 2), // Nr of N-cycle-loops for 0.5 stop bit
                #endif
                [s_loops] "M" ((BIT_TICKS / 4) - 2) // Nr of N-cycle-loops for stop bit
    );
}