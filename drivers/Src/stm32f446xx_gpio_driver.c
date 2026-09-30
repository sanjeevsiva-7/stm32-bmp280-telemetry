/*
 * stm32f446xx_gpio_driver.c
 *
 *  Created on: 12-Sept-2026
 *      Author: SANJEEV
 */


#include "stm32f446xx_gpio_driver.h"



/*
 * Peripheral Clock Setup
 */
/***********************************************************************************************
 * @fn							-GPIO_PeriClockControl
 *
 * @brief						-The Function Enable or Disable the Peripheral clock for the given GPIO Port
 *
 * @param[in]					-The Base Address of the GPIO Peripheral
 * @param[in]					-ENABLE or DISABLE macros
 *
 * @return						-none
 *
 * note							-none
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx ,uint8_t EnorDi){
	if(EnorDi == ENABLE){
		if(pGPIOx == GPIOA){
			GPIOA_PCLK_EN();
		}
		else if(pGPIOx == GPIOB){
			GPIOB_PCLK_EN();
		}
		else if(pGPIOx == GPIOC){
					GPIOC_PCLK_EN();
		}
		else if(pGPIOx == GPIOD){
					GPIOD_PCLK_EN();
		}
		else if(pGPIOx == GPIOE){
					GPIOE_PCLK_EN();
		}
		else if(pGPIOx == GPIOF){
					GPIOF_PCLK_EN();
				}
		else if(pGPIOx == GPIOG){
					GPIOG_PCLK_EN();
		}
		else if(pGPIOx == GPIOH){
					GPIOH_PCLK_EN();
	    }

	}
	else{
		if(pGPIOx == GPIOA){
			GPIOA_PCLK_DI();
		}
		else if(pGPIOx == GPIOB){
			GPIOB_PCLK_DI();
		}
		else if(pGPIOx == GPIOC){
					GPIOC_PCLK_DI();
		}
		else if(pGPIOx == GPIOD){
					GPIOD_PCLK_DI();
		}
		else if(pGPIOx == GPIOE){
					GPIOE_PCLK_DI();
		}
		else if(pGPIOx == GPIOF){
					GPIOF_PCLK_DI();
				}
		else if(pGPIOx == GPIOG){
					GPIOG_PCLK_DI();
		}
		else if(pGPIOx == GPIOH){
					GPIOH_PCLK_DI();
	    }

	}
}

/*
 * init and De-init
 */

/*********************************************************************
 * @fn      		  - GPIO_Init
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 * M
 * @Note              -

 */
void GPIO_Init(GPIO_Handler_t *pGPIOHandler){
	//enable the peripheral clock((some time user forget to enable the Pclk so we can integrate that API inside this Init API)
	GPIO_PeriClockControl(pGPIOHandler->pGPIOx, ENABLE);
    uint32_t temp = 0;
	//1.Configure the mode of the GPIO pin
	if(pGPIOHandler->GPIO_PinConf.GPIO_PinMode <= GPIO_MODE_ANALOG){
		//non-interrupt modes
		temp = pGPIOHandler->GPIO_PinConf.GPIO_PinMode << (2 * pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
		pGPIOHandler->pGPIOx->MODER &= ~(0x3 << pGPIOHandler->GPIO_PinConf.GPIO_PinNumber); //safety:clearing before setting the bit
		pGPIOHandler->pGPIOx->MODER |= temp;//setting
	}else{
		//this part will code later (the interrupt mode)
	    // interrupt mode — pin is ALWAYS an input, regardless of prior state
		//All GPIO pins are in INPUT MODE by default but for safety we are setting it again
	    pGPIOHandler->pGPIOx->MODER &= ~(0x3 << (2 * pGPIOHandler->GPIO_PinConf.GPIO_PinNumber));
		if(pGPIOHandler->GPIO_PinConf.GPIO_PinMode == GPIO_MODE_IT_FT){
			//1. Configure the Fall Edge Trigger selection register (FTSR)
			EXTI->EXTI_FTSR |=(1 <<pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
			//2. Safety:clear the corresponding RTSR bit
			EXTI->EXTI_RTSR &= ~(1 <<pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
		}else if(pGPIOHandler->GPIO_PinConf.GPIO_PinMode == GPIO_MODE_IT_RT){
			//1. Configure the Rise Edge Trigger selection register (RTSR)
			EXTI->EXTI_RTSR |=(1 <<pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
			//2. Safety:clear the corresponding FTSR bit
			EXTI->EXTI_FTSR &= ~(1 <<pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);


		}else if(pGPIOHandler->GPIO_PinConf.GPIO_PinMode == GPIO_MODE_IT_RFT){
			//1.Configure both RTSR and FTSR
			EXTI->EXTI_FTSR |=(1 <<pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
			EXTI->EXTI_RTSR |=(1 <<pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);


		}
		//2. configure the GPIO PORT Selection SYSCFG_EXTCR
		uint8_t temp1,temp2;
		temp1 = pGPIOHandler->GPIO_PinConf.GPIO_PinNumber/4; //decide which CFGRx
		temp2 = pGPIOHandler->GPIO_PinConf.GPIO_PinNumber%4; //decide the bit field
		uint8_t port_code = (uint8_t)GPIO_BASEADDR_TO_CODE(pGPIOHandler->pGPIOx);
		SYSCFG_PCLK_EN(); // enable PCLK for SYSCFG
		SYSCFG->SYSCFG_EXTICR[temp1] |= (port_code << (4 *temp2));




		//3.enable the EXTI interrupt delivery using IMR
		EXTI->EXTI_IMR |=(1 << pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);

	}

	//2.configure the GPIO speed
	temp=0;
	temp = pGPIOHandler->GPIO_PinConf.GPIO_PinSpeed << (2 * pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->OSPEEDER &= ~(0x3 << pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->OSPEEDER |= temp;

	//3. configure the PULL UP/DOWN Setting
	temp = 0;
	temp = pGPIOHandler->GPIO_PinConf.GPIO_PinPuPdControl << (2 * pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->PUPDR &= ~(0x3 << pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->PUPDR |= temp;

	//4.Configure the GPIO output type
	temp = 0;
	temp = pGPIOHandler->GPIO_PinConf.GPIO_PinOPType << (pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->OTYPER &= ~(1 <<pGPIOHandler->GPIO_PinConf.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->OTYPER |= temp;
	//TODO 5.configure the GPIO alternate functionality
	if(pGPIOHandler->GPIO_PinConf.GPIO_PinMode == GPIO_MODE_ALTFN){
	temp = 0;
	uint8_t temp1 = pGPIOHandler->GPIO_PinConf.GPIO_PinNumber / 8; //decide AFH or AFL reg based on the pin number
	uint8_t temp2 = pGPIOHandler->GPIO_PinConf.GPIO_PinNumber % 8; //compute the desired bit field
	temp = pGPIOHandler->GPIO_PinConf.GPIO_PinAltFunMode << (4 * temp2);
	pGPIOHandler->pGPIOx->AFR[temp1] &= ~(0xF << (4 * temp2));
	pGPIOHandler->pGPIOx->AFR[temp1] |= temp;


	}


}

/*********************************************************************
 * @fn      		  - GPIO_DeInit
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 * M
 * @Note              -
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx){
	if(pGPIOx == GPIOA){
				GPIOA_REG_RESET();
			}
			else if(pGPIOx == GPIOB){
				GPIOB_REG_RESET();
			}
			else if(pGPIOx == GPIOC){
						GPIOC_REG_RESET();
			}
			else if(pGPIOx == GPIOD){
						GPIOD_REG_RESET();
			}
			else if(pGPIOx == GPIOE){
						GPIOE_REG_RESET();
			}
			else if(pGPIOx == GPIOF){
						GPIOF_REG_RESET();
					}
			else if(pGPIOx == GPIOG){
						GPIOG_REG_RESET();
			}
			else if(pGPIOx == GPIOH){
						GPIOH_REG_RESET();
		    }

		}




/*
 * Data Read and Write
 */
/*********************************************************************
 * @fn      		  - GPIO_ReadFromInputPin
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -   0 or 1
 *
 * @Note              -

 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber){
	uint8_t value;
	value = (uint8_t)((pGPIOx->IDR >> PinNumber) &(0x1));
	return value;

}
/*********************************************************************
 * @fn      		  - GPIO_ReadFromInputPort
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
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx){
	uint16_t value;
	value = (uint16_t)pGPIOx->IDR;
	return value;
}
/*********************************************************************
 * @fn      		  - GPIO_WriteToOutputPin
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
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber,uint8_t value){
	if (value == GPIO_PIN_SET){
		//write 1 to the output data register at the bit field corresponding to the pin number
	pGPIOx->ODR |= (1 << PinNumber);
}
	else{
		//write 0
		pGPIOx->ODR &= ~(1 << PinNumber);
}
}


/*********************************************************************
 * @fn      		  - GPIO_WriteToOutputPort
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
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx,uint16_t value){
	pGPIOx->ODR = value;
}
/*********************************************************************
 * @fn      		  - GPIO_ToggleOutputPin
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
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber){
	pGPIOx->ODR ^= (1 << PinNumber);
}

/*
 * IRQ Configuration and Handling
 */
/*********************************************************************
 * @fn      		  - GPIO_IRQInterruptConfig
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
void GPIO_IRQInterruptConfig(uint8_t IRQNumber,uint8_t EnorDi){

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
 * @fn      		  - GPIO_IRQPriorityConfig
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

void GPIO_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority){
	//1.lets find out the IPRx
	uint8_t iprx =  (uint8_t)IRQNumber / 4;
	uint8_t iprx_section =  (uint8_t)IRQNumber % 4;
	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
	NVIC_PR_BASE_ADDR[iprx] |= (IRQPriority << shift_amount);
	// equaltent to // *(NVIC_PR_BASE_ADDR + (iprx ) ) |= IRQPriority << shift_amount);
	}

void GPIO_IRQHandling(uint8_t PinNumber){
	//1 clear (manually done by programmer not like in the case of NVIC_PR)  the EXTI_PR reg corresponding to the pin number
	if(EXTI->EXTI_PR &= (1 << PinNumber)){  //check it is really pended
	EXTI->EXTI_PR &= ~(1 << PinNumber); //as per the refer manual ,This bit is cleared by programming it to 1.
	}

}
