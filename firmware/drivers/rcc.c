/*
 * rcc.c
 *
 *  Created on: Mar 2, 2023
 *      Author: martin
 */

#include "rcc.h"
#include "stm32h7xx.h"

#define DIVM1 3U
#define DIVN1 120U
#define DIVP1 1U
#define DIVQ1 2U
#define DIVR1 2U

#define HPRE  RCC_D1CFGR_HPRE_DIV2

#define DIVM2 3U
#define DIVN2 128U
#define DIVP2 2U
#define DIVQ2 2U
#define DIVR2 2U

#define DIVM3 3U
#define DIVN3 128U
#define DIVP3 2U
#define DIVQ3 2U
#define DIVR3 2U

void rcc_init(void)
{
	RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;

//	SCB_EnableDCache();
//	SCB_EnableICache();

	PWR->CR3 &= ~(PWR_CR3_BYPASS | PWR_CR3_SCUEN);
	PWR->CR3 |= PWR_CR3_LDOEN;
	while(!(PWR->CSR1 & PWR_CSR1_ACTVOSRDY));
	PWR->D3CR &= ~ PWR_D3CR_VOS;
	(void) (PWR->D3CR & PWR_D3CR_VOS);
	while(!(PWR->D3CR & PWR_D3CR_VOSRDY));
	while(!(PWR->CSR1 & PWR_CSR1_ACTVOSRDY));

	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_3WS | FLASH_ACR_WRHIGHFREQ;

	RCC->CR &= ~RCC_CR_PLL1ON;
	while((RCC->CR & RCC_CR_PLL1RDY));
	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY));
	RCC->CR |= RCC_CR_HSI48ON;
	while(!(RCC->CR & RCC_CR_HSI48ON));

	RCC->PLLCKSELR &= ~(        RCC_PLLCKSELR_DIVM1_Msk |          RCC_PLLCKSELR_DIVM2_Msk |          RCC_PLLCKSELR_DIVM3_Msk | RCC_PLLCKSELR_PLLSRC_Msk);
	RCC->PLLCKSELR |=  DIVM1 << RCC_PLLCKSELR_DIVM1_Pos | DIVM2 << RCC_PLLCKSELR_DIVM2_Pos | DIVM3 << RCC_PLLCKSELR_DIVM3_Pos | RCC_PLLCKSELR_PLLSRC_HSE;

	RCC->PLLCFGR   &= ~(0x01FF0FFFU);
	RCC->PLLCFGR   |=   RCC_PLLCFGR_PLL1RGE_0 | RCC_PLLCFGR_PLL2RGE_0 | RCC_PLLCFGR_PLL3RGE_0 | RCC_PLLCFGR_PLL2VCOSEL | 0x1FFU << 16;

	RCC->PLL1DIVR  &= ~(               RCC_PLL1DIVR_N1_Msk |                 RCC_PLL1DIVR_P1_Msk |                 RCC_PLL1DIVR_Q1_Msk |                 RCC_PLL1DIVR_R1_Msk);
	RCC->PLL1DIVR  |=  (DIVN1 - 1U) << RCC_PLL1DIVR_N1_Pos | (DIVP1 - 1U) << RCC_PLL1DIVR_P1_Pos | (DIVQ1 - 1U) << RCC_PLL1DIVR_Q1_Pos | (DIVR1 - 1U) << RCC_PLL1DIVR_R1_Pos;

	RCC->PLL2DIVR  &= ~(               RCC_PLL2DIVR_N2_Msk |                 RCC_PLL2DIVR_P2_Msk |                 RCC_PLL2DIVR_Q2_Msk |                 RCC_PLL2DIVR_R2_Msk);
	RCC->PLL2DIVR  |=  (DIVN2 - 1U) << RCC_PLL2DIVR_N2_Pos | (DIVP2 - 1U) << RCC_PLL2DIVR_P2_Pos | (DIVQ2 - 1U) << RCC_PLL2DIVR_Q2_Pos | (DIVR2 - 1U) << RCC_PLL2DIVR_R2_Pos;

	RCC->PLL3DIVR  &= ~(               RCC_PLL3DIVR_N3_Msk |                 RCC_PLL3DIVR_P3_Msk |                 RCC_PLL3DIVR_Q3_Msk |                 RCC_PLL3DIVR_R3_Msk);
	RCC->PLL3DIVR  |=  (DIVN3 - 1U) << RCC_PLL3DIVR_N3_Pos | (DIVP3 - 1U) << RCC_PLL3DIVR_P3_Pos | (DIVQ3 - 1U) << RCC_PLL3DIVR_Q3_Pos | (DIVR3 - 1U) << RCC_PLL3DIVR_R3_Pos;

	RCC->CR |= RCC_CR_PLL1ON;
	while(!(RCC->CR & RCC_CR_PLL1RDY));
	RCC->CR |= RCC_CR_PLL2ON;
	while(!(RCC->CR & RCC_CR_PLL2RDY));
	RCC->CR |= RCC_CR_PLL3ON;
	while(!(RCC->CR & RCC_CR_PLL3RDY));

	// HSI Divide by 4
	RCC->CR     &=       ~(RCC_CR_HSIDIV_Msk);
	RCC->CR     |=  (2U << RCC_CR_HSIDIV_Pos);

	// Divide by 2
	RCC->D1CFGR &=       ~(RCC_D1CFGR_D1PPRE_Msk   |          RCC_D1CFGR_HPRE_Msk);
	RCC->D1CFGR |=  (4U << RCC_D1CFGR_D1PPRE_Pos)  | (HPRE << RCC_D1CFGR_HPRE_Pos);

	// Divide by 2
	RCC->D2CFGR &=       ~RCC_D2CFGR_D2PPRE1_Msk;
	RCC->D2CFGR |=  4U << RCC_D2CFGR_D2PPRE1_Pos;

	// Divide by 2
	RCC->D2CFGR &=       ~RCC_D2CFGR_D2PPRE2_Msk;
	RCC->D2CFGR |=  4U << RCC_D2CFGR_D2PPRE2_Pos;

	// Divide by 2
	RCC->D3CFGR &=       ~RCC_D3CFGR_D3PPRE_Msk;
	RCC->D3CFGR |=  4U << RCC_D3CFGR_D3PPRE_Pos;

	// Select SPI123 clock source (apb1)
	RCC->D2CCIP1R &= ~(RCC_D2CCIP1R_SPI123SEL_Msk);
	RCC->D2CCIP1R |=   RCC_D2CCIP1R_SPI123SEL_2;

	// Select I2C1235 clock source (hsi), USB 48MHz HSI
	RCC->D2CCIP2R &= ~(RCC_D2CCIP2R_I2C1235SEL_Msk |       RCC_D2CCIP2R_USBSEL_Msk);
	RCC->D2CCIP2R |=   RCC_D2CCIP2R_I2C1235SEL_1   | 3U << RCC_D2CCIP2R_USBSEL_Pos;

	// Select ADC clock source (hsi)
	RCC->D3CCIPR  &= ~(RCC_D3CCIPR_ADCSEL_0);
	RCC->D3CCIPR  |=   RCC_D3CCIPR_ADCSEL_0;

	// Switch to PLL
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL1;
	while(((RCC->CFGR&RCC_CFGR_SWS)!=RCC_CFGR_SWS_PLL1));

	SystemCoreClockUpdate();
	(void)SystemCoreClock;
}

uint32_t rcc_d1cpre_div_get(void)
{
	switch(RCC_READ_BITS(RCC->D1CFGR, RCC_D1CFGR_D1CPRE)) {
		case 8:  return 2;
		case 9:  return 4;
		case 10: return 8;
		case 11: return 16;
		case 12: return 64;
		case 13: return 128;
		case 14: return 256;
		case 15: return 512;
	}
	return 1;
}

uint32_t rcc_hpre_div_get(void)
{
	switch(RCC_READ_BITS(RCC->D1CFGR, RCC_D1CFGR_HPRE)) {
		case 8:  return 2;
		case 9:  return 4;
		case 10: return 8;
		case 11: return 16;
		case 12: return 64;
		case 13: return 128;
		case 14: return 256;
		case 15: return 512;
	}
	return 1;
}

uint32_t rcc_d3ppre_div_get(void)
{
	switch(RCC_READ_BITS(RCC->D3CFGR, RCC_D3CFGR_D3PPRE)) {
		case 4: return 2;
		case 5: return 4;
		case 6: return 8;
		case 7: return 16;
	}
	return 1;
}

uint32_t rcc_d2ppre1_div_get(void)
{
	switch(RCC_READ_BITS(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1)) {
		case 4: return 2;
		case 5: return 4;
		case 6: return 8;
		case 7: return 16;
	}
	return 1;
}

uint32_t rcc_d2ppre2_div_get(void)
{
	switch(RCC_READ_BITS(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2)) {
		case 4: return 2;
		case 5: return 4;
		case 6: return 8;
		case 7: return 16;
	}
	return 1;
}

uint32_t rcc_ahbx_get_clock(void)
{
	return SystemCoreClock / rcc_d1cpre_div_get() / rcc_hpre_div_get();
}

uint32_t rcc_d2ppre1_timx_div_get(void)
{
	if(RCC->CFGR & RCC_CFGR_TIMPRE) {
		switch(RCC_READ_BITS(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1)) {
			case 4: return 1;
			case 5: return 1;
			case 6: return 2;
			case 7: return 4;
		}
		return 1;
	}
	switch(RCC_READ_BITS(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1)) {
		case 4: return 1;
		case 5: return 2;
		case 6: return 4;
		case 7: return 8;
	}
	return 1;
}

uint32_t rcc_d2ppre2_timx_div_get(void)
{
	if(RCC->CFGR & RCC_CFGR_TIMPRE) {
		switch(RCC_READ_BITS(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2)) {
			case 4: return 1;
			case 5: return 1;
			case 6: return 2;
			case 7: return 4;
		}
		return 1;
	}
	switch(RCC_READ_BITS(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2)) {
		case 4: return 1;
		case 5: return 2;
		case 6: return 4;
		case 7: return 8;
	}
	return 1;
}

uint32_t rcc_hsi_get_clock(void)
{
	return 64000000UL / (1UL << RCC_READ_BITS(RCC->CR, RCC_CR_HSIDIV));
}

uint32_t rcc_apb4_get_clock(void)
{
	return SystemCoreClock / rcc_d1cpre_div_get() / rcc_hpre_div_get() / rcc_d3ppre_div_get();
}

uint32_t rcc_apb1_get_clock(void)
{
	return SystemCoreClock / rcc_d1cpre_div_get() / rcc_hpre_div_get() / rcc_d2ppre1_div_get();
}

uint32_t rcc_apb2_get_clock(void)
{
	return SystemCoreClock / rcc_d1cpre_div_get() / rcc_hpre_div_get() / rcc_d2ppre2_div_get();
}

uint32_t rcc_apb1_timx_get_clock(void)
{
	return SystemCoreClock / rcc_d1cpre_div_get() / rcc_hpre_div_get() / rcc_d2ppre1_timx_div_get();
}

uint32_t rcc_apb2_timx_get_clock(void)
{
	return SystemCoreClock / rcc_d1cpre_div_get() / rcc_hpre_div_get() / rcc_d2ppre2_timx_div_get();
}
