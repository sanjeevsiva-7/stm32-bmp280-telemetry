/*
 * main.h
 *
 *  Created on: 06-Oct-2026
 *      Author: LENOVO
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f446xx.h"
#include "app_init.h"
#include "bmp280.h"
#include "ring_buffer.h"
#include "cli.h"



/* ---- Forward declarations for small UART print helpers ---- */
void uart_print_decimal(USART_Handle_t *pUSARTHandle, int32_t value);



#endif /* MAIN_H_ */
