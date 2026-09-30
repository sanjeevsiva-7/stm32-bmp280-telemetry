/*
 * 005spi_tx_testing.c
 *
 *  Created on: 17-Sept-2026
 *      Author: LENOVO
 */


/*
 * PB15-->SPI2_MOSI
 * PB14-->SPI2_MISO
 * PB13-->SPI2_SCK
 * PB12-->SPI2_NSS
 * ALTERNATE GPIO FUNCTION MODE:5
 */
#include "stm32f446xx.h"
#include <string.h>

void SPI2_GPIO_Inits(void){
	GPIO_Handler_t SPIpins;
	SPIpins.pGPIOx = GPIOB;
	SPIpins.GPIO_PinConf.GPIO_PinMode =GPIO_MODE_ALTFN;
	SPIpins.GPIO_PinConf.GPIO_PinAltFunMode = 5;
	SPIpins.GPIO_PinConf.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIpins.GPIO_PinConf.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIpins.GPIO_PinConf.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//SCK
	SPIpins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIpins);
	//MOSI
	SPIpins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&SPIpins);
	//MISO -here there is no slave ,no MISO required
	//SPIpins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_14;
	//GPIO_Init(&SPIpins);
	//NSS --here there is no slave ,no NSS required
	//SPIpins.GPIO_PinConf.GPIO_PinNumber = GPIO_PIN_NO_12;
	//GPIO_Init(&SPIpins);
}

void SPI2_Init(void){
	SPI_Handler_t SPI2Handle;
	SPI2Handle.pSPIx = SPI2;
	SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	SPI2Handle.SPIConfig.SPI_BusConfig = SPI_DEVICE_CONFIG_FD;
	SPI2Handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_DI;
	SPI2Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV2; //generate sclk of 8Mhz

	SPI_Init(&SPI2Handle);
}

int main(void)
{
	char user_Data[] = "Hello World";
    // This function is used to initialize the GPIO Pins behave as SPI Pin
	SPI2_GPIO_Inits();
	//This function is used to initialize the SPI2 peripheral parameters
	SPI2_Init();
	//this makes the NSS signal internally High and avoid MODF error
	SPI_SSIConfig(SPI2,ENABLE);
	//enable the SPI2 peripheral
	SPI_PeripheralControl(SPI2, ENABLE);
	//to send data
	SPI_SendData(SPI2, (uint8_t*) user_Data, strlen(user_Data));
	//1. lets confirm that SPI is not busy, w8 until spi is not busy
	while(SPI_GetFlagStatus(SPI2,SPI_BSY_FLAG) == FLAG_SET);
	//disable the SPI2 peripheral
	SPI_PeripheralControl(SPI2, DISABLE);

	/* Loop forever */
	for(;;);
}
