#include <avr/io.h>

.global PCINT0_vect
PCINT0_vect:
    ;Instruction                 ; Relative PC's: Comment (# cycles)
    ; Check if we need to act
    in __tmp_reg__, _SFR_IO_ADDR(PINB)            ; 0: Read input pins (1c)
    andi __tmp_reg__, 0x01           ;%[pin_mask]    ; 1: Check if the interrupt was triggered on the rx pin (1c)
    brne END                        ; 2: If rx pin was 1 (Z flag == 0) jump to END (2c), else if rx pin was 0 (Z flag == 1) continue (1c)
;     ; Start bit
;     ldi %[bit], %[bits]             ; 3: Initialize bit counter (1c)
;     clr %[data]                     ; 4: Clear data register (1c)
;     ldi %[i], %[s_loops]            ; 5: Initialize loop counter for center of start bit busy-wait loop (1c)|
;     nop                             ; 6: busy-wait (1c)
;     nop                             ; 7: busy-wait (1c)
; STRT:
;     dec %[i]                        ; 8 ...204: Decrement loop counter (1c)                  |
;     nop                             ; 9 ...205: busy-wait (1c)                               | 4 cycle loop
;     brne STRT                       ; 10...206: If i == 0, continue (1c), else goto STRT (2c)|
;     dec %[bit]                      ; 207: Halfway point of start bit, decrement bit counter (1c) !!!
;     ; Data bits 
;     nop                             ; 0: Delay to get correct timing for first data bit loop (1c)
;     nop                             ; 1:  (1c)
;     nop                             ; 2:  (1c)
;     nop                             ; 3:  (1c)
;     nop                             ; 4:  (1c)
;     nop                             ; 5:  (1c)
;     nop                             ; 6:  (1c)
;     nop                             ; 7:  (1c)
;     nop                             ; 8:  (1c)
;     nop                             ; 9:  (1c)
; DATA:
;     nop                             ; 10, 426: Delay to get correct timing for data bit loop (1c)
;     lsr %[data]                     ; 11: Shift data byte so the MSB is ready to recieve the next bit (1c)
;     ldi %[i], %[d_loops]            ; 12, 428: Initialize loop counter for data bit busy-wait loop (1c)
; D:    
;     dec %[i]                        ; 13...413, 429...829: Decrement loop counter (1c)                  |
;     nop                             ; 14...414, 430...830: busy-wait (1c)                               | 4 cycle loop
;     brne D                          ; 15...415, 431...831: If i == 0, continue (1c), else goto D (2c)|
;     ; Read data bit / start of stop bit (After all data bits read)
;     in __tmp_reg__, %[pin]          ; 0, 416, 832: Read input pins (1c) !!!
;     and __tmp_reg__, %[pin_mask]    ; 1, 417: Check the rx pin (1c)
;     breq ZERO                       ; 2, 418: If rx pin was 0 (Z flag == 1) jump to ZERO (2c), else if rx pin was 1 (Z flag == 0) continue (1c)
;     sbr %[data], 0x80               ; 3, 419: Set MSB in data register if rx pin was a 1 (1c)
; ZERO:
;     nop                             ; 4, 420: Delay to get correct timing for data bit loop (1c)
;     nop                             ; 5, 421:  (1c)
;     dec %[bit]                      ; 6, 422: Decrement bit counter (1c)
;     cpi %[bit], 0                   ; 7, 423: Check if bit counter indicates we are done reading (1c)
;     brne DATA                       ; 8, 424: If bit counter indicates we need to read more, jump to DATA (2c), else continue (1c)
;     ; Stop bit
;     nop                             ; 9, 425: Delay to get correct timing for stop bit loop (1c)
;     nop                             ; 10, 426:  (1c)
;     nop                             ; 11, 427:  (1c)
;     ldi %[i], %[d_loops]            ; 12, 428: Initialize loop counter for stop bit busy-wait loop (1c)
; STOP:  
;     dec %[i]                        ; 13...413: Decrement loop counter (1c)                  |
;     nop                             ; 14...414: busy-wait (1c)                               | 4 cycle loop
;     brne STOP                        15...415: If i == 0, continue (1c), else goto STOP (2c)|
;     nop                             ; 416: Halfway point of first stop bit (1c) !!!
END:                                ; Done
    reti
