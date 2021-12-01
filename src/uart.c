#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#define UART_BAUD (115200)


static struct uart_rxbuf {
    uint8_t pendig;
    uint8_t dropped;
    uint8_t size;
    char buf[64];
} g_uart_rxbuf;

/* UART Tx Complete */
ISR(USART_TX_vect) 
{

}

ISR(USART_RX_vect)
{
    struct uart_rxbuf *rxb =  &g_uart_rxbuf;
    char c = UDR0;

    if (rxb->pendig) {
        rxb->dropped++;
        return;
    }

    if (rxb->size >= (sizeof(rxb->buf) - 1)) {
        rxb->dropped += rxb->size;
        rxb->buf[0] = '\0';
        rxb->size = 0;
        return;
    }

    rxb->buf[rxb->size] = c;
    rxb->size++;

    if (c == '\n' || c == '\r') {
        rxb->pendig = true;
    }
}


static struct cmd_args g_uart_cmd_args = {0};

void uart_cmd_process(cmd_cb *cb)
{
    struct uart_rxbuf *rxb =  &g_uart_rxbuf;
    static struct cmd_args *args = &g_uart_cmd_args;

    if (!rxb->pendig) {
        return;
    }
    cmd_make_argv(args, rxb->buf);
    cb(args);

    rxb->pendig = false;
}


void uart_init(void)
{
#if (F_CPU == 16000000) && (UART_BAUD == 115200)

    UCSR0A |= 1 << U2X0;
    UBRR0H = 0;
    UBRR0L = 16;
#else
#warning "uart config not tested"
    uint16_t ubrr;
    if (UART_BAUD & 0x8000) {
        UCSR0A |= 1 << U2X0;
        ubrr = F_CPU / (8 * UART_BAUD) - 1;
    } else {
        ubrr = F_CPU / (16 * UART_BAUD) - 1;
    }

    UBRR0H = (uint8_t)((ubrr >> 8) & 0xf);
    UBRR0L = (uint8_t)(ubrr & 0xff);
#endif
    UCSR0B = _BV(RXCIE0);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);

    UCSR0B |= _BV(RXCIE0);
}

void uart_putc(char c)
{
    uint16_t cnt = 10000;
    while (!(UCSR0A & _BV(UDRE0)) && cnt) {
        cnt--;
    }

    if (cnt == 0)
        return;

    UDR0 = (uint8_t) c;
}

void uart_puts(const char *s)
{
    while (1) {
        char c = *s++;
        if (c == '\0') {
            break;
        }
        uart_putc(c);
    }
}

