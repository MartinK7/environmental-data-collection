
#include "dcmi.h"
#include "rcc.h"
#include "gpio.h"//REMOVE
#include "systick.h"
#include "driver_os.h"

#include "stm32h7xx.h"

void DCMI_IRQHandler(void)
{
	DCMI->ICR |= DCMI_ICR_LINE_ISC | DCMI_ICR_VSYNC_ISC | DCMI_ICR_ERR_ISC | DCMI_ICR_OVR_ISC;
}

int8_t dcmi_init(void)
{
	uint32_t dcmi_pclock;

	// Reset
	__disable_irq();
	RCC->AHB2ENR  |=   RCC_AHB2ENR_DCMIEN;
	RCC->AHB2RSTR |=   RCC_AHB2RSTR_DCMIRST;
	RCC->AHB2RSTR &= ~(RCC_AHB2RSTR_DCMIRST);
	dcmi_pclock = rcc_ahbx_get_clock();
	__enable_irq();

	// Overclock check
	if(dcmi_pclock > 240000000) {
		return -1;
	}

	DCMI->CR  |= DCMI_CR_PCKPOL | DCMI_CR_JPEG | DCMI_CR_CM | DCMI_CR_EDM_0;
	DCMI->IER |= DCMI_IER_LINE_IE | DCMI_IER_VSYNC_IE | DCMI_IER_ERR_IE | DCMI_IER_OVR_IE;

	//NVIC_SetPriority(DCMI_PSSI_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	//NVIC_EnableIRQ(DCMI_PSSI_IRQn);

	return 0;
}

uint32_t dcmi_capture(uint32_t *buffer, uint32_t buffer_size_words)
{
	// Configure DMA
	// TODO move this in some universal library instead!
	__disable_irq();
	RCC->AHB1ENR      |= RCC_AHB1ENR_DMA2EN;
	DMAMUX1_Channel8->CCR |= 75U;
	DMA2_Stream0->CR  &= ~(0x01FFFFFFUL);
	DMA2_Stream0->CR  |= (3 << DMA_SxCR_PL_Pos);
	DMA2_Stream0->CR  |= (2 << DMA_SxCR_MSIZE_Pos);
	DMA2_Stream0->CR  |= (2 << DMA_SxCR_PSIZE_Pos);
	DMA2_Stream0->CR  |= (DMA_SxCR_MINC);
	DMA2_Stream0->FCR  = DMA_SxFCR_DMDIS;
	DMA2_Stream0->PAR  = (uint32_t)&DCMI->DR;
	DMA2_Stream0->M0AR = (uint32_t)buffer;
	DMA2_Stream0->NDTR = buffer_size_words;
	DMA2->LIFCR        = 0x3FU << ((0 - 0U) * 6);
	DMA2_Stream0->CR  |= DMA_SxCR_EN;
	__enable_irq();

	// Enable DCMI and start capture
	DCMI->CR |= DCMI_CR_CAPTURE;
	DCMI->CR |= DCMI_CR_ENABLE;

	uint32_t timeout = systick_get();
	while(DCMI->CR & DCMI_CR_CAPTURE || !(DCMI->RISR & DCMI_RIS_FRAME_RIS)) {
		if(systick_check_timeout(timeout, 1000)) {
			// Clean exit
			DCMI->CR &= ~(DCMI_CR_ENABLE | DCMI_CR_CAPTURE);
			DMA2_Stream0->CR &= ~DMA_SxCR_EN;
			return 0;
		}
		driver_os_yield();
	}

	// TODO Check capture
	//DCMI_RIS_FRAME_RIS

	// Clean exit
	DCMI->CR &= ~(DCMI_CR_ENABLE | DCMI_CR_CAPTURE);
	DMA2_Stream0->CR &= ~DMA_SxCR_EN;
	if(DMA2_Stream0->NDTR <= buffer_size_words && DMA2_Stream0->NDTR > 0)
		return buffer_size_words - DMA2_Stream0->NDTR;
	return 0;
}
