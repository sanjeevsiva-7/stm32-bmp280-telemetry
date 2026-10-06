/*
 * uart_utils.h
 *
 *  Created on: 06-Oct-2026
 *      Author: LENOVO
 */

#ifndef UART_UTILS_H_
#define UART_UTILS_H_

#include "stm32f446xx.h"

void uart_print_fixed(USART_Handle_t *pUSARTHandle, int32_t value, uint8_t decimals, const char *unit_suffix);

#endif /* UART_UTILS_H_ */
