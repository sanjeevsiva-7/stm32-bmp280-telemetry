/*
 * cli.h
 *
 *  Created on: 06-Oct-2026
 *      Author: LENOVO
 */

#ifndef CLI_H_
#define CLI_H_


#include "rx_ring_buffer.h"
#include "stm32f446xx.h"
#include <string.h>
#include "ring_buffer.h"
#include "uart_utils.h"

#define CLI_LINE_MAX 32




extern USART_Handle_t usart2_handle;   // declared in main.c, used here for sending responses
extern ring_buffer_t sample_log;   // defined in main.c, reach in via extern=
extern I2C_Handle_t I2C1Handle;
extern bmp280_calib_t calib;
extern RX_ring_buffer_t cli_rx_buffer;



void cli_init(void);
void cli_poll(void);
void cli_process_command(char *cmd);
void cli_handle_read(void);
void cli_handle_status(void);
void cli_handle_dump(void);
void cli_handle_clear(void);





#endif /* CLI_H_ */
