/*
 * stm32f446xx_i2c_driver.c
 *
 *  Created on: 20-Sept-2026
 *      Author: SANJEEV
 */

#include "stm32f446xx.h"

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_GenerateStoptCondition(I2C_RegDef_t *pI2Cx); //if the application requires stop generation API ,then "static" can be cleared

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx) {
	pI2Cx->I2C_CR1 |= (1 << I2C_CR1_START);
}

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr) {
	SlaveAddr = (SlaveAddr << 1);
	SlaveAddr &= ~(1 << 0); //here slave address means  slave address + R/nW bit=0
	pI2Cx->I2C_DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr) {
	SlaveAddr = (SlaveAddr << 1);
	SlaveAddr |= (1 << 0); //here slave address means  slave address + R/nW bit=1
	pI2Cx->I2C_DR = SlaveAddr;
}

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle) {
	uint32_t dummyRead;
//check the device mode
	if (pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL)) {
		//device in Master mode
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
			//Master in Rx
			if (pI2CHandle->RxSize == 1) {
				//1. disable the ACK
				I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);
				//2. clear the ADDR flag (ie 1)read SR1  2)read SR2)
				dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
				dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
				(void) dummyRead;

			}
		} else {
			//Master in Tx
			// clear the ADDR flag (ie 1)read SR1  2)read SR2)
			dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
			dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
			(void) dummyRead;

		}

	} else {
		//device in slave mode

		//. clear the ADDR flag (ie 1)read SR1  2)read SR2)
		dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
		dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
		(void) dummyRead;
	}

}

static void I2C_GenerateStoptCondition(I2C_RegDef_t *pI2Cx) {
	pI2Cx->I2C_CR1 |= (1 << I2C_CR1_STOP);
}

void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnaDi) {
	if (EnaDi == I2C_ACK_ENABLE) {
		//enable the ack
		pI2Cx->I2C_CR1 |= (1 << I2C_CR1_ACK);
	} else {
		////disable the ack
		pI2Cx->I2C_CR1 &= ~(1 << I2C_CR1_ACK);
	}

}

uint32_t RCC_GetPLLOutput(void) {
	//not implemented
	return 0;
}

uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName) {
	if (pI2Cx->I2C_SR1 & FlagName) {
		return FLAG_SET;
	}
	return FLAG_RESET;
}



/*********************************************************************
 * @fn      		  - I2C_SlaveEnableDisableCallbackEvents
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - this API is only used in SLAVE MODE ,
 *                      in order to enable the " Enable control bit of I2C interrupts"
 *                      then only the I2C EVENT and ERROR IRQ triggers

 */
void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx,uint8_t EnorDi)
{
	 if(EnorDi == ENABLE)
	 {
			pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITEVTEN);
			pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITBUFEN);
			pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITERREN);
	 }else
	 {
			pI2Cx->I2C_CR2 &= ~( 1 << I2C_CR2_ITEVTEN);
			pI2Cx->I2C_CR2 &= ~( 1 << I2C_CR2_ITBUFEN);
			pI2Cx->I2C_CR2 &= ~( 1 << I2C_CR2_ITERREN);
	 }

}

/*********************************************************************
 * @fn      		  - I2C_PeriClockControl
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
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnaDi) {
	if (EnaDi == ENABLE) {
		if (pI2Cx == I2C1) {
			I2C1_PCLK_EN();
		} else if (pI2Cx == I2C2) {
			I2C2_PCLK_EN();
		} else if (pI2Cx == I2C3) {
			I2C3_PCLK_EN();
		}

	} else {
		if (pI2Cx == I2C1) {
			I2C1_PCLK_DI();
		} else if (pI2Cx == I2C2) {
			I2C2_PCLK_DI();
		} else if (pI2Cx == I2C3) {
			I2C3_PCLK_DI();
		}
	}
}
/*********************************************************************
 * @fn      		  - I2C_Init
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
void I2C_Init(I2C_Handle_t *pI2CHandle) {
	uint32_t tempreg = 0; //this tempreg method can only used when register Reset value: 0x0000 otherwise it may alter the default mode setting
	//1.enable the clock for the i2cx peripheral
	I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);
	//2.first lets configure the CR1 register -ACk bit field
	tempreg |= pI2CHandle->I2C_Config.I2C_ACKControl << 10;
	pI2CHandle->pI2Cx->I2C_CR1 = tempreg;
	//3 2nd configure the CR2 register - FREQ bit field
	tempreg = 0;
	tempreg |= RCC_GetPCLK1Value() / 1000000U;  // if PCLK1 is 16MHz ,we need 16
	pI2CHandle->pI2Cx->I2C_CR2 = (tempreg & 0x3F);
	//4 3rd lets configure the I2C own address register (program the own device address)
	tempreg = 0;
	tempreg |= pI2CHandle->I2C_Config.I2C_DeviceAddress << 1; //it must be left shifted by 1 bcz  I2C_OAR1 register Bit 0 ADD0: Interface address7-bit addressing mode: don’t care
	tempreg |= (1 << 14); //Bit 14 Should always be kept at 1 by software.
	pI2CHandle->pI2Cx->I2C_OAR1 = tempreg;
	//5 4th lets configure the I2C clock control register (I2C_CCR)
	//CCR calculations
	uint16_t ccr_value = 0;
	tempreg = 0;
	if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM) {
		//mode :Standard MODE
		// if T_high = T_low ,then T_scl = 2*ccr*Tplck1 (refer RM)
		ccr_value = RCC_GetPCLK1Value()/ (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		tempreg |= (ccr_value & 0xFFF);
	} else {
		//mode :Fast MODE
		tempreg |= (1 << 15); //Bit 15 F/S: I2C controller mode selection ,0: Sm mode I2C ,1: Fm mode I2C
		tempreg |= pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14;
		if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_CYCLE_2) {
			ccr_value = RCC_GetPCLK1Value()/ (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		} else {
			ccr_value = RCC_GetPCLK1Value()/ (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		}
		tempreg |= (ccr_value & 0xFFF);
	}
	pI2CHandle->pI2Cx->I2C_CCR = tempreg;

	//6 lets configure the I2C TRISE register
	tempreg = 0;
	if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM) {
		//mode :Standard MODE
		//for eqn refer RM
		tempreg = (RCC_GetPCLK1Value() / 1000000UL) + 1; // in Sm mode, the maximum allowed SCL rise time is 1000ns = 1us = (1/1MHz)
	} else {
		//mode :Fast MODE
		tempreg = ((RCC_GetPCLK1Value() * 300) / 1000000000UL) + 1; //in Fm mode, the maximum allowed SCL rise time is 300ns
	}
	pI2CHandle->pI2Cx->I2C_TRISE = (tempreg & 0x3F);
}

/*********************************************************************
 * @fn      		  - I2C_DeInit
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
void I2C_DeInit(I2C_RegDef_t *pI2Cx) {
	if (pI2Cx == I2C1) {
		I2C1_REG_RESET();
	} else if (pI2Cx == I2C2) {
		I2C2_REG_RESET();
	} else if (pI2Cx == I2C3) {
		I2C3_REG_RESET();
	}
}

/*********************************************************************
 * @fn      		  - I2C_PeripheralControl
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
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnaDi) {
	if (EnaDi == ENABLE) {
		pI2Cx->I2C_CR1 |= (1 << I2C_CR1_PE);
	} else {
		pI2Cx->I2C_CR1 &= ~(1 << I2C_CR1_PE);
	}
}

/*
 * IRQ Configuration and Handling
 */
/*********************************************************************
 * @fn      		  -  I2C_MasterSendData
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

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxbuffer,
		uint32_t Len, uint8_t SlaveAddr, uint8_t Sr) {

	//1. Generate the START condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//2. confirm that start generation is completed by checking the SB flag in the SR1
	//   Note: Until SB is cleared SCL will be stretched (pulled to LOW)
	while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB))
		; //here we actually reading SR1 reg (this is the one among the steps to clearing the SB flag)

	//3. Send the address of the slave with r/nw bit set to w(0) (total 8 bits )
	I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

	//4. Confirm that address phase is completed by checking the ADDR flag in the SR1
	while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR)) {
		;
	}

	//5. clear the ADDR flag according to its software sequence
	//   Note: Until ADDR is cleared SCL will be stretched (pulled to LOW)
	I2C_ClearADDRFlag(pI2CHandle);

	//6. send the data until len becomes 0
	while (Len > 0) {
		while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
			; ////Wait till TXE is set
		pI2CHandle->pI2Cx->I2C_DR = *pTxbuffer;
		pTxbuffer++;
		Len--;
	}

	//7. when Len becomes zero wait for TXE=1 and BTF=1 before generating the STOP condition
	//   Note: TXE=1 , BTF=1 , means that both SR and DR are empty and next transmission should begin
	//   when BTF=1 SCL will be stretched (pulled to LOW)
	while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
		;
	while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF))
		;

	//8. Generate STOP condition and master need not to wait for the completion of stop condition.
	//   Note: generating STOP, automatically clears the BTF
	if (Sr == I2C_DISABLE_SR) {
		I2C_GenerateStoptCondition(pI2CHandle->pI2Cx);
	}

}

/*********************************************************************
 * @fn      		  - I2C_MasterReceiveData
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

void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxbuffer,
		uint32_t Len, uint8_t SlaveAddr, uint8_t Sr) {

	//1. Generate the START condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//2. confirm that start generation is completed by checking the SB flag in the SR1
	//   Note: Until SB is cleared SCL will be stretched (pulled to LOW)
	while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB))
		; //here we actually reading SR1 reg (this is the one among the steps to clearing the SB flag)

	//3. Send the address of the slave with r/nw bit set to w(0) (total 8 bits )
	I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

	//4. Confirm that address phase is completed by checking the ADDR flag in the SR1
	while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR)) {
		;
	}

	//procedure to read only 1 byte from the slave
	if (Len == 1) {
		//disable Acking
		I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);
		//clear the ADDR
		I2C_ClearADDRFlag(pI2CHandle);
		//Wait until RXNE = 1
		while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE))
			;
		//generate STOP condition
		if (Sr == I2C_DISABLE_SR) {
			I2C_GenerateStoptCondition(pI2CHandle->pI2Cx);
		}
		//read the data into the  buffer
		*pRxbuffer = pI2CHandle->pI2Cx->I2C_DR;
	}
	//procedure to read data from slave when Len > 1
	if (Len > 1) {
		//clear the ADDR
		I2C_ClearADDRFlag(pI2CHandle);
		//read the data until the len zero
		for (uint32_t i = Len; i > 0; i--) {
			//Wait until RXNE = 1
			while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE))
				;

			if (i == 2) { //if last 2 bytes are remaining
				//clear the ack bit
				I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);
				//generate the STOP condition
				if (Sr == I2C_DISABLE_SR) {
					I2C_GenerateStoptCondition(pI2CHandle->pI2Cx);
				}
			}

			//read the data from the data register into the buffer
			*pRxbuffer = pI2CHandle->pI2Cx->I2C_DR;
			//increment the buffer address
			pRxbuffer++;

		}
	}
//renable the ACKing to maintain the configuration setting otherwise after the API user ack control setting may alter
	if (pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE) {
		I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
	}

}

/*********************************************************************
 * @fn      		  - I2C_MasterSendDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Complete the below code . Also include the function prototype in header file

 */
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer,
		uint32_t Len, uint8_t SlaveAddr, uint8_t Sr) {

	uint8_t busystate = pI2CHandle->TxRxState;

	if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX)) {
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITERREN);

	}

	return busystate;

}

/*********************************************************************
 * @fn      		  - I2C_MasterReceiveDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the below code . Also include the fn prototype in header file

 */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer,
		uint32_t Len, uint8_t SlaveAddr, uint8_t Sr) {

	uint8_t busystate = pI2CHandle->TxRxState;

	if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX)) {
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = Len; //Rxsize is used in the ISR code to manage the data reception
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITERREN);

	}

	return busystate;
}

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle) {
	if (pI2CHandle->TxLen > 0) {
		//1. first load the data into the DR
		pI2CHandle->pI2Cx->I2C_DR = *(pI2CHandle->pTxBuffer);
		//2. decrement the Tx Length
		pI2CHandle->TxLen--;
		//3. increment the buffer address
		pI2CHandle->pTxBuffer++;

	}
}

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle) {
	//we have to do the data reception
	if (pI2CHandle->RxSize == 1) {
		*(pI2CHandle->pRxBuffer) = pI2CHandle->pI2Cx->I2C_DR;
		pI2CHandle->RxLen--;
	}
	if (pI2CHandle->RxSize > 1) {

		if (pI2CHandle->RxLen == 2) {
			//clear the ACK bit
			I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

		}

		//read DR
		*(pI2CHandle->pRxBuffer) = pI2CHandle->pI2Cx->I2C_DR;
		pI2CHandle->pRxBuffer++;
		pI2CHandle->RxLen--;

	}
	if (pI2CHandle->RxLen == 0) {
		//close the I2C data Reception and notify the application

		//1. Generate Stop Condition
		if (pI2CHandle->Sr == I2C_DISABLE_SR) { //generate stop condition only if repeated start is disabled
			I2C_GenerateStoptCondition(pI2CHandle->pI2Cx);
		}

		//2. reset all the member elements of the handle structure
		I2C_CloseReceiveData(pI2CHandle);

		//3. notify the application about the transmission complete
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMP);

	}
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle) {
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pRxBuffer = NULL;
	pI2CHandle->RxLen = 0;
	pI2CHandle->RxSize = 0;

	if (pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE) {
		I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
	}

}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle) {
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pTxBuffer = NULL;
	pI2CHandle->TxLen = 0;
}

/*********************************************************************
 * @fn      		  - I2C_SlaveSendData
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
void I2C_SlaveSendData(I2C_RegDef_t *pI2C, uint8_t data) {
	pI2C->I2C_DR = data;

}
/*********************************************************************
 * @fn      		  - I2C_SlaveReceiveData
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
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C) {

	return (uint8_t) pI2C->I2C_DR;

}

/*********************************************************************
 * @fn      		  - I2C_EV_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Interrupt handling for different I2C events (refer SR1)

 */

void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle) {

	//Interrupt handling for both master and slave mode of a device

	uint32_t temp1, temp2, temp3;

	//here check all the interrupt enabling bits are set or not
	temp1 = pI2CHandle->pI2Cx->I2C_CR2 & (1 << I2C_CR2_ITEVTEN);
	temp2 = pI2CHandle->pI2Cx->I2C_CR2 & (1 << I2C_CR2_ITBUFEN);

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_SB);

	//1. Handle For interrupt generated by SB event
	//	Note : SB flag is only applicable in Master mode
	if (temp1 && temp3) {
		//SB flag is set; this interrupt is generated because of SB event
		//This block will not executed in slave mode because for slave mode SB is always zero
		//in this block,lets execute the address phase
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX) {
			I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx,
					pI2CHandle->DevAddr);
		} else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
			I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}

	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_ADDR);
	//2. Handle For interrupt generated by ADDR event
	//Note : When master mode : Address is sent
	//		 When Slave mode   : Address matched with own address
	if (temp1 && temp3) {
		//ADDR  flag is set;the interrupt happens because of setting of ADDR flag
		I2C_ClearADDRFlag(pI2CHandle);

	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_BTF);
	//3. Handle For interrupt generated by BTF(Byte Transfer Finished) event
	if (temp1 && temp3) {
		//BTF  flag is set in both scenario - Tx and Rx
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX) {
			//make sure TXE is also set
			if (pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_TXE)) {
				//BTF,TXE = 1 (means both Data (DR) and Shift (SR)register are empty,which means close the Tx)
				if (pI2CHandle->TxLen == 0) {
					//1. Generate Stop Condition
					if (pI2CHandle->Sr == I2C_DISABLE_SR) { //generate stop condition only if repeated start is disabled
						I2C_GenerateStoptCondition(pI2CHandle->pI2Cx);
					}

					//2. reset all the member elements of the handle structure
					I2C_CloseSendData(pI2CHandle);

					//3. notify the application about the transmission complete
					I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMP);

				}
			}
		} else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
			;
		}

	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_STOPF);
	//4. Handle For interrupt generated by STOPF event
	// Note : Stop detection flag is applicable only slave mode . For master this flag will never be set
	if (temp1 && temp3) {
		//STOPF  flag is set
		//Clear the STOPF flag (ie 1)read SR1  2)write to CR1)  step 1 is already done in the above temp3 calculation
		//2nd step :write something that doesn't affect the value of CR1 :idea is "OR with 0"
		pI2CHandle->pI2Cx->I2C_CR1 |= 0x0000;

		//notify the application that STOP is detected
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);
	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_TXE);
	//5. Handle For interrupt generated by TXE event
	if (temp1 && temp2 && temp3) {
		//check for device mode - Master or Slave ,
		if (pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL)) {
			//TXE  flag is set
			//We have to do the Data Transmission
			if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX) {
				I2C_MasterHandleTXEInterrupt(pI2CHandle);
			}
		} else {
			//slave
			//make sure that the slave is really in Transmitting mode
			if (pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_TRA)) {
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
			}

		}
	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_RXNE);
	//6. Handle For interrupt generated by RXNE event
	if (temp1 && temp2 && temp3) {
		//RXNE  flag is set
		//check for device mode - Master or Slave ,
		if (pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL)) {
			if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
				I2C_MasterHandleRXNEInterrupt(pI2CHandle);
			}

		} else {
			//slave
			//make sure that the slave is really in Receiving mode
			if (!(pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_TRA))) {
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);

			}
		}
	}
}

	/*********************************************************************
	 * @fn      		  - I2C_ER_IRQHandling
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

	void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle) {

		uint32_t temp1, temp2;

		//Know the status of  ITERREN control bit in the CR2
		temp2 = (pI2CHandle->pI2Cx->I2C_CR2) & (1 << I2C_CR2_ITERREN);

		/***********************Check for Bus error************************************/
		temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & (1 << I2C_SR1_BERR);
		if (temp1 && temp2) {
			//This is Bus error

			//Implement the code to clear the buss error flag
			pI2CHandle->pI2Cx->I2C_SR1 &= ~(1 << I2C_SR1_BERR);

			//Implement the code to notify the application about the error
			I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_BERR);
		}

		/***********************Check for arbitration lost error************************************/
		temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & (1 << I2C_SR1_ARLO);
		if (temp1 && temp2) {
			//This is arbitration lost error

			//Implement the code to clear the arbitration lost error flag
			pI2CHandle->pI2Cx->I2C_SR1 &= ~(1 << I2C_SR1_ARLO);

			//Implement the code to notify the application about the error
			I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_ARLO);
		}

		/***********************Check for ACK failure  error************************************/

		temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & (1 << I2C_SR1_AF);
		if (temp1 && temp2) {
			//This is ACK failure error

			//Implement the code to clear the ACK failure error flag
			pI2CHandle->pI2Cx->I2C_SR1 &= ~(1 << I2C_SR1_AF);
			//Implement the code to notify the application about the error
			I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);
		}

		/***********************Check for Overrun/underrun error************************************/
		temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & (1 << I2C_SR1_OVR);
		if (temp1 && temp2) {
			//This is Overrun/underrun

			//Implement the code to clear the Overrun/underrun error flag
			pI2CHandle->pI2Cx->I2C_SR1 &= ~(1 << I2C_SR1_OVR);

			//Implement the code to notify the application about the error
			I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
		}

		/***********************Check for Time out error************************************/
		temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & (1 << I2C_SR1_TIMEOUT);
		if (temp1 && temp2) {
			//This is Time out error

			//Implement the code to clear the Time out error flag
			pI2CHandle->pI2Cx->I2C_SR1 &= ~(1 << I2C_SR1_TIMEOUT);

			//Implement the code to notify the application about the error
			I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_TIMEOUT);
		}

	}

	/*
	 * IRQ Configuration and Handling
	 */
	/*********************************************************************
	 * @fn      		  - I2C_IRQInterruptConfig
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
	void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi) {

		if (EnorDi == ENABLE) {
			if (IRQNumber <= 31) {
				//program the NVIC_ISER0
				*NVIC_ISER0 |= (1 << IRQNumber);

			} else if (IRQNumber > 31 && IRQNumber <= 63) {
				//program the NVIC_ISER1
				*NVIC_ISER1 |= (1 << (IRQNumber % 32));

			} else if (IRQNumber > 63 && IRQNumber <= 95) {
				////program the NVIC_ISER2
				*NVIC_ISER2 |= (1 << (IRQNumber % 32));
			}
		} else {
			if (IRQNumber <= 31) {
				//program the NVIC_ICER0
				*NVIC_ICER0 |= (1 << IRQNumber);

			} else if (IRQNumber > 31 && IRQNumber <= 63) {
				//program the NVIC_ICER1
				*NVIC_ICER1 |= (1 << (IRQNumber % 32));

			} else if (IRQNumber > 63 && IRQNumber <= 95) {
				////program the NVIC_ICER2
				*NVIC_ICER2 |= (1 << (IRQNumber % 32));
			}

		}

	}

	/*********************************************************************
	 * @fn      		  - I2C_IRQPriorityConfig
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

	void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority) {
		//1.lets find out the IPRx
		uint8_t iprx = (uint8_t) IRQNumber / 4;
		uint8_t iprx_section = (uint8_t) IRQNumber % 4;
		uint8_t shift_amount = (8 * iprx_section)
				+ (8 - NO_PR_BITS_IMPLEMENTED);
		NVIC_PR_BASE_ADDR[iprx] |= (IRQPriority << shift_amount);
		// equaltent to // *(NVIC_PR_BASE_ADDR + (iprx ) ) |= IRQPriority << shift_amount);
	}

	__weak void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv) {
		//AppEv:application event
		////This is a weak implementation . the user application may override this function.
	}


