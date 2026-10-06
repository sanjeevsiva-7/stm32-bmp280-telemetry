/*
 * app_init.c
 *
 *  Created on: 03-Oct-2026
 *      Author: LENOVO
 */
#include "stm32f446xx.h"
#include "app_init.h"



void I2C1_GPIOInits(void)
{
    GPIO_Handler_t I2CPins;
    I2CPins.pGPIOx = GPIOB;
    I2CPins.GPIO_PinConf.GPIO_PinMode = GPIO_MODE_ALTFN;
    I2CPins.GPIO_PinConf.GPIO_PinAltFunMode = 4;      // AF4 = I2C1 on PB8/PB9
    I2CPins.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_OD;   // open-drain, required for I2C
    I2CPins.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_NO_PUPD;  // internal pull-up (backup if board lacks one)
    I2CPins.GPIO_PinConf.GPIO_PinSpeed = GPIO_SPEED_FAST;

    I2CPins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_8;  // SCL
    GPIO_Init(&I2CPins);

    I2CPins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_9;  // SDA
    GPIO_Init(&I2CPins);
}




void I2C1_Inits(void)
{
    I2C1Handle.pI2Cx = I2C1;
    I2C1Handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
    I2C1Handle.I2C_Config.I2C_DeviceAddress = 0x61;   // arbitrary, unused since we're master-only
    I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_CYCLE_2;//unused parameter
    I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;  // 100kHz, safest to start

    I2C_Init(&I2C1Handle);
}



void USART2_GPIOInits(void)
{
    GPIO_Handler_t USARTPins;
    USARTPins.pGPIOx = GPIOA;
    USARTPins.GPIO_PinConf.GPIO_PinMode = GPIO_MODE_ALTFN;
    USARTPins.GPIO_PinConf.GPIO_PinAltFunMode = 7;   // AF7 = USART2 on PA2/PA3
    USARTPins.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    USARTPins.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    USARTPins.GPIO_PinConf.GPIO_PinSpeed = GPIO_SPEED_FAST;

    USARTPins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_2;  // TX
    GPIO_Init(&USARTPins);

    USARTPins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_3;  // RX
    GPIO_Init(&USARTPins);
}


void USART2_Inits(void)
{
    usart2_handle.pUSARTx = USART2;
    usart2_handle.USART_Config.USART_Baud = USART_STD_BAUD_115200;
    usart2_handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
    usart2_handle.USART_Config.USART_Mode = USART_MODE_TXRX;
    usart2_handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
    usart2_handle.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
    usart2_handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;

    USART_Init(&usart2_handle);
}




void TIM6_IRQ_Config(void)
{

    *NVIC_ISER1 |= (1 << (IRQ_NO_TIM6_DAC % 32));   // 54 falls in ISER1 range (32–63)
}

void TIM6_Init(void)
{
    TIM6_PCLK_EN();

    TIM6->PSC = 15999;   // 16MHz / 16000 = 1kHz tick rate
    TIM6->ARR = 999;     // 1000 ticks = 1 second period

    TIM6->DIER |= (1 << TIM_DIER_UIE);   // enable update interrupt

    TIM6_IRQ_Config();   // NVIC side, next step

    TIM6->CR1 |= (1 << TIM_CR1_CEN);     // start counting
}


void USART2_RX_Interrupt_Enable(void)
{
    usart2_handle.pUSARTx->USART_CR1 |= (1 << USART_CR1_RXNEIE);

    // NVIC enable — IRQ_NO_USART2 = 38, falls in ISER1 range (32-63)
    *NVIC_ISER1 |= (1 << (IRQ_NO_USART2 % 32));
}


