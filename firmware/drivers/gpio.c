
#include "gpio.h"

static struct {
	struct gpio_pin pin;
	void(*callback)(struct gpio_pin pin);
} callbacks[16] = {0};

// ISRs

static void gpio_exti_isr(void)
{
	uint32_t pin;
	uint32_t pr1 = EXTI->PR1 & 0xFU;

	for(pin = 0; pin < 16; ++pin) {
		// Interrupt flag?
		if(!(pr1 & (1U << pin))) {
			// Nope
			continue;
		}

		// Callback?
		if(callbacks[pin].callback && callbacks[pin].pin.gpio) {
			callbacks[pin].callback(callbacks[pin].pin);
		}

		// Clear flag
		EXTI->PR1 = (1U << pin);
	}
}

void EXTI0_IRQHandler(void)
{
	gpio_exti_isr();
}

void EXTI1_IRQHandler(void)
{
	gpio_exti_isr();
}

void EXTI2_IRQHandler(void)
{
	gpio_exti_isr();
}

void EXTI3_IRQHandler(void)
{
	gpio_exti_isr();
}

void EXTI4_IRQHandler(void)
{
	gpio_exti_isr();
}

void EXTI9_5_IRQHandler(void)
{
	gpio_exti_isr();
}

void EXTI15_10_IRQHandler(void)
{
	gpio_exti_isr();
}

// Global functions

void gpio_init(void)
{
	RCC->AHB4ENR |= \
		RCC_AHB4ENR_GPIOAEN | \
		RCC_AHB4ENR_GPIOBEN | \
		RCC_AHB4ENR_GPIOCEN | \
		RCC_AHB4ENR_GPIODEN | \
		RCC_AHB4ENR_GPIOEEN | \
		RCC_AHB4ENR_GPIOFEN | \
		RCC_AHB4ENR_GPIOGEN | \
		RCC_AHB4ENR_GPIOHEN | \
		RCC_AHB4ENR_GPIOJEN | \
		RCC_AHB4ENR_GPIOKEN;

	// LED
	gpio_pin_configure(GPIO_PIN_USER_LED,         GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);

	// Camera
	gpio_pin_configure(GPIO_PIN_DCMI_D0,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D1,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D2,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D3,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D4,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D5,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D6,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D7,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D8,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_D9,          GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_PIXCL,       GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_HSYNC,       GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_DCMI_VSYNC,       GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO, 13, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_I2C1_SDA,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  4, GPIO_PIN_STATE_HIGH);
	gpio_pin_configure(GPIO_PIN_I2C1_SCL,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  4, GPIO_PIN_STATE_HIGH);
	gpio_pin_configure(GPIO_PIN_CAM_XCLK,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_MEDIUM   , GPIO_PIN_PUPD_NO,  2, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CAM_PWDN,         GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CAM_RESET,        GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CAM_PWR_EN,       GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);

	// Power management pins
	gpio_pin_configure(GPIO_PIN_WIFI_EN,          GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_ETH_EN,           GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_I2C4_3V3_EN,  GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART5_5V_EN,  GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_I2C2_5V_EN,   GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_I2C2_3V3_EN,  GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);

	// Sensors
	gpio_pin_configure(GPIO_PIN_I2C4_SDA,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  4, GPIO_PIN_STATE_HIGH);
	gpio_pin_configure(GPIO_PIN_I2C4_SCL,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  4, GPIO_PIN_STATE_HIGH);
	gpio_pin_configure(GPIO_PIN_I2C2_SDA,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  4, GPIO_PIN_STATE_HIGH);
	gpio_pin_configure(GPIO_PIN_I2C2_SCL,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN, GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  4, GPIO_PIN_STATE_HIGH);

	// Mechanical sensors
	gpio_pin_configure(GPIO_PIN_ANEMOMETER,       GPIO_PIN_MODE_INPUT,     GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_GAUGE,            GPIO_PIN_MODE_INPUT,     GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_ADC2_WIND_DIR,    GPIO_PIN_MODE_ANALOG,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);

	// PMS sensor
	gpio_pin_configure(GPIO_PIN_CON_PMS_RESET,    GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_PMS_SET,      GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART5_RTS,    GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART5_CTS,    GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART5_RX,     GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART5_TX,     GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);

	// Analog feedback
	gpio_pin_configure(GPIO_PIN_ADC3_BAT_VOLTAGE, GPIO_PIN_MODE_ANALOG,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);

	// GSM/LTE
	gpio_pin_configure(GPIO_PIN_UART4_RTS,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_UART4_CTS,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_UART4_RX,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_UART4_TX,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  8, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_LTE_PWRKEY,       GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_LTE_RESET,        GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  0, GPIO_PIN_STATE_LOW);

	// External serial device
	gpio_pin_configure(GPIO_PIN_CON_UART7_RTS,    GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  7, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART7_CTS,    GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  7, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART7_RX,     GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  7, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_CON_UART7_TX,     GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  7, GPIO_PIN_STATE_LOW);

	// WiFi
	gpio_pin_configure(GPIO_PIN_ESP32_MISO,       GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  5, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_ESP32_MOSI,       GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  5, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_ESP32_SCK,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  5, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_ESP32_CS,         GPIO_PIN_MODE_OUTPUT,    GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_LOW,       GPIO_PIN_PUPD_NO,  5, GPIO_PIN_STATE_LOW);

	// Ethernet
	gpio_pin_configure(GPIO_PIN_RMII_REFCLK,      GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_MDIO,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_MDC,         GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_RXDATAVALID, GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_RXD0,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_RXD1,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_TXEN,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_TXD0,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
	gpio_pin_configure(GPIO_PIN_RMII_TXD1,        GPIO_PIN_MODE_ALTERNATE, GPIO_PIN_OUTPUT_TYPE_PUSH_PULL,  GPIO_PIN_OUTPUT_SPEED_VERY_HIGH,      GPIO_PIN_PUPD_NO, 11, GPIO_PIN_STATE_LOW);
}

void gpio_pin_configure(struct gpio_pin pin, enum gpio_pin_mode mode, enum gpio_pin_output_type output_type, enum gpio_pin_output_speed output_speed, enum gpio_pin_pupd pupd, uint8_t alternate_function, enum gpio_pin_state state)
{
	// Lock this interface, next registers manipulation must be atomic
	__disable_irq();

	// (Switch off) set as analog for safety
	pin.gpio->MODER   &= ~((                 3U) << pin.pin * 2U);

	// This function must not change irq state
	gpio_pin_write(pin, state);

	if(pin.pin < 8U) {
		pin.gpio->AFR[0U] &= ~((                              15U) << (pin.pin - 0U) * 4U);
		pin.gpio->AFR[0U] |=   ((uint8_t)alternate_function & 15U) << (pin.pin - 0U) * 4U;
	} else if(pin.pin < 16U) {
		pin.gpio->AFR[1U] &= ~((                              15U) << (pin.pin - 8U) * 4U);
		pin.gpio->AFR[1U] |=   ((uint8_t)alternate_function & 15U) << (pin.pin - 8U) * 4U;
	} else {
		// Error, unlock
		__enable_irq();
		return;
	}

	pin.gpio->OTYPER  &= ~((                        1U) << pin.pin * 1U);
	pin.gpio->OTYPER  |=   ((uint32_t)output_type & 1U) << pin.pin * 1U;

	pin.gpio->OSPEEDR &= ~((                         3U) << pin.pin * 2U);
	pin.gpio->OSPEEDR |=   ((uint32_t)output_speed & 3U) << pin.pin * 2U;

	pin.gpio->PUPDR   &= ~((                 1U) << pin.pin * 1U);
	pin.gpio->PUPDR   |=   ((uint32_t)pupd & 1U) << pin.pin * 1U;

	pin.gpio->MODER   &= ~((                 3U) << pin.pin * 2U);
	pin.gpio->MODER   |=   ((uint32_t)mode & 3U) << pin.pin * 2U;

	// Unlock
	__enable_irq();
}

void gpio_pin_write(struct gpio_pin pin, enum gpio_pin_state state)
{
	pin.gpio->BSRR = state == GPIO_PIN_STATE_HIGH ? (1U << pin.pin) : (1U << (pin.pin + 16U));
}

enum gpio_pin_state gpio_pin_read(struct gpio_pin pin)
{
	return pin.gpio->IDR & (1 << pin.pin) ? GPIO_PIN_STATE_HIGH : GPIO_PIN_STATE_LOW;
}

void gpio_pin_toggle(struct gpio_pin pin)
{
	gpio_pin_write(pin, !gpio_pin_read(pin));
}

void gpio_pin_attach_interrupt(struct gpio_pin pin, enum gpio_pin_edge edge, void(*callback)(struct gpio_pin pin))
{
	// Lock this interface, next registers manipulation must be atomic
	__disable_irq();

	// Register callback
	callbacks[pin.pin].callback = callback;
	callbacks[pin.pin].pin = pin;

	// Select port
	uint32_t port = ((uint32_t)pin.gpio - (uint32_t)GPIOA) / ((uint32_t)GPIOB - (uint32_t)GPIOA);
	SYSCFG->EXTICR[(pin.pin & 0xCU) >> 2U] &= ~(0xFU << ((pin.pin & 0x3) * 4));
	SYSCFG->EXTICR[(pin.pin & 0xCU) >> 2U] |=  (port << ((pin.pin & 0x3) * 4));

	// Select edge
	switch(edge) {
		case GPIO_PIN_EDGE_RISING:
			EXTI->RTSR1 |=  (0x1U << (pin.pin & 0xFU));
			EXTI->FTSR1 &= ~(0x1U << (pin.pin & 0xFU));
			break;
		case GPIO_PIN_EDGE_FALLING:
			EXTI->RTSR1 &= ~(0x1U << (pin.pin & 0xFU));
			EXTI->FTSR1 |=  (0x1U << (pin.pin & 0xFU));
			break;
		case GPIO_PIN_EDGE_BOTH:
			EXTI->RTSR1 |=  (0x1U << (pin.pin & 0xFU));
			EXTI->FTSR1 |=  (0x1U << (pin.pin & 0xFU));
			break;
		default:
			break;
	}

	// Enable EXTI interrupt
	SET_BIT(EXTI->IMR1, EXTI_IMR1_IM0 << (pin.pin & 0xFU));

	switch(pin.pin) {
			case 0:
				NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(EXTI0_IRQn);
				break;
			case 1:
				NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(EXTI1_IRQn);
				break;
			case 2:
				NVIC_SetPriority(EXTI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(EXTI2_IRQn);
				break;
			case 3:
				NVIC_SetPriority(EXTI3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(EXTI3_IRQn);
				break;
			case 4:
				NVIC_SetPriority(EXTI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(EXTI4_IRQn);
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(EXTI9_5_IRQn);
				break;
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(EXTI15_10_IRQn);
				break;
		}

	// Unlock
	__enable_irq();
}
