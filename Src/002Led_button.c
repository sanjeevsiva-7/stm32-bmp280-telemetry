/*
 * 002Led_button.c
 *
 *  Created on: 13-Sept-2026
 *      Author: LENOVO
 */



#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"

#define BTN_PRESSED LOW

void delay(void){
	for(uint32_t i = 0; i < 500000; i++);
}

int main(void){
	GPIO_Handler_t GpioLed,GpioButton;
	GpioLed.pGPIOx = GPIOA;
	GpioLed.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_5;
	GpioLed.GPIO_PinConf.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConf.GPIO_PinSpeed =GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	//API CALLING
	GPIO_PeriClockControl(GPIOA,ENABLE);
	GPIO_Init(&GpioLed);



	GpioButton.pGPIOx = GPIOC;
	GpioButton.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_13;
	GpioButton.GPIO_PinConf.GPIO_PinMode = GPIO_MODE_IN;
	GpioButton.GPIO_PinConf.GPIO_PinSpeed =GPIO_SPEED_FAST;
	//GpioButton.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	GpioButton.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_NO_PUPD;

		//API CALLING
		GPIO_PeriClockControl(GPIOC,ENABLE);
		GPIO_Init(&GpioButton);


	while(1){
		if(GPIO_ReadFromInputPin(GPIOC,GPIO_PIN_NO_13) == BTN_PRESSED){
		delay();
		GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
		}

	}

	return 0;
}
