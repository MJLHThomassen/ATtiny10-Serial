#include <avr/io.h>

__tmp_reg__=r16

.global buffer
.global PCINT0_vect

PCINT0_vect:
    push XL;
    push XH;
    ldi XL, lo8(buffer);
    ldi XH, hi8(buffer);

    ld      __tmp_reg__, X;
    sbis    PINB, PB1;
    inc     __tmp_reg__;
    st      X, __tmp_reg__;

    sbi     PCIFR, PCIF0;

    pop XH;
    pop XL;

    reti;