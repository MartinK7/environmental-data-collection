
#include "stm32h7xx.h"

#include "FreeRTOS.h"
#include "task.h"

__STATIC_FORCEINLINE void error_callback(const char *FaultType, uint32_t PC, uint32_t R0, uint32_t R1, uint32_t R2, uint32_t LR, uint32_t SP)
{
	uint32_t HFSR  = SCB->HFSR;
	uint32_t CFSR  = SCB->CFSR;
	uint32_t BFAR  = SCB->BFAR;
	uint32_t MMFAR = SCB->MMFAR;
	uint32_t ABFSR = SCB->ABFSR;

	(void) HFSR;
	(void) CFSR;
	(void) BFAR;
	(void) BFAR;
	(void) MMFAR;
	(void) ABFSR;
	(void) PC;
	(void) R0;
	(void) R1;
	(void) R2;
}

#define FAULT_HANDLER(function)                       \
	uint32_t SP;	__asm("mov %0, sp" : "=r"(SP));   \
	uint32_t PC;	__asm("mov %0, pc" : "=r"(PC));   \
	uint32_t R0;	__asm("mov %0, r0" : "=r"(R0));   \
	uint32_t R1;	__asm("mov %0, r1" : "=r"(R1));   \
	uint32_t R2;	__asm("mov %0, r2" : "=r"(R2));   \
	uint32_t LR;	__asm("mov %0, lr" : "=r"(LR));   \
	error_callback(function, PC, R0, R1, R2, LR, SP); \
	NVIC_SystemReset();

void HardFault_Handler(void)
{
	FAULT_HANDLER(__FUNCTION__);
}

void BusFault_Handler(void)
{
	FAULT_HANDLER(__FUNCTION__);
}

void UsageFault_Handler(void)
{
	FAULT_HANDLER(__FUNCTION__);
}

void MemManage_Handler(void)
{
	FAULT_HANDLER(__FUNCTION__);
}

void WWDG_IRQHandler(void)
{
	FAULT_HANDLER(__FUNCTION__);
}

void ETH_WKUP_IRQHandler(void)
{
	FAULT_HANDLER(__FUNCTION__);
}

void WWDG1_IRQHandler(void)
{
	FAULT_HANDLER(__FUNCTION__);
}
