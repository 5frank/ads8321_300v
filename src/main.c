#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <util/delay.h>
#include <avr/cpufunc.h> 
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "rbuf.h"
#include "cmd.h"

#include "uart.h"
#include "timer0.h"
#include "ads8321.h"

static struct ads8321_data g_ads8321_data = { 0 };


ISR(TIMER0_COMPA_vect)
{
#if 0
    struct ads8321_data *ads = &g_ads8321_data;
    ads8321_clk_tick_update(ads);
    if (ads->state == ADS8321_STATE_DONE) {
        //rbuf_enqueue(&g_rbuf, ctx->sample);
    }
#endif
}

static void clock_init(void)
{
    CLKPR = _BV(CLKPCE);
    CLKPR = 0;
}

static void main_init(void)
{
    cli();

    clock_init();
    ads8321_init();
    //timer0_init();
    uart_init();

    sei();
}
/**
 * raw == 124 == +10V
 * raw == 62 == +5V
 * raw == -1980 == -10V
 */ 
static int16_t convert_sample(uint16_t data) 
{
    int16_t val;

#if 1
    const uint16_t zero_offs = 1 << 15;
    if (data > zero_offs) {
        val = data - zero_offs;
    }
    else {
        val = -((int16_t)(data - zero_offs));
    }

#else
    const uint16_t msb = 1 << 15;
    if (data & msb) {
        data &= ~msb;
        val =  ((int16_t)data);
    } else {
        data &= ~msb;
        val =  -((int16_t)data);
    }
#endif
    return val;
}

static void print_s16(int16_t sample)
{
    static char buf[sizeof("65535") + 2];
    // note nsigned int at least 16 bit (in standard C)
    itoa(sample, buf, 10);
    uart_puts(buf);
    uart_putc('\n');
}

static void print_u16(uint16_t sample)
{
    static char buf[sizeof("65535") + 2];
    // note unsigned int at least 16 bit (in standard C)
    utoa(sample, buf, 10);
    uart_puts(buf);
    uart_putc('\n');
}

static void main_cmd(struct cmd_args *args)
{
/* TODO
# start dump data
start --format={bin,csv,ascii}

#stop
stop

# get single
get --nsamples=3
*/
}
int main(void)
{
    main_init();

    while (1) {
        int16_t sample = ads8321_blocking_read();
        print_s16(sample);
        //uart_cmd_process(main_cmd);
        _delay_ms(500);
    }

    return 0;
}


