#ifndef UART_H_
#define UART_H_

#include <stdbool.h>
#include <avr/io.h>
#include <stdio.h>

void uart_init(void);
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);
bool uart_available();

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

#endif /* UART_H_ */
