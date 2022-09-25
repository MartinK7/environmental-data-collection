
#include "i2c.h"
#include "rcc.h"
#include "systick.h"
#include "driver_os.h"

static int8_t i2c_init_reset(I2C_TypeDef *i2c, uint32_t *clock)
{
	__disable_irq();

	uint32_t src = RCC_READ_BITS(RCC->D2CCIP2R, RCC_D2CCIP2R_I2C1235SEL);

	switch(src) {
	case 0: // rcc_pclk1
		switch((uint32_t)i2c) {
				case (uint32_t)I2C1:
					*clock = rcc_apb1_get_clock();
					break;
				case (uint32_t)I2C2:
					*clock = rcc_apb1_get_clock();
					break;
				case (uint32_t)I2C3:
					*clock = rcc_apb1_get_clock();
					break;
				case (uint32_t)I2C4:
					*clock = rcc_apb4_get_clock();
					break;
				case (uint32_t)I2C5:
					*clock = rcc_apb1_get_clock();
					break;
				default:
					__enable_irq();
					return -1;
			}
		break;
	case 1: // pll3_r_ck
		// TODO Not implemented
		__enable_irq();
		return -1;
	case 2: // hsi_ker_ck
		*clock = rcc_hsi_get_clock();
		break;
	case 3: // csi_ker_ck
		// TODO Not implemented
		__enable_irq();
		return -1;
	}

	switch((uint32_t)i2c) {
		case (uint32_t)I2C1:
			RCC->APB1LENR  |=   RCC_APB1LENR_I2C1EN;
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_I2C1RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_I2C1RST);
			break;
		case (uint32_t)I2C2:
			RCC->APB1LENR  |=   RCC_APB1LENR_I2C2EN;
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_I2C2RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_I2C2RST);
			break;
		case (uint32_t)I2C3:
			RCC->APB1LENR  |=   RCC_APB1LENR_I2C3EN;
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_I2C3RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_I2C3RST);
			break;
		case (uint32_t)I2C4:
			RCC->APB4ENR  |=   RCC_APB4ENR_I2C4EN;
			RCC->APB4RSTR |=   RCC_APB4RSTR_I2C4RST;
			RCC->APB4RSTR &= ~(RCC_APB4RSTR_I2C4RST);
			break;
		case (uint32_t)I2C5:
			RCC->APB1LENR  |=   RCC_APB1LENR_I2C5EN;
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_I2C5RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_I2C5RST);
			break;
		default:
			__enable_irq();
			return -1;
	}

	__enable_irq();
	return 0;
}

int8_t i2c_init_master(I2C_TypeDef *i2c, enum i2c_timing timing)
{
	uint32_t i2c_pclock;

	// Note: The i2c_ker_ck period tI2CCLK must respect the following conditions:
	//       t_I2CCLK < (t_LOW - t_filters) / 4 and t_I2CCLK < t_HIGH
	//
	// Analog filter delay is maximum 260 ns. Digital filter delay is DNF x t_I2CCLK.
	// The i2c_pclk clock period tPCLK must respect the following condition:
	// t_PCLK < 4/3 t_SCL
	
	if(timing > I2C_TIMING_FAST_1000KHZ) {
		return -1;
	}

	// Reset
	if(i2c_init_reset(i2c, &i2c_pclock)) {
		// Unknown I2C peripheral
		return -2;
	}

	// Noise filters
	//I2C_CR1_ANFOFF, I2C_CR1_DNF

	// I2C timings
	//SDADEL[3:0], SCLDEL[3:0], SCLH[7:0], SCLL[7:0] in I2C_TIMINGR
	// Consequently the master clock period is:
	// t_SCL = t_SYNC1 + t_SYNC2 + {[(SCLH+1) + (SCLL+1)] x (PRESC+1) x t_I2CCLK}

	static const uint8_t timings[3][4][5] = {
		// {{PRESC, SCLL, SCLH, SDADEL, SCLDEL}, {...}, {...}},
		// I2C_TIMING_STANDARD_10KHZ  I2C_TIMING_STANDARD_100KHZ  I2C_TIMING_FAST_400KHZ   I2C_TIMING_FAST_1000KHZ
		{{  1, 0xC7, 0xC3, 0x2, 0x4}, {  1, 0x13, 0xF, 0x2, 0x4}, {0, 0x9, 0x3, 0x1, 0x3}, {0, 0x6, 0x3, 0x0, 0x1}}, // 8MHz
		{{  3, 0xC7, 0xC3, 0x2, 0x4}, {  3, 0x13, 0xF, 0x2, 0x4}, {1, 0x9, 0x3, 0x2, 0x3}, {0, 0x4, 0x2, 0x0, 0x2}}, // 16MHz
		{{0xB, 0xC7, 0xC3, 0x2, 0x4}, {0xB, 0x13, 0xF, 0x2, 0x4}, {5, 0x9, 0x3, 0x3, 0x3}, {5, 0x3, 0x1, 0x0, 0x1}}  // 48MHz
	};
	
	const uint8_t *timing_ptr;
	
	switch(i2c_pclock) {
		case 8000000:
			timing_ptr = &timings[0][timing][0];
			break;
		case 16000000:
			timing_ptr = &timings[1][timing][0];
			break;
		case 48000000:
			timing_ptr = &timings[2][timing][0];
			break;
		default:
			// This driver can only work with 8,16,48 MHz pclk
			return -3;
	}
	
	i2c->TIMINGR = (timing_ptr[0] << I2C_TIMINGR_PRESC_Pos ) | \
	               (timing_ptr[1] << I2C_TIMINGR_SCLL_Pos  ) | \
	               (timing_ptr[2] << I2C_TIMINGR_SCLH_Pos  ) | \
	               (timing_ptr[3] << I2C_TIMINGR_SDADEL_Pos) | \
	               (timing_ptr[4] << I2C_TIMINGR_SCLDEL_Pos);

	// Enable automatically sending STOP
	i2c->CR2 |= I2C_CR2_AUTOEND;

	i2c->OAR1 |= I2C_OAR1_OA1EN;

	// Enable I2C
	i2c->CR1 |= I2C_CR1_PE;

	// Ready
	return 0;
}

int8_t i2c_master_transaction(I2C_TypeDef *i2c, uint8_t address_7bit, enum i2c_direction direction, uint8_t *buffer, uint8_t count)
{
	uint32_t timeout;

	if(count == 0) {
		// Zero byte count not supported
		return -1;
	}

	timeout = systick_get();
	while(i2c->ISR & I2C_ISR_BUSY) {
		if(systick_check_timeout(timeout, 50)) {
			// Error busy
			return -2;
		}
		driver_os_yield();
	}

	// Addressing mode (7bit)
	i2c->CR2 &= ~I2C_CR2_ADD10;

	// Slave address to be sent
	i2c->CR2 &= ~I2C_CR2_SADD_Msk;
	i2c->CR2 |= (address_7bit & 0x7FU) << (I2C_CR2_SADD_Pos + 1U);

	// Enable automatically sending STOP
	i2c->CR2 |= I2C_CR2_AUTOEND;

	// Transfer direction
	if(direction == I2C_DIRECTION_WRITE) {
		i2c->CR2 &= ~I2C_CR2_RD_WRN;
	} else if(direction == I2C_DIRECTION_READ) {
		i2c->CR2 |= I2C_CR2_RD_WRN;
	} else {
		return -3;
	}

	// The number of bytes to be transferred
	i2c->CR2 &= ~I2C_CR2_NBYTES_Msk;
	i2c->CR2 |= (count & 0xFFU) << I2C_CR2_NBYTES_Pos;

	// Clear NACK flag
	i2c->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;

	// Set the START bit in I2C_CR2 register
	i2c->CR2 |= I2C_CR2_START;

	if(direction == I2C_DIRECTION_READ) {
		/// Read procedure
		while(count--) {
			// Wait for RXNE
			timeout = systick_get();
			while(!(i2c->ISR & I2C_ISR_RXNE)) {
				if(systick_check_timeout(timeout, 50)) {
					i2c->CR1 &= ~I2C_CR1_PE;
					i2c->CR1 |=  I2C_CR1_PE;
					return -22;
				}
				driver_os_yield();
			}

			// Read byte
			*(buffer++) = i2c->RXDR;
		}

		// Wait
		timeout = systick_get();
		while(!(i2c->ISR & I2C_ISR_STOPF)) {
			if(systick_check_timeout(timeout, 50)) {
				i2c->CR1 &= ~I2C_CR1_PE;
				i2c->CR1 |=  I2C_CR1_PE;
				return -23;
			}
			driver_os_yield();
		}
		return 0;
	}

	/// Write procedure
	while(count--) {
		// Wait for TXIS
		timeout = systick_get();
		while(!(i2c->ISR & I2C_ISR_TXIS)) {
			if(systick_check_timeout(timeout, 50)) {
				i2c->CR1 &= ~I2C_CR1_PE;
				i2c->CR1 |=  I2C_CR1_PE;
				return -12;
			}
			driver_os_yield();
		}

		// Write byte
		i2c->TXDR = *(buffer++);
	}
	// Wait
	timeout = systick_get();
	while(!(i2c->ISR & I2C_ISR_STOPF)) {
		if(systick_check_timeout(timeout, 50)) {
			i2c->CR1 &= ~I2C_CR1_PE;
			i2c->CR1 |=  I2C_CR1_PE;
			return -23;
		}
		driver_os_yield();
	}
	return 0;
}
