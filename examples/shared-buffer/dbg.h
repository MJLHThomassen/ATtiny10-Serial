#pragma once

#include <avr/io.h>

#define DBG_PIN PB2
#define DBG_ON() PORTB |= _BV(DBG_PIN)
#define DBG_OFF() PORTB &= ~_BV(DBG_PIN)
#define DBG_TOGGLE() PORTB ^= _BV(DBG_PIN)
#define DBG_SETUP() { \
    PUEB &= ~_BV(DBG_PIN); /* Disable Pull-Up for the DBG pin */ \
    DDRB |= _BV(DBG_PIN); /* Set DBG Pin as output */ \
}

