#include <avr/io.h>
#include <avr/fuse.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <util/parity.h>

#include "main.h"
#include "led.h"
#include "../../src/attiny10_serial.h"

static volatile uint8_t rx_data = 0;
static volatile bool rx_data_available = false;

static void dbg_byte(uint8_t byte)
{
    DBG_ON();
    DBG_OFF();

    for (int j = 0; j < 8; ++j)
    {
        if (byte & 0x01)
        {
            DBG_ON();
        }
        else
        {
            DBG_OFF();
        }

        byte >>= 1;

        _delay_us(10);

        DBG_OFF();
    }
}

static void setupPowerSaving()
{
    ACSR = _BV(ACD);     // Disable Analog Comparator
    power_adc_disable(); // Disable ADC
}

static void setupClock()
{
    CCP = 0xD8;
    CLKMSR = 0x00; // Internal 8MHz Oscillator
    CCP = 0xD8;
    CLKPSR = 0X03; // Clock Division Factor 8 -> 1Mhz clock (default)
    //OSCCAL = 0x53;
    OSCCAL = 0x87;
}

static void setupSleep()
{
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
}

static void setupIO()
{
    PUEB &= ~_BV(LED_PIN); // Disable Pull-Up for the LED pin
    DDRB |= _BV(LED_PIN); // Set LED Pin as output

    PUEB &= ~_BV(DBG_PIN); // Disable Pull-Up for the DBG pin
    DDRB |= _BV(DBG_PIN); // Set DBG Pin as output
}

static void setup()
{
    setupPowerSaving();
    setupClock();
    setupSleep();
    setupIO();
}

// Resources:
// AVR Instruction Set Manual: https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/ReferenceManuals/AVR-InstructionSet-Manual-DS40002198.pdf
// GCC Manual: Using inline assembly in C:
// - Overview: https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html
// - Details: https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
// - AVR Specifics: https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html (Search for "AVR family")
// Inline Assembler Cookbook: https://www.nongnu.org/avr-libc/user-manual/inline_asm.html

ISR(PCINT0_vec, ISR_NAKED)
{
    uint8_t data = attiny10_serial_read(UART1_RX_PIN);

    rx_data = data;
    rx_data_available = true;

    // Clear Pin Change Interrupt Flag 0 so we don't fire another PCINT0_vect caused by the line toggeling during the byte we just received
    PCIFR |= _BV(PCIF0);
}

int main(void)
{
    uint8_t data = 0;

    setup();

    _delay_ms(1000);

    attiny10_serial_begin_tx(PB1);
    attiny10_serial_write(PB1, 0x55);
    attiny10_serial_end_tx(PB1);

    attiny10_serial_begin_rx(PB1);

    // Enable global interrupts
    sei();

    while (true)
    {
        if(rx_data_available)
        {
            data = rx_data;
            rx_data_available = false;

            attiny10_serial_end_rx(PB1);
            attiny10_serial_begin_tx(PB1);
            attiny10_serial_write(PB1, ++data);
            attiny10_serial_end_tx(PB1);
            attiny10_serial_begin_rx(PB1);
        }
    }
}