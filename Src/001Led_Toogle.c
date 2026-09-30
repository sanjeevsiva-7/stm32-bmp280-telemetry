/*
 * 001Led_Toogle.c
 *
 *  Created on: 13-Sept-2026
 *      Author: LENOVO
 */

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"

void delay(void){
	for(uint32_t i = 0; i < 500000; i++);
}

int main(void){
	GPIO_Handler_t GpioLed;
	GpioLed.pGPIOx = GPIOA;
	GpioLed.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_5;
	GpioLed.GPIO_PinConf.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	GpioLed.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_PIN_PU;

	//API CALLING
	GPIO_PeriClockControl(GPIOA,ENABLE);
	GPIO_Init(&GpioLed);
	while(1){
		GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
		delay();
	}

	return 0;
}
