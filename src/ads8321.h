#ifndef ADC8321_INCLUDE_H_
#define ADC8321_INCLUDE_H_

#include <limits.h>
#include <stdint.h>
#include "gpio.h"

// PORT, PIN
#define ADS8321_PIN_CS   D,4
#define ADS8321_PIN_CLK  D,4
#define ADS8321_PIN_DATA  D,3
// TOOD use min clock_time
#define ADS8321_CLK_USEC 1000

static inline void ads8321_init(void) 
{
    GPIO_OUTPUT(ADS8321_PIN_CS, 1);
    GPIO_OUTPUT(ADS8321_PIN_CLK, 1);
    GPIO_INPUT(ADS8321_PIN_DATA, false);
}

static inline uint8_t ads8321_clock_pulse_read(void) 
{
        GPIO_WRITE(ADS8321_PIN_CLK, 0);

        _delay_us(ADS8321_CLK_USEC / 2);

        GPIO_WRITE(ADS8321_PIN_CLK, 1);
        // Use positive clock edge to read data pin
        uint8_t val = GPIO_READ(ADS8321_PIN_DATA);

       _delay_us(ADS8321_CLK_USEC / 2); 

       return val ? 1 : 0;
}
/**
Minimum 22 clock cycles required for 16-bit conversion. 
not that if CS remains LOW at the end of conversion, a new datastream with
LSB-first is shifted out again.
*/
static inline uint16_t ads8321_blocking_read(void) 
{
    // aussume ADS8321_PIN_CLK already HIGH here
    _delay_us(1);

    GPIO_WRITE(ADS8321_PIN_CS, 0);
    _delay_us(20); // needed?

    /* The first 4.5 to 5.0 clock periods of the conversion cycle areused to
     * sample the input signal */
    uint8_t bit = 0;
    for (uint8_t i = 0; i < 7; i++) {
        bit = ads8321_clock_pulse_read();
        if (bit == 0) {
            break;
        }
    }

    uint16_t sample = 0;
    const uint8_t nbits = sizeof(sample) * CHAR_BIT;
    for (uint8_t i = 0; i < nbits; i++) {

        bit = ads8321_clock_pulse_read();
        sample |= bit;
        sample <<= 1;
    }

    GPIO_WRITE(ADS8321_PIN_CS, 1);

    return sample;
}

enum ads8321_state_e {
    ADS8321_STATE_START = 0,
    ADS8321_STATE_WAIT,
    ADS8321_STATE_READ,
    ADS8321_STATE_DONE,
    ADS8321_STATE_END,
};

struct ads8321_data {
    uint16_t sample;
    uint8_t state;
    uint8_t count;
    uint8_t rising;
};

static void ads8321_clk_tick_update(struct ads8321_data *ctx)
{
    if (ctx->state == ADS8321_STATE_START) {
        GPIO_WRITE(ADS8321_PIN_CS, 0);
        ctx->rising = 0;
        ctx->count = 0;
        ctx->state = ADS8321_STATE_WAIT;
        return;
    }

    // ensure clock stay high first time after STATE_START
    ctx->rising = !ctx->rising;

    GPIO_WRITE(ADS8321_PIN_CLK, ctx->rising);

    if (!ctx->rising) {
        // falling clock edge - nothing more to do on 
        return;
    }

    // Use rising clock edge to read data pin. MSB first
    uint8_t bit = GPIO_READ(ADS8321_PIN_DATA) ? 1 : 0;

    /* wait for DATA_PIN LOW, which indicates sample is ready and will start on
     * next rising clock edge..
     * Minimum 22 clock cycles required for 16-bit conversion. */
    if (ctx->state == ADS8321_STATE_WAIT) { 
        if (bit == 0) {
            // prepare next state
            ctx->sample = 0;
            ctx->count = 0;
            ctx->state = ADS8321_STATE_READ;
        }
        return;
    }

    /* read sample bits. MSB received first. */
    if (ctx->state == ADS8321_STATE_READ) { 
        if (ctx->count < 16) {
            ctx->sample |= bit;
            ctx->sample <<= 1;
            ctx->count++;
        }
        else {
            GPIO_WRITE(ADS8321_PIN_CS, 1);
            // prepare next state
            ctx->count = 0;
            ctx->state = ADS8321_STATE_DONE;
        }
        return;
    }

    /* one "clock tick" in DONE state for caller to retrive sample once */
    if (ctx->state == ADS8321_STATE_DONE) {
        // prepare next state
        ctx->count = 0;
        ctx->state = ADS8321_STATE_END;
        return;
    }

    /*
     * extra clock pulses after sample read for "power down". needed?
     * note: If CS remains LOW at the end of conversion, a new datastream with
     * LSB-first is shifted out again. */
    if (ctx->state == ADS8321_STATE_END) {
        if (ctx->count < 5) {
            ctx->count++;
        }
        else {
            ctx->count = 0;
            ctx->state = ADS8321_STATE_START;
        }
        return;
    }
}
#endif
