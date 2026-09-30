/*
 * stm32f446xx_rcc_driver.c
 *
 *  Created on: 29-Sept-2026
 *      Author: LENOVO
 */


#include "stm32f446xx.h"

uint16_t AHB_PreScaler[8] = {2,4,8,16,64,128,256,512};
uint8_t APB1_PreScaler[4] = { 2, 4 , 8, 16};



uint32_t RCC_GetPCLK1Value(void) { //read the path from source to pclk1 in clock tree
	uint32_t pclk1, SystemClk;
	uint8_t clksrc, temp, ahbp, apb1p;
	clksrc = ((RCC->CFGR >> 2) & 0x3);
	if (clksrc == 0) {
		//HSI oscillator (16MHz) used as the system clock
		SystemClk = 16000000UL;
	} else if (clksrc == 1) {
		//HSE oscillator used as the system clock
		//in our Nucleo board there is no HSE (if needed we can connect XTAL oscillator externally or can used clk source output of the ST LINK as HSE
	} else if (clksrc == 2) {
		// PLL used as the system clock
		SystemClk = RCC_GetPLLOutputClock();
	}

	temp = ((RCC->CFGR >> 4) & 0xF);//we need to find HPRE: AHB prescaler because the system clock come to this AHB prescalar
	if (temp < 8) {
		// system clock not divided
		ahbp = 1;
	} else {
		ahbp = AHB_PreScaler[temp - 8]; //here this array maps correct prescalar value
	}

	temp = ((RCC->CFGR >> 10) & 0x7); //we need to find  PPRE1:: APB1 prescaler because the output of AHB prescalar come to this APB1 prescalar
	if (temp < 4) {
		// system clock not divided
		apb1p = 1;
	} else {
		apb1p = APB1_PreScaler[temp - 4];
	}

	pclk1 =  (SystemClk / ahbp) /apb1p;
	return pclk1;
}

/*********************************************************************
 * @fn      		  - RCC_GetPCLK2Value
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
uint32_t RCC_GetPCLK2Value(void)
{
	uint32_t SystemClock=0,tmp,pclk2;
	uint8_t clk_src = ( RCC->CFGR >> 2) & 0X3;

	uint8_t ahbp,apb2p;

	if(clk_src == 0)
	{
		SystemClock = 16000000;
	}else
	{
		SystemClock = 8000000;
	}
	tmp = (RCC->CFGR >> 4 ) & 0xF;

	if(tmp < 0x08)
	{
		ahbp = 1;
	}else
	{
       ahbp = AHB_PreScaler[tmp-8];
	}

	tmp = (RCC->CFGR >> 13 ) & 0x7;
	if(tmp < 0x04)
	{
		apb2p = 1;
	}else
	{
		apb2p = APB1_PreScaler[tmp-4];
	}

	pclk2 = (SystemClock / ahbp )/ apb2p;

	return pclk2;
}

uint32_t  RCC_GetPLLOutputClock()
{

	return 0;
}
