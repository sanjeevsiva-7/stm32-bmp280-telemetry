/*
 * cli.c
 *
 *  Created on: 06-Oct-2026
 *      Author: LENOVO
 */


#include "cli.h"

static char cli_line[CLI_LINE_MAX];
static uint8_t cli_line_index = 0;

RX_ring_buffer_t cli_rx_buffer;  // not static — USART2_IRQHandler in main.c needs to reach this



void cli_init(void){
	RX_ring_buffer_init(&cli_rx_buffer);
	cli_line_index = 0;
}

void cli_poll(void)
{
    uint8_t byte;
    while (RX_ring_buffer_pop(&cli_rx_buffer, &byte)) {
        if (byte == '\r') {
            cli_line[cli_line_index] = '\0';
            cli_process_command(cli_line);
            cli_line_index = 0;
        }
        else if (cli_line_index < (CLI_LINE_MAX - 1)) {
            cli_line[cli_line_index++] = (char)byte;
        }
    }
}

void cli_process_command(char *cmd)
{
    if (strcmp(cmd, "READ") == 0) {
        cli_handle_read();
    }
    else if (strcmp(cmd, "STATUS") == 0) {
        cli_handle_status();
    }
    else if (strcmp(cmd, "DUMP") == 0) {
        cli_handle_dump();
    }
    else if (strcmp(cmd, "CLEAR") == 0) {
        cli_handle_clear();
    }
    else {
        uint8_t msg[] = "Unknown command\r\n";
        USART_SendData(&usart2_handle, msg, sizeof(msg) - 1);
    }
}




void cli_handle_status(void)
{
    uint8_t header[] = "STATUS: ";
    USART_SendData(&usart2_handle, header, sizeof(header) - 1);

    if (sample_log.count == 0) {
        uint8_t msg[] = "no samples yet\r\n";
        USART_SendData(&usart2_handle, msg, sizeof(msg) - 1);
        return;
    }

    // most recent sample is the one just before head (head points to the NEXT write slot)
    uint16_t last_index = (sample_log.head == 0) ? (RING_BUFFER_SIZE - 1) : (sample_log.head - 1);
    sensor_sample_t last = sample_log.buffer[last_index];

    uint8_t msg[] = "last temp=";
    USART_SendData(&usart2_handle, msg, sizeof(msg) - 1);
    uart_print_fixed(&usart2_handle, last.temperature, 2, " C");

    uint8_t msg2[] = "last pressure=";
    USART_SendData(&usart2_handle, msg2, sizeof(msg2) - 1);
    uart_print_fixed(&usart2_handle, last.pressure, 2, " hPa");
}

void cli_handle_read(void)
{
    int32_t raw_press, raw_temp, t_fine;
    bmp280_read_raw(&I2C1Handle, &raw_press, &raw_temp);

    int32_t temp_comp = bmp280_compensate_temperature(&calib, raw_temp, &t_fine);
    uint32_t press_comp = bmp280_compensate_pressure(&calib, raw_press, t_fine);

    uint8_t msg[] = "READ: temp=";
    USART_SendData(&usart2_handle, msg, sizeof(msg) - 1);
    uart_print_fixed(&usart2_handle, temp_comp, 2, " C");

    uint8_t msg2[] = "pressure=";
    USART_SendData(&usart2_handle, msg2, sizeof(msg2) - 1);
    uart_print_fixed(&usart2_handle,  press_comp / 256, 2, " hPa");
}

void cli_handle_dump(void)
{
    uint8_t header[] = "DUMP:\r\n";
    USART_SendData(&usart2_handle, header, sizeof(header) - 1);

    if (sample_log.count == 0) {
        uint8_t msg[] = "no samples stored\r\n";
        USART_SendData(&usart2_handle, msg, sizeof(msg) - 1);
        return;
    }

    uint16_t index = sample_log.tail;
    for (uint16_t i = 0; i < sample_log.count; i++) {
        sensor_sample_t s = sample_log.buffer[index];
        uart_print_fixed(&usart2_handle, s.temperature, 2, " C");
        uart_print_fixed(&usart2_handle, s.pressure , 2, " hPa");
        index = (index + 1) % RING_BUFFER_SIZE;
    }
}


void cli_handle_clear(void)
{
    ring_buffer_init(&sample_log);   // resets head, tail, count — data effectively discarded
    uint8_t msg[] = "Buffer cleared\r\n";
    USART_SendData(&usart2_handle, msg, sizeof(msg) - 1);
}
