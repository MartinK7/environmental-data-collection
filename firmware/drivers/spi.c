
#include "spi.h"

#include <math.h>

#include "rcc.h"
#include "systick.h"

static int8_t spi_init_reset(SPI_TypeDef *spi, uint32_t *clock)
{
	__disable_irq();
	switch((uint32_t)spi) {
		case (uint32_t)SPI1:
			RCC->APB2ENR   |=   RCC_APB2ENR_SPI1EN;
			RCC->APB2RSTR  |=   RCC_APB2RSTR_SPI1RST;
			RCC->APB2RSTR  &= ~(RCC_APB2RSTR_SPI1RST);
			*clock = rcc_hsi_get_clock();
			break;
		case (uint32_t)SPI2:
			RCC->APB1LENR  |=   RCC_APB1LENR_SPI2EN;
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_SPI2RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_SPI2RST);
			*clock = rcc_hsi_get_clock();
			break;
		case (uint32_t)SPI3:
			RCC->APB1LENR  |=   RCC_APB1LENR_SPI3EN;
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_SPI3RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_SPI3RST);
			*clock = rcc_hsi_get_clock();
			break;
		case (uint32_t)SPI4:
			RCC->APB2ENR  |=   RCC_APB2ENR_SPI4EN;
			RCC->APB2RSTR |=   RCC_APB2RSTR_SPI4RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI4RST);
			*clock = rcc_apb2_get_clock();
			break;
		case (uint32_t)SPI5:
			RCC->APB2ENR  |=   RCC_APB2ENR_SPI5EN;
			RCC->APB2RSTR |=   RCC_APB2RSTR_SPI5RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI5RST);
			*clock = rcc_apb2_get_clock();
			break;
		case (uint32_t)SPI6:
			RCC->APB4ENR  |=   RCC_APB4ENR_SPI6EN;
			RCC->APB4RSTR |=   RCC_APB4RSTR_SPI6RST;
			RCC->APB4RSTR &= ~(RCC_APB4RSTR_SPI6RST);
			*clock = rcc_apb4_get_clock();
			break;
		default:
			__enable_irq();
			return -1;
	}
	__enable_irq();
	return 0;
}


int8_t spi_master_deinit(SPI_TypeDef *spi)
{
	__disable_irq();
	switch((uint32_t)spi) {
		case (uint32_t)SPI1:
			RCC->APB2RSTR  |=   RCC_APB2RSTR_SPI1RST;
			RCC->APB2RSTR  &= ~(RCC_APB2RSTR_SPI1RST);
			RCC->APB2ENR   &= ~(RCC_APB2ENR_SPI1EN);
			break;
		case (uint32_t)SPI2:
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_SPI2RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_SPI2RST);
			RCC->APB1LENR  &= ~(RCC_APB1LENR_SPI2EN);
			break;
		case (uint32_t)SPI3:
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_SPI3RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_SPI3RST);
			RCC->APB1LENR  &= ~(RCC_APB1LENR_SPI3EN);
			break;
		case (uint32_t)SPI4:
			RCC->APB2RSTR |=   RCC_APB2RSTR_SPI4RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI4RST);
			RCC->APB2ENR  &= ~(RCC_APB2ENR_SPI4EN);
			break;
		case (uint32_t)SPI5:
			RCC->APB2RSTR |=   RCC_APB2RSTR_SPI5RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI5RST);
			RCC->APB2ENR  &= ~(RCC_APB2ENR_SPI5EN);
			break;
		case (uint32_t)SPI6:
			RCC->APB4RSTR |=   RCC_APB4RSTR_SPI6RST;
			RCC->APB4RSTR &= ~(RCC_APB4RSTR_SPI6RST);
			RCC->APB4ENR  &= ~(RCC_APB4ENR_SPI6EN);
			break;
		default:
			__enable_irq();
			return -1;
	}
	__enable_irq();
	return 0;
}

int8_t spi_master_init(SPI_TypeDef *spi, uint32_t baudrate)
{
	uint32_t spi_pclock;
	uint32_t spi_clock;

	// Reset
	if(spi_init_reset(spi, &spi_pclock)) {
		// Unknown SPI peripheral
		return -1;
	}

	spi->CR1  |= SPI_CR1_SSI;

	const uint32_t presc_max = SPI_CFG1_MBR_Msk >> SPI_CFG1_MBR_Pos;
	uint32_t presc = spi_pclock / baudrate;
	presc = round(log2(presc));
	if(presc > presc_max) {
		presc = presc_max;
	}

	// Prescaler + data size 8 bits
	spi->CFG1 |= (presc << SPI_CFG1_MBR_Pos) | (7U << SPI_CFG1_DSIZE_Pos);

	// Baud rate check
	spi_clock = (spi->CFG1 & SPI_CFG1_MBR_Msk) >> SPI_CFG1_MBR_Pos;
	spi_clock = spi_pclock / (2U << spi_clock);
	if(spi_clock > baudrate) {
		return -5;
	}

	// Master mode
	spi->CFG2 |= SPI_CFG2_SSOM | SPI_CFG2_SSM | SPI_CFG2_MASTER;

	return 0;
}

int8_t spi_master_transaction(SPI_TypeDef *spi, const uint8_t *buffer_tx, uint16_t count_tx, uint8_t *buffer_rx, uint16_t count_rx)
{
	uint32_t timeout;
	uint8_t data_count = count_tx > count_rx ? count_tx : count_rx;

	__IO __attribute__((unused)) uint8_t sink;

	SPI1->CR2  &= ~(              SPI_CR2_TSIZE_Msk);
	SPI1->CR2  |=  (data_count << SPI_CR2_TSIZE_Pos);

	SPI1->CR1  |=  (SPI_CR1_SPE);
	SPI1->CR1  |=  (SPI_CR1_CSTART);

	timeout = systick_get();
	while(count_tx || count_rx) {
		if(spi->SR & SPI_SR_TXP) {
			if(count_tx) {
				*((__IO uint8_t*)&spi->TXDR) = *(buffer_tx++);
				count_tx--;
			} else {
				*((__IO uint8_t*)&spi->TXDR) = (uint8_t)0x5A;
			}
		}
		if(spi->SR & SPI_SR_RXP) {
			if(count_rx) {
				*(buffer_rx++) = *((__IO uint8_t*)&spi->RXDR);
				count_rx--;
			} else {
				sink = *((__IO uint8_t*)&spi->RXDR);
			}
		}
		if(systick_check_timeout(timeout, 100)) {
		    // Clear flags and disable SPI
		    SET_BIT(spi->IFCR, 0xFFFFFFFFU);
		    CLEAR_BIT(spi->CR1, SPI_CR1_SPE);
			return -1;
		}
	}

	timeout = systick_get();
	while(!(SPI1->SR & SPI_SR_EOT)) {
		if(systick_check_timeout(timeout, 100)) {
		    // Clear flags and disable SPI
		    SET_BIT(spi->IFCR, 0xFFFFFFFFU);
		    CLEAR_BIT(spi->CR1, SPI_CR1_SPE);
			return -2;
		}
	}

    // Clear flags and disable SPI
    SET_BIT(spi->IFCR, 0xFFFFFFFFU);
    CLEAR_BIT(spi->CR1, SPI_CR1_SPE);
	return 0;
}
