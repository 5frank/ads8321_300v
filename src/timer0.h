#ifndef TIMER0_INCLUDE_H_
#define TIMER0_INCLUDE_H_

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <stdint.h>

static void timer0_init(void)
{
    uint8_t temp = 0;
    // Timer0 setup
    temp   = _BV(WGM00) | _BV(WGM01);
    TCCR0A = temp;

    temp = TCCR0B;
    temp &= ~(_BV(CS00) | _BV(CS01) | _BV(CS02)); // Clear lower 3 bits
    temp |= _BV(CS01) | _BV(WGM02); // Timer0 clock source = Fio/8, TOP==OCR0A
    //  temp |= _BV(CS00) | _BV(CS02) | _BV(WGM02); // Timer0 clock source =
    //  Fio/1024

    // F_OSC/timer0_div/timer0_out_freq-1 = 16000000/8/57600-1 ~= 34
    OCR0A = 34; // 17.5 us timer period

    TCCR0B = temp; // Start timer
}

static inline void timer0_enable(uint8_t enable)
{
    if (enable) {
        TIMSK0 |= (_BV(OCIE0A));
    } else {
        TIMSK0 &= ~(_BV(OCIE0A));
    }
}

#endif

