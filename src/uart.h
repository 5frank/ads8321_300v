#ifndef UART_INCLUDE_H_
#define UART_INCLUDE_H_

#include "cmd.h"

void uart_init(void);
void uart_cmd_process(cmd_cb *cb);
void uart_putc(char c);
void uart_puts(const char *s);

#endif

