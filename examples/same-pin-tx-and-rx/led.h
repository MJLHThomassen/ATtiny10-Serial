#pragma once

#include <avr/io.h>

#define LED_PIN PB2
#define LED_ON() PORTB |= _BV(LED_PIN)
#define LED_OFF() PORTB &= ~_BV(LED_PIN)
#define LED_TOGGLE() PORTB ^= _BV(LED_PIN)

