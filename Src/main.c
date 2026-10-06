#include "main.h"
/* ---- Peripheral handles ---- */
I2C_Handle_t   I2C1Handle;
USART_Handle_t usart2_handle;

/* ---- Sensor calibration + sample storage ---- */
bmp280_calib_t calib;
ring_buffer_t  sample_log;

/* ---- Timer flag, set by TIM6 ISR, cleared in main loop ---- */
volatile uint8_t sample_ready = 0;


int main(void)
{
    /* 1. Bring up peripherals */
    I2C1_GPIOInits();
    I2C1_Inits();
    USART2_GPIOInits();
    USART2_Inits();
    TIM6_Init();

    I2C_PeripheralControl(I2C1, ENABLE);
    I2C_ManageAcking(I2C1, I2C_ACK_ENABLE);   // must come AFTER PE=1, or ACK silently fails
    USART_PeripheralControl(USART2, ENABLE);

    /* 2. Enable CLI's UART RX interrupt, init CLI + sensor ring buffer */
    USART2_RX_Interrupt_Enable();
    cli_init();
    ring_buffer_init(&sample_log);

    /* 3. Sensor one-time setup */
    bmp280_set_mode(&I2C1Handle);
    bmp280_read_calibration(&I2C1Handle, &calib);

    /* 4. Main loop — non-blocking, services both the 1Hz sample flag and CLI input */
    while (1)
    {
        if (sample_ready) {
            sample_ready = 0;

            int32_t  raw_press, raw_temp, t_fine;
            bmp280_read_raw(&I2C1Handle, &raw_press, &raw_temp);

            int32_t  temp_comp  = bmp280_compensate_temperature(&calib, raw_temp, &t_fine);
            uint32_t press_comp = bmp280_compensate_pressure(&calib, raw_press, t_fine);

            sensor_sample_t s;
            s.temperature = temp_comp;
            s.pressure    = press_comp / 256;
            ring_buffer_push(&sample_log, s);

            //uart_print_decimal(&usart2_handle, s.temperature);
            //uart_print_decimal(&usart2_handle, s.pressure);
            uart_print_fixed(&usart2_handle, s.temperature, 2, " C");
            uart_print_fixed(&usart2_handle, s.pressure, 2, " hPa");
        }

        cli_poll();
    }
}

/* ---- Decimal print helper ---- */
void uart_print_decimal(USART_Handle_t *pUSARTHandle, int32_t value)
{
    char buf[12];
    int i = 0;
    uint8_t is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    if (value == 0) {
        buf[i++] = '0';
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    uint8_t out[12];
    uint8_t len = 0;
    while (i > 0) {
        out[len++] = buf[--i];
    }
    out[len++] = '\r';
    out[len++] = '\n';

    USART_SendData(pUSARTHandle, out, len);
}

/* ---- Interrupt Service Routines ---- */

void TIM6_DAC_IRQHandler(void)
{
    if (TIM6->SR & (1 << TIM_SR_UIF)) {
        TIM6->SR &= ~(1 << TIM_SR_UIF);   // must clear manually, or it re-fires instantly
        sample_ready = 1;
    }
}

void USART2_IRQHandler(void)
{
    if (usart2_handle.pUSARTx->USART_SR & (1 << USART_SR_RXNE)) {
        uint8_t received_byte = (uint8_t)(usart2_handle.pUSARTx->USART_DR);
        // reading DR above already cleared RXNE — no manual clear needed
        RX_ring_buffer_push(&cli_rx_buffer, received_byte);
    }
}
