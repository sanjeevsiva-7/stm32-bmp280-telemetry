/*
 * stm32f446xx_spi_driver.h
 *
 *  Created on: 16-Sept-2026
 *      Author: SANJEEV
 */

#ifndef INC_STM32F446XX_SPI_DRIVER_H_
#define INC_STM32F446XX_SPI_DRIVER_H_

#include "stm32f446xx.h"


/*
 * This is a Configuration structure for a GPIO pin
 */
typedef struct{
	uint8_t SPI_DeviceMode;
	uint8_t SPI_BusConfig;
	uint8_t SPI_SclkSpeed;
	uint8_t SPI_DFF;
	uint8_t SPI_CPOL;
	uint8_t SPI_CPHA;
	uint8_t SPI_SSM;
}SPI_Config_t;

/*
 * This is a Handle Structure for a GPIO pin
 */
typedef struct{
	SPI_RegDef_t *pSPIx;
	SPI_Config_t SPIConfig;

	uint8_t 		*pTxBuffer; /* !< To store the app. Tx buffer address > */
	uint8_t 		*pRxBuffer;	/* !< To store the app. Rx buffer address > */
	uint32_t 		TxLen;		/* !< To store Tx len > */
	uint32_t 		RxLen;		/* !< To store Tx len > */
	uint8_t 		TxState;	/* !< To store Tx state > */
	uint8_t 		RxState;	/* !< To store Rx state > */
}SPI_Handler_t;

/*
 *  Possible SPI Application States
 */
#define SPI_READY		0
#define SPI_BUSY_IN_RX	1
#define SPI_BUSY_IN_TX	2

/*
 * possible spi application event
 */
#define SPI_EVENT_TX_CMPLT  1
#define SPI_EVENT_RX_CMPLT  2
#define SPI_EVENT_OVR_ERR   3


//possible value
/*
 * @SPI_DeviceMode
 */
#define SPI_DEVICE_MODE_MASTER 1
#define SPI_DEVICE_MODE_SLAVE  0

/*
 * @SPI_BusConfig
 */
#define SPI_DEVICE_CONFIG_FD				0
#define SPI_DEVICE_CONFIG_HD				1
//#define SPI_DEVICE_CONFIG_SIMPLEX_TXONLY	SPI_DEVICE_CONFIG_FD
#define SPI_DEVICE_CONFIG_SIMPLEX_RXONLY	3

/*
 * @SPI_SclkSpeed
 */
#define SPI_SCLK_SPEED_DIV2		0
#define SPI_SCLK_SPEED_DIV4		1
#define SPI_SCLK_SPEED_DIV8		2
#define SPI_SCLK_SPEED_DIV16	3
#define SPI_SCLK_SPEED_DIV32	4
#define SPI_SCLK_SPEED_DIV64	5
#define SPI_SCLK_SPEED_DIV128	6
#define SPI_SCLK_SPEED_DIV256	7

/*
 * @SPI_DFF
 */
#define SPI_DFF_8BITS	0
#define SPI_DFF_16BITS  1

/*
 *@SPI_CPOL
 */
#define SPI_CPOL_LOW	0
#define SPI_CPOL_HIGH	1

/*
 * @SPI_CPHA
 */
#define SPI_CPHA_LOW	0
#define SPI_CPHA_HIGH	1

/*
 * @SPI_SSM
 */
#define SPI_SSM_DI 0
#define SPI_SSM_EN 1


/*
 * SPI related Stautus flags definition
 */
#define SPI_TXE_FLAG  (1 << SPI_SR_TXE)
#define SPI_RXNE_FLAG (1 << SPI_SR_RXNE)
#define SPI_BSY_FLAG  (1 << SPI_SR_BSY)



/******************************************************************************************
 *								APIs supported by this driver
 *		 For more information about the APIs check the function definitions
 ******************************************************************************************/
/*
 * Peripheral Clock setup
 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx,uint8_t EnaDi);

/*
 * Init and De-init
 */
void SPI_Init(SPI_Handler_t *pSPIhandle);
void SPI_DeInit(SPI_RegDef_t *pSPIx);
/*
 * Data Send and Receive
 */
//1.POLLING(BLOCKING) BASED API
void SPI_SendData(SPI_RegDef_t *pSPIx,uint8_t *TxBuffer,uint32_t Len);  //Using uint32_t for the length parameter (Len) is an industry best practice in 32-bit embedded development for three primary reasons:
void SPI_ReceiveData(SPI_RegDef_t *pSPIx,uint8_t *RxBuffer,uint32_t Len);
//2.INTERRUPT (BLOCKING) BASED API
uint8_t SPI_SendDataWithIT(SPI_Handler_t *pSPIHandle,uint8_t *TxBuffer,uint32_t Len);
uint8_t SPI_ReceiveDataWithIT(SPI_Handler_t *pSPIHandle,uint8_t *RxBuffer,uint32_t Len);


/*
 * IRQ Configuration and ISR handling
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber,uint8_t EnorDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority);
void SPI_IRQHandling(SPI_Handler_t *pHandle);

/*
 * other peripheral control API
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnaDi);
void SPI_SSIConfig(SPI_RegDef_t *pSPIx,uint8_t EnaDi);
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx,uint8_t EnaDi);
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx , uint32_t FlagName);
void SPI_CloseTransmisson(SPI_Handler_t *pSPIHandle);
void SPI_CloseReception(SPI_Handler_t *pSPIHandle);
void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
/*
 * application callback API
 */
void SPI_ApplicationEventCallback(SPI_Handler_t *pSPIHandle,uint8_t AppEv);


#endif /* INC_STM32F446XX_SPI_DRIVER_H_ */
