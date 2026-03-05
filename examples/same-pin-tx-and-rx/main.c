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
}

static void setup()
{
    cli();

    setupPowerSaving();
    setupClock();
    setupSleep();
    setupIO();
    attiny10_serial_begin_tx();

    sei();
}

// Resources:
// AVR Instruction Set Manual: https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/ReferenceManuals/AVR-InstructionSet-Manual-DS40002198.pdf
// GCC Manual: Using inline assembly in C:
// - Overview: https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html
// - Details: https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
// - AVR Specifics: https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html (Search for "AVR family")
// Inline Assembler Cookbook: https://www.nongnu.org/avr-libc/user-manual/inline_asm.html

int main(void)
{
    uint8_t j = 0;

    setup();
    LED_ON();

    while (true)
    {
        attiny10_serial_write(++j);
    }
}