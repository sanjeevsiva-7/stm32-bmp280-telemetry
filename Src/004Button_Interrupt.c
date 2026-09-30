/*
 * 004Button_Interrupt.c
 *
 *  Created on: 14-Sept-2026
 *      Author: LENOVO
 */


/*
 * 003Led_EXT_Button.c
 *
 *  Created on: 13-Sept-2026
 *      Author: LENOVO
 */

#include<string.h>
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"

#define BTN_PRESSED LOW

void delay(void){
	for(uint32_t i = 0; i < 500000; i++);
}

int main(void){
	GPIO_Handler_t GpioLed,GpioButton;
	memset(&GpioLed,0,sizeof(GpioLed));
	memset(&GpioButton,0,sizeof(GpioButton));

	GpioLed.pGPIOx = GPIOA;
	GpioLed.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_4;
	GpioLed.GPIO_PinConf.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConf.GPIO_PinSpeed =GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	//API CALLING
	GPIO_PeriClockControl(GPIOA,ENABLE);
	GPIO_Init(&GpioLed);


 // this is button gpio configuration
	GpioButton.pGPIOx = GPIOB;
	GpioButton.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_5;
	GpioButton.GPIO_PinConf.GPIO_PinMode = GPIO_MODE_IT_FT;
	GpioButton.GPIO_PinConf.GPIO_PinSpeed =GPIO_SPEED_FAST;
	//GpioButton.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	GpioButton.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_PIN_PU;


	//API CALLING
	GPIO_PeriClockControl(GPIOB,ENABLE);
	GPIO_Init(&GpioButton);

	//this is GPIO IRQ configuration
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI9_5,NVIC_IRQ_PRIORITY_15), //IRQPriority - only 4 bits are implemented so possible values 0-15
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, ENABLE);

	while(1);

	return 0;
}


void EXTI9_5_IRQHandler(void){
	delay(); //wait till button de-bouncing gets over
	GPIO_IRQHandling(GPIO_PIN_NO_5);
	GPIO_ToggleOutputPin(GPIOA,GPIO_PIN_NO_4);
}
