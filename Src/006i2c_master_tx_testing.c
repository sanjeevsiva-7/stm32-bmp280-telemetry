/*
 * 006i2c_master_tx_testing.c
 *
 *  Created on: 21-Sept-2026
 *      Author: LENOVO
 */


#include "stm32f446xx.h"
#include <string.h>

#define MY_ADDR  0x61
#define SlaveAddr 0x68

/*
 * PB6-->I2C1_SCL
 * PB7-->I2C1_SDA
 * ALTERNATE GPIO FUNCTION MODE:4
 */

I2C_Handle_t I2C1Handle;
uint8_t some_data[]= "we are testing I2C master Tx\n";



void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i ++);
}

void I2C1_GPIOInits(){
	GPIO_Handler_t *pI2Cpins;
	pI2Cpins->pGPIOx = GPIOB;
	pI2Cpins->GPIO_PinConf.GPIO_PinMode = GPIO_MODE_ALTFN;
	pI2Cpins->GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	pI2Cpins->GPIO_PinConf.GPIO_PinPuPdControl = GPIO_PIN_PU;
	pI2Cpins->GPIO_PinConf.GPIO_PinAltFunMode = 4;
	pI2Cpins->GPIO_PinConf.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//lets configure SCL
	pI2Cpins->GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&pI2Cpinsr);

	//lets configure SDA
	pI2Cpins->GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_7;
	GPIO_Init(&pI2Cpinsr);


}

void I2C_Inits(void){
	I2C1Handle.pI2Cx =I2C1;
	I2C1Handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
	I2C1Handle.I2C_Config.I2C_DeviceAddress = MY_ADDR;
	I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

	I2C_Init(&I2C1Handle);

}


void GPIO_ButtonInit(void)
{
	GPIO_Handle_t GPIOBtn;

	//this is btn gpio configuration
	GPIOBtn.pGPIOx = GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GPIOBtn);

}


int main(void){

	GPIO_ButtonInit();

	//I2C PIN INITS
	I2C1_GPIOInits();

	//I2C PERIPHERAL CONFIG
	I2C_Inits();

	//ENABLE THE I2C PERPIPHERAL
	I2C_PeripheralControl(I2C1Handle.pI2Cx, ENABLE);

	//SEND SOME DATA TO THE SLAVE
	I2C_MasterSendData(&I2C1Handle,some_data,strlen((char*)some_data), SlaveAddr);


	while(1){
		//wait till button is pressed
		while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

		//to avoid button de-bouncing related issues 200ms of delay
		delay();

		//send some data to the slave
		I2C_MasterSendData(&I2C1Handle,some_data,strlen((char*)some_data),SLAVE_ADDR,0);
	}

}
