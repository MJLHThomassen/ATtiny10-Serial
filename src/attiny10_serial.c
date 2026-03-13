#include "attiny10_serial.h"

#include <util/parity.h>

[[gnu::always_inline]]
extern inline uint8_t attiny10_serial_read(const uint8_t pin)
{
    volatile uint8_t i;
    volatile uint8_t bit_counter;
    volatile uint8_t data;

    asm volatile(
    //Label:    Instruction                         // Relative PC's: Comment (# cycles)
                // Check if we need to act
                "in __tmp_reg__, %[pin] \n\t"       // 0: Read input pins (1c)
                "and __tmp_reg__, %[pin_mask] \n\t" // 1: Check if the interrupt was triggered on the rx pin (1c)
                "brne END_%= \n\t"                  // 2: If rx pin was 1 (Z flag == 0) jump to END_%= (2c), else if rx pin was 0 (Z flag == 1) continue (1c)
                // Start bit
                "ldi %[bit], %[bits] \n\t"          // 3: Initialize bit counter (1c)
                "clr %[data] \n\t"                  // 4: Clear data register (1c)
                "ldi %[i], %[s_loops] \n\t"         // 5: Initialize loop counter for center of start bit busy-wait loop (1c)|
                "nop \n\t"                          // 6: busy-wait (1c)
                "nop \n\t"                          // 7: busy-wait (1c)
    "STRT_%=:"  "dec %[i] \n\t"                     // 8 ...204: Decrement loop counter (1c)                  |
                "nop \n\t"                          // 9 ...205: busy-wait (1c)                               | 4 cycle loop
                "brne STRT_%= \n\t"                 // 10...206: If i == 0, continue (1c), else goto STRT_%= (2c)|
                "dec %[bit] \n\t"                   // 207: Halfway point of start bit, decrement bit counter (1c) !!!
                // Data bits
                "nop \n\t"                          // 0: Delay to get correct timing for first data bit loop (1c)
                "nop \n\t"                          // 1: " (1c)
                "nop \n\t"                          // 2: " (1c)
                "nop \n\t"                          // 3: " (1c)
                "nop \n\t"                          // 4: " (1c)
                "nop \n\t"                          // 5: " (1c)
                "nop \n\t"                          // 6: " (1c)
                "nop \n\t"                          // 7: " (1c)
                "nop \n\t"                          // 8: " (1c)
                "nop \n\t"                          // 9: " (1c)
    "DATA_%=:"  "nop \n\t"                          // 10, 426: Delay to get correct timing for data bit loop (1c)
                "lsr %[data] \n\t"                  // 11: Shift data byte so the MSB is ready to recieve the next bit (1c)
                "ldi %[i], %[d_loops] \n\t"         // 12, 428: Initialize loop counter for data bit busy-wait loop (1c)
    "D_%=:"     "dec %[i] \n\t"                     // 13...413, 429...829: Decrement loop counter (1c)                  |
                "nop \n\t"                          // 14...414, 430...830: busy-wait (1c)                               | 4 cycle loop
                "brne D_%= \n\t"                    // 15...415, 431...831: If i == 0, continue (1c), else goto D_%= (2c)|
                // Read data bit / start of stop bit (After all data bits read)
                "in __tmp_reg__, %[pin] \n\t"       // 0, 416, 832: Read input pins (1c) !!!
                "and __tmp_reg__, %[pin_mask] \n\t" // 1, 417: Check the rx pin (1c)
                "breq ZERO_%= \n\t"                 // 2, 418: If rx pin was 0 (Z flag == 1) jump to ZERO_%= (2c), else if rx pin was 1 (Z flag == 0) continue (1c)
                "sbr %[data], 0x80 \n\t"            // 3, 419: Set MSB in data register if rx pin was a 1 (1c)
    "ZERO_%=:"  "nop \n\t"                          // 4, 420: Delay to get correct timing for data bit loop (1c)
                "nop \n\t"                          // 5, 421: " (1c)
                "dec %[bit] \n\t"                   // 6, 422: Decrement bit counter (1c)
                "cpi %[bit], 0 \n\t"                // 7, 423: Check if bit counter indicates we are done reading (1c)
                "brne DATA_%= \n\t"                 // 8, 424: If bit counter indicates we need to read more, jump to DATA_%= (2c), else continue (1c)
                // Stop bit
                "nop \n\t"                          // 9, 425: Delay to get correct timing for stop bit loop (1c)
                "nop \n\t"                          // 10, 426: " (1c)
                "nop \n\t"                          // 11, 427: " (1c)
                "ldi %[i], %[d_loops] \n\t"         // 12, 428: Initialize loop counter for stop bit busy-wait loop (1c)
    "STOP_%=:"  "dec %[i] \n\t"                     // 13...413: Decrement loop counter (1c)                  |
                "nop \n\t"                          // 14...414: busy-wait (1c)                               | 4 cycle loop
                "brne STOP_%= \n\t"                 // 15...415: If i == 0, continue (1c), else goto STOP_%= (2c)|
    "END_%=:"   "nop\n\t"                           // 416: Halfway point of first stop bit (1c) !!!
                // Done
            : [data] "=&d" (data)
            : [pin] "I" (_SFR_IO_ADDR(PINB)),
                [pin_mask] "d" (_BV(pin)),
                [i] "d" (i),
                [bit] "d" (bit_counter),
                [bits] "M" (BIT_COUNT),
                [s_loops] "M" (HALF_BIT_LOOPS - 3), // Nr of N-cycle-loops to delay before center of (start) bit
                [d_loops] "M" ((BIT_LOOPS - 3)) // Nr of N-cycle-loops to delay between reading (data) bits
    );

    return data;
}

[[gnu::always_inline]]
static inline void _attiny10_serial_write(const uint8_t pin, uint8_t data)
{
    volatile uint8_t i = 0;
    volatile uint8_t bit_counter = 0;
    #if PARITY != PARITY_NONE
    volatile uint8_t parity;
    #endif

    #if DATA_MASK != 0xFF
    // DATA_MASK ensures data contains only 0's after BIT_COUNT bits are send.
    // This ensures the brts ONE__%= instruction is always skipped and,
    // subsequently, breq PAR_%= / STOP_%= instruction is executed (and taken).
    data &= DATA_MASK;
    #endif

    #if PARITY == PARITY_EVEN
    parity = parity_even_bit(data);
    #elif PARITY == PARITY_ODD
    parity = 0x01 ^ parity_even_bit(data);
    #elif PARITY == PARITY_MARK
    parity = 0x01;
    #elif PARITY == PARITY_SPACE
    parity = 0x00;
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
                "dec %[bit] \n\t"           // 413, 829: Decrement bit counter (1c)
                "brts ONE__%= \n\t"         // 414, 830: Test T flag, if we need to send a 1, goto ONE__%= (2c), else continue (1c)
                #if PARITY != PARITY_NONE
                "breq PAR_%= \n\t"          // 415, 831: If zero flag is set (from previous dec instruction), jump to PAR_%= (2c), else continue (1c)
                #else
                "breq STOP_%= \n\t"         // 415, 831: If zero flag is set (from previous dec instruction), jump to STOP_%= (2c), else continue (1c)
                #endif
    "ZERO_%=:"  SB0 " %[port], %[tx] \n\t"  // 416, 832: Write 0 (1c) !!!
                "rjmp BIT_%= \n\t"          // 417, 833: Jump to BIT_%= (2c)
    "ONE__%=:"  SB1 " %[port], %[tx] \n\t"  // 416, 832: Write 1 (1c) !!!
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
                [tx] "I" (pin),
                [i] "a" (i),
                [bit] "a" (bit_counter),
                [data] "a" (data),
                #if PARITY != PARITY_NONE
                [par] "a" (parity),
                #endif
                [bits] "M" (BIT_COUNT),
                [sbits] "M" (STOP_BIT_LOOPS), // Nr of loops for stop bit(s)
                [bp_loops] "M" (BIT_LOOPS - 2), // Nr of N-cycle-loops for data bits and parity bit
                #if STOP_BITS == STOP_BIT_1_5
                [hs_loops] "M" ((BIT_LOOPS - 2) / 2), // Nr of N-cycle-loops for 0.5 stop bit
                #endif
                [s_loops] "M" (BIT_LOOPS - 2) // Nr of N-cycle-loops for stop bit
    );
}


void attiny10_serial_write_pb0(uint8_t data)
{
    _attiny10_serial_write(PB0, data);
}

void attiny10_serial_write_pb1(uint8_t data)
{
    _attiny10_serial_write(PB1, data);
}

void attiny10_serial_write_pb2(uint8_t data)
{
    _attiny10_serial_write(PB2, data);
}

void attiny10_serial_write_pb3(uint8_t data)
{
    _attiny10_serial_write(PB3, data);
}