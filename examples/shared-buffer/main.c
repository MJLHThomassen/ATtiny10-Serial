#include <stdint.h>
#include <avr/interrupt.h>

#include "dbg.h"

volatile uint8_t buffer[8];

static void setup()
{
    DBG_SETUP();

    PCICR |= _BV(PCIE0); // Enable pin change interrupt 0
    PCMSK |= _BV(PB1); // Enable pin change interrupt on RX pin
    sei(); // Enable global interrupts
}

int main(void)
{
    setup();

    while(true)
    {
        if(buffer[0] & 0x01)
        {
            DBG_ON();
        }
        else
        {
            DBG_OFF();
        }
    }

}