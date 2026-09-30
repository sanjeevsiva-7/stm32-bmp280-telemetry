/*
 * stm32f446xx_spi_driver.c
 *
 *  Created on: 16-Sept-2026
 *      Author: SANJEEV
 */

#include "stm32f446xx.h"

static void  spi_txe_interrupt_handle(SPI_Handler_t *pSPIHandle);
static void  spi_rxne_interrupt_handle(SPI_Handler_t *pSPIHandle);
static void  spi_ovr_err_interrupt_handle(SPI_Handler_t *pSPIHandle);
/*********************************************************************
 * @fn      		  - SPI_PeriClockControl
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi) {
	if (EnorDi == ENABLE) {
		if (pSPIx == SPI1) {
			SPI1_PCLK_EN();
		} else if (pSPIx == SPI2) {
			SPI2_PCLK_EN();
		} else if (pSPIx == SPI3) {
			SPI3_PCLK_EN();
		} else if (pSPIx == SPI4) {
			SPI4_PCLK_EN();
		}
	} else {
		if (pSPIx == SPI1) {
			SPI1_PCLK_DI();
		} else if (pSPIx == SPI2) {
			SPI2_PCLK_DI();
		} else if (pSPIx == SPI3) {
			SPI3_PCLK_DI();
		} else if (pSPIx == SPI4) {
			SPI4_PCLK_DI();
		}
	}
}
/*********************************************************************
 * @fn      		  - SPI_Init
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_Init(SPI_Handler_t *pSPIhandle) {
//1.peripheral clock enable (some time user forget to enable the Pclk so we can integrate that API inside this Init API
	SPI_PeriClockControl(pSPIhandle->pSPIx, ENABLE);
// first lets configure the SPI_CR! register
	uint16_t temp_reg = 0;
//1.config the device mode - master or slave
	temp_reg |= pSPIhandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR;
//2.config the bus configuration -FD(same as Simplex_TXOnly),HD,Simplex_RXOnly
	if (pSPIhandle->SPIConfig.SPI_BusConfig == SPI_DEVICE_CONFIG_FD) {
		//bidi mode should be cleared
		temp_reg &= ~(1 << SPI_CR1_BIDIMODE);
	} else if (pSPIhandle->SPIConfig.SPI_BusConfig == SPI_DEVICE_CONFIG_HD) {
		//bidi mode should be set
		temp_reg |= (1 << SPI_CR1_BIDIMODE);
	} else if (pSPIhandle->SPIConfig.SPI_BusConfig== SPI_DEVICE_CONFIG_SIMPLEX_RXONLY) {
		//bidi mode should be cleared
		temp_reg &= ~(1 << SPI_CR1_BIDIMODE);
		//Rx only bit must be set
		temp_reg |= (1 << SPI_CR1_RXONLY);
	}
//3.configure the spi serial clock speed (baurd rate)
	temp_reg |= pSPIhandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR;
//4.configure the DFF
	temp_reg |= pSPIhandle->SPIConfig.SPI_DFF << SPI_CR1_DFF;
//5. configure the CPOL
	temp_reg |= pSPIhandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;
//6. configure the CPHA
	temp_reg |= pSPIhandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;

////here we use this method because the reset value of SPI_CR1 =Reset value: 0x0000
	pSPIhandle->pSPIx->SPI_CR1 = temp_reg;

}

/*********************************************************************
 * @fn      		  - SPI_DeInit
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_DeInit(SPI_RegDef_t *pSPIx) {
	if (pSPIx == SPI1) {
		SPI1_REG_RESET();
	} else if (pSPIx == SPI2) {
		SPI2_REG_RESET();
	} else if (pSPIx == SPI3) {
		SPI3_REG_RESET();
	} else if (pSPIx == SPI4) {
		SPI4_REG_RESET();
	}

}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName) {

	if (pSPIx->SPI_SR & SPI_TXE_FLAG) {
		return FLAG_SET;
	}
	return FLAG_RESET;
}

/*********************************************************************
 * @fn      		  - SPI_SendData
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - This is blocking call (Polling based)

 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len) {
	while (Len > 0) {
		//1.Wait until TX buffer is empty (Wait until TXE  is SET)
		//while(!((pSPIx->SPI_SR >> 1) & 0x1));
		while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);
		//2.Check the DFF bit in CR1
		if (pSPIx->SPI_CR1 & (1 << SPI_CR1_DFF)) {
			//16 bit DFF
			//1. load the data into data register
			pSPIx->SPI_DR = *(uint16_t*) pTxBuffer;
			Len--;
			Len--;
			(uint16_t*) pTxBuffer++;
		} else {
			//8 bit DFF
			//1. load the data into data register
			pSPIx->SPI_DR = *pTxBuffer;
			Len--;
			pTxBuffer++;
		}
	}
}





/*********************************************************************
 * @fn      		  - SPI_ReceiveData
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - This is blocking call (Polling based)

 */
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len) {
	while (Len > 0) {
		//1.(Wait until RXNE( Receive buffer not empty ) is SET)
		while (SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_SET);
		//2.Check the DFF bit in CR1
		if (pSPIx->SPI_CR1 & (1 << SPI_CR1_DFF)) {
			//16 bit DFF
			//1. load the data From data register to RxBuffer address
			*(uint16_t*)pRxBuffer = pSPIx->SPI_DR;
			Len--;
			Len--;
			(uint16_t*)pRxBuffer++;
		} else {
			//8 bit DFF
			//1. load the data From data register to RxBuffer address
			*pRxBuffer = pSPIx->SPI_DR;
			Len--;
			pRxBuffer++;
		}
	}
}



/*********************************************************************
 * @fn      		  - SPI_PeripheralControl
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnaDi) {
	if (EnaDi == ENABLE) {
		pSPIx->SPI_CR1 |= (1 << SPI_CR1_SPE);
	} else {
		pSPIx->SPI_CR1 &= ~(1 << SPI_CR1_SPE);
	}
}

/*********************************************************************
 * @fn      		  - SPI_SSIConfig
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnaDi) {
	if (EnaDi == ENABLE) {
		pSPIx->SPI_CR1 |= (1 << SPI_CR1_SSI);
	} else {
		pSPIx->SPI_CR1 &= ~(1 << SPI_CR1_SSI);
	}
}
/*********************************************************************
 * @fn      		  - SPI_SSOEConfig
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnaDi) {
	if (EnaDi == ENABLE) {
		pSPIx->SPI_CR2 |= (1 << SPI_CR2_SSOE);
	} else {
		pSPIx->SPI_CR2 &= ~(1 << SPI_CR2_SSOE);
	}
}



/*
 * IRQ Configuration and Handling
 */
/*********************************************************************
 * @fn      		  - SPI_IRQInterruptConfig
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber,uint8_t EnorDi){

	if(EnorDi == ENABLE){
		if(IRQNumber <= 31){
			//program the NVIC_ISER0
			*NVIC_ISER0 |= (1 << IRQNumber);

		}else if(IRQNumber >31 && IRQNumber <=63){
			//program the NVIC_ISER1
			*NVIC_ISER1 |= (1 << (IRQNumber%32));

		}else if(IRQNumber >63 && IRQNumber <=95){
			////program the NVIC_ISER2
			*NVIC_ISER2 |= (1 << (IRQNumber%32));
		}
	}else{
		if(IRQNumber <= 31){
			//program the NVIC_ICER0
			*NVIC_ICER0 |= (1 << IRQNumber);

		}else if(IRQNumber >31 && IRQNumber <=63){
			//program the NVIC_ICER1
			*NVIC_ICER1 |= (1 << (IRQNumber%32));

		}else if(IRQNumber >63 && IRQNumber <=95){
			////program the NVIC_ICER2
			*NVIC_ICER2 |= (1 << (IRQNumber%32));
		}

	}

}



/*********************************************************************
 * @fn      		  - SPI_IRQPriorityConfig
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */

void SPI_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority){
	//1.lets find out the IPRx
	uint8_t iprx =  (uint8_t)IRQNumber / 4;
	uint8_t iprx_section =  (uint8_t)IRQNumber % 4;
	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
	NVIC_PR_BASE_ADDR[iprx] |= (IRQPriority << shift_amount);
	// equaltent to // *(NVIC_PR_BASE_ADDR + (iprx ) ) |= IRQPriority << shift_amount);
	}

/*********************************************************************
 * @fn      		  - SPI_SendDataWithIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */

uint8_t SPI_SendDataWithIT(SPI_Handler_t *pSPIHandle,uint8_t *TxBuffer,uint32_t Len){
	uint8_t state = pSPIHandle->TxState;
	if(state != SPI_BUSY_IN_TX){
	//1.Save the Txbuffer address and Length information in some global variables
	pSPIHandle->pTxBuffer = TxBuffer;
	pSPIHandle->TxLen = Len;
	//2.  Mark the SPI state as busy in transmission so that
		// no other code can take over same SPI peripheral until transmission is over
	pSPIHandle->TxState = SPI_BUSY_IN_TX;
	//3. Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
	pSPIHandle->pSPIx->SPI_CR2 |= (1 << SPI_CR2_TXEIE);
	}
	//4. data transmission is handled by the ISR code
	return state;
}
/*********************************************************************
 * @fn      		  - SPI_ReceiveDataWithIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */

uint8_t SPI_ReceiveData_IT(SPI_Handler_t *pSPIHandle,uint8_t *RxBuffer,uint32_t Len){
	uint8_t state = pSPIHandle->RxState;
	if(state != SPI_BUSY_IN_RX){
	//1.Save the Txbuffer address and Length information in some global variables
	pSPIHandle->pRxBuffer = RxBuffer;
	pSPIHandle->RxLen = Len;
	//2.  Mark the SPI state as busy in transmission so that
		// no other code can take over same SPI peripheral until transmission is over
	pSPIHandle->RxState = SPI_BUSY_IN_RX;
	//3. Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
	pSPIHandle->pSPIx->SPI_CR2 |= (1 << SPI_CR2_RXNEIE);
	}
	/*********************************************************************
	 * @fn      		  - SPI_IRQHandling
	 *
	 * @brief             -
	 *
	 * @param[in]         -
	 * @param[in]         -
	 * @param[in]         -
	 *
	 * @return            -
	 *
	 * @Note              -

	 */

	return state;
}

void SPI_IRQHandling(SPI_Handler_t *pHandle){
	//check why the interrupt is happen (check status register of SPI)
	uint8_t temp1,temp2;
	//1.lets check for TXE flag
	temp1 = pHandle->pSPIx->SPI_SR & (1 <<  SPI_SR_TXE);
	temp2 = pHandle->pSPIx->SPI_CR2 & (1 <<  SPI_CR2_TXEIE);

	if(temp1 && temp2){
		//handle TXE
		spi_txe_interrupt_handle(pHandle);//helper function (not an API exposed to the user)
	}

	//2.lets check for RXNE flag
		temp1 = pHandle->pSPIx->SPI_SR & (1 <<  SPI_SR_RXNE);
		temp2 = pHandle->pSPIx->SPI_CR2 & (1 <<  SPI_CR2_RXNEIE);
		if(temp1 && temp2){
			//handle RXNE
			spi_rxne_interrupt_handle(pHandle);//helper function (not an API exposed to the user)
		}

	//3 check for Overrun error
		temp1 = pHandle->pSPIx->SPI_SR & (1 <<  SPI_SR_OVR);
		temp2 = pHandle->pSPIx->SPI_CR2 & (1 <<  SPI_CR2_ERRIE);
		if(temp1 && temp2){
					//handle RXNE
		spi_ovr_err_interrupt_handle(pHandle);//helper function (not an API exposed to the user)
		}

	//4.check for  Master Mode fault event,CRC error,TI frame format error are not implemented
}

//some helper function implementation
static void  spi_txe_interrupt_handle(SPI_Handler_t *pSPIHandle){
	//.Check the DFF bit in CR1
	if (pSPIHandle->pSPIx->SPI_CR1 & (1 << SPI_CR1_DFF)) {
		//16 bit DFF
		//1. load the data into data register
		pSPIHandle->pSPIx->SPI_DR = *((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen--;
		pSPIHandle->TxLen--;
		(uint16_t*)pSPIHandle->pTxBuffer++;
	} else {
		//8 bit DFF
		//1. load the data into data register
		pSPIHandle->pSPIx->SPI_DR = *pSPIHandle->pTxBuffer;
		pSPIHandle->TxLen--;
		pSPIHandle->pTxBuffer++;
	}
	if(pSPIHandle->TxLen == 0){
		//close the SPI transmission and inform the application that TX is over
		SPI_CloseTransmisson(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_TX_CMPLT); //tx complete event
	}
}
static void  spi_rxne_interrupt_handle(SPI_Handler_t *pSPIHandle){
	//do receiving based on the DFF
	if(pSPIHandle->pSPIx->SPI_CR1 & (1 << SPI_CR1_DFF)){
		//16bit
		*((uint16_t*)pSPIHandle->pRxBuffer) = (uint16_t)pSPIHandle->pSPIx->SPI_DR;
		pSPIHandle->RxLen -= 2;
		pSPIHandle->pRxBuffer++;
		pSPIHandle->pRxBuffer++;
	}else{
		//8bit
		*(pSPIHandle->pRxBuffer) = pSPIHandle->pSPIx->SPI_DR;
		pSPIHandle->RxLen--;
		pSPIHandle->pRxBuffer++;
	}
	if(pSPIHandle->RxLen == 0){
		//reception is complete
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_RX_CMPLT);
	}
}
static void  spi_ovr_err_interrupt_handle(SPI_Handler_t *pSPIHandle){
	//1 clear the OVR flag
	//Clearing the OVR bit is done by a read access to the SPI_DR register followed by a read
	//access to the SPI_SR register. (reference manual)
	uint8_t temp;
		//1. clear the ovr flag
		if(pSPIHandle->TxState != SPI_BUSY_IN_TX)
		{
			temp = pSPIHandle->pSPIx->SPI_DR;
			temp = pSPIHandle->pSPIx->SPI_SR;
		}
		(void)temp;//It is a standard C idiom used to silence compiler warnings (specifically -Wunused-variable)
		//2. inform the application
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_OVR_ERR);
}



void SPI_CloseTransmisson(SPI_Handler_t *pSPIHandle)
{
	pSPIHandle->pSPIx->SPI_CR2 &= ~( 1 << SPI_CR2_TXEIE); //// this prevent the interrupt from setting of TXE flag
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;

}

void SPI_CloseReception(SPI_Handler_t *pSPIHandle)
{
	pSPIHandle->pSPIx->SPI_CR2 &= ~( 1 << SPI_CR2_RXNEIE);//// this prevent the interrupt from setting of RXNE flag
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;

}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx){
	uint8_t temp;
	temp = pSPIx->SPI_DR;
	temp = pSPIx->SPI_SR;
    (void)temp;
}

__weak void SPI_ApplicationEventCallback(SPI_Handler_t *pSPIHandle,uint8_t AppEv){ //AppEv:application event
	////This is a weak implementation . the user application may override this function.
}
