#ifndef ADC8321_INCLUDE_H_
#define ADC8321_INCLUDE_H_

#include <limits.h>
#include <stdint.h>
#include "gpio.h"

// IO pins inverted (inverted isolation)
#define ADC8321_PINS_INVERTED 1
// PORT, PIN
#define ADS8321_PIN_CS    D,4 // arduino "D4"
#define ADS8321_PIN_CLK   D,3 // arduino "D3"
#define ADS8321_PIN_DATA  D,2 // arduino "D2"
// TOOD use min clock_time
#define ADS8321_CLK_USEC 8000

static inline void ads8321_init(void) 
{
    GPIO_OUTPUT(ADS8321_PIN_CS, 1);
    GPIO_OUTPUT(ADS8321_PIN_CLK, 1);
    GPIO_INPUT(ADS8321_PIN_DATA, 0);
}

static inline void ads8321_pin_cs_set(uint8_t state) 
{
#if ADC8321_PINS_INVERTED
    state = !state;
#endif
    GPIO_WRITE(ADS8321_PIN_CS, state);
}

static inline void ads8321_pin_clk_set(uint8_t state) 
{
#if ADC8321_PINS_INVERTED
    state = !state;
#endif
    GPIO_WRITE(ADS8321_PIN_CLK, state);
}

static inline uint8_t ads8321_pin_data_get(void) 
{
    uint8_t val = GPIO_READ(ADS8321_PIN_DATA);
#if ADC8321_PINS_INVERTED
    return !val;
#else
    return !!val;
#endif
}


static inline uint8_t ads8321_clock_pulse_read(void) 
{
    ads8321_pin_clk_set(0);
    _delay_us(ADS8321_CLK_USEC / 2);

    ads8321_pin_clk_set(1);

    // Use positive clock edge to read data pin
    uint8_t val = ads8321_pin_data_get();

   _delay_us(ADS8321_CLK_USEC / 2); 

   return val;
}
/**
Minimum 22 clock cycles required for 16-bit conversion. 
not that if CS remains LOW at the end of conversion, a new datastream with
LSB-first is shifted out again.
*/

union ads8321_sample_u {
    uint16_t u16;
    int16_t s16;
    uint8_t u8[2];
};

static inline int16_t ads8321_blocking_read(void) 
{
    uint8_t bit = 0;
    // aussume ADS8321_PIN_CLK already HIGH here
    _delay_us(1);

    ads8321_pin_cs_set(0);

    /* The first 4.5 to 5.0 clock periods of the conversion cycle areused to
     * sample the input signal */
    uint8_t start_sig = 0;
    for (uint8_t i = 0; i < 5; i++) {
        if (ads8321_clock_pulse_read() == 0) {
            start_sig = 1;
        }
    }
    if (!start_sig) {
        // sanity check
        return -1;
    }

    uint16_t data = 0;
    const uint8_t nbits = sizeof(data) * CHAR_BIT;
    for (uint8_t i = 0; i < nbits; i++) {
        bit = ads8321_clock_pulse_read();
        data |= bit;
        data <<= 1;
    }

    ads8321_pin_cs_set(1);
    // "power down". unclear if needed
    for (uint8_t i = 0; i < 10; i++) {
        bit = ads8321_clock_pulse_read();
        (void) bit;
    }

    union ads8321_sample_u sample = { .u16 = data };
    return sample.s16;
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
        ads8321_pin_cs_set(0);
        ctx->rising = 0;
        ctx->count = 0;
        ctx->state = ADS8321_STATE_WAIT;
        return;
    }

    // ensure clock stay high first time after STATE_START
    ctx->rising = !ctx->rising;

    ads8321_pin_clk_set(ctx->rising);

    if (!ctx->rising) {
        // falling clock edge - nothing more to do on 
        return;
    }

    // Use rising clock edge to read data pin. MSB first
    uint8_t bit = ads8321_pin_data_get();
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
            ads8321_pin_cs_set(1);
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
