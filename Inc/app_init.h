/*
 * app_init.h
 *
 *  Created on: 06-Oct-2026
 *      Author: LENOVO
 */

#ifndef APP_INIT_H_
#define APP_INIT_H_

#include "stm32f446xx.h"

/* I2C1 setup */
void I2C1_GPIOInits(void);
void I2C1_Inits(void);

/* USART2 setup */
void USART2_GPIOInits(void);
void USART2_Inits(void);
void USART2_RX_Interrupt_Enable(void);

/* TIM6 setup */
void TIM6_IRQ_Config(void);
void TIM6_Init(void);

/* Shared peripheral handles — defined for real in main.c, used here via extern */
extern I2C_Handle_t   I2C1Handle;
extern USART_Handle_t usart2_handle;

#endif /* APP_INIT_H_ */
