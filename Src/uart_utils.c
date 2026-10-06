/*
 * uart_utils.c
 *
 *  Created on: 06-Oct-2026
 *      Author: LENOVO
 */


// uart_utils.c
#include "uart_utils.h"

void uart_print_fixed(USART_Handle_t *pUSARTHandle, int32_t value, uint8_t decimals, const char *unit_suffix)
{
    char buf[16];
    int i = 0;
    uint8_t is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    // build digits in reverse, inserting a decimal point at the right position
    uint8_t digit_count = 0;
    int32_t temp = value;
    do {
        buf[i++] = '0' + (temp % 10);
        temp /= 10;
        digit_count++;
        if (digit_count == decimals) {
            buf[i++] = '.';
        }
    } while (temp > 0 || digit_count < decimals + 1);

    if (is_negative) {
        buf[i++] = '-';
    }

    uint8_t out[32];
    uint8_t len = 0;
    while (i > 0) {
        out[len++] = buf[--i];
    }

    while (*unit_suffix) {
        out[len++] = *unit_suffix++;
    }
    out[len++] = '\r';
    out[len++] = '\n';

    USART_SendData(pUSARTHandle, out, len);
}
