
#ifndef GPIO_H
#define GPIO_H

#include "stm32h7xx.h"

/// Configuration

// LED
#define GPIO_PIN_USER_LED         (struct gpio_pin){.gpio = GPIOE, .pin = 15}

// Camera
#define GPIO_PIN_DCMI_D0          (struct gpio_pin){.gpio = GPIOC, .pin =  6}
#define GPIO_PIN_DCMI_D1          (struct gpio_pin){.gpio = GPIOC, .pin =  7}
#define GPIO_PIN_DCMI_D2          (struct gpio_pin){.gpio = GPIOE, .pin =  0}
#define GPIO_PIN_DCMI_D3          (struct gpio_pin){.gpio = GPIOE, .pin =  1}
#define GPIO_PIN_DCMI_D4          (struct gpio_pin){.gpio = GPIOC, .pin = 11}
#define GPIO_PIN_DCMI_D5          (struct gpio_pin){.gpio = GPIOD, .pin =  3}
#define GPIO_PIN_DCMI_D6          (struct gpio_pin){.gpio = GPIOB, .pin =  8}
#define GPIO_PIN_DCMI_D7          (struct gpio_pin){.gpio = GPIOE, .pin =  6}
#define GPIO_PIN_DCMI_D8          (struct gpio_pin){.gpio = GPIOC, .pin = 10}
#define GPIO_PIN_DCMI_D9          (struct gpio_pin){.gpio = GPIOC, .pin = 12}
#define GPIO_PIN_DCMI_PIXCL       (struct gpio_pin){.gpio = GPIOA, .pin =  6}
#define GPIO_PIN_DCMI_HSYNC       (struct gpio_pin){.gpio = GPIOA, .pin =  4}
#define GPIO_PIN_DCMI_VSYNC       (struct gpio_pin){.gpio = GPIOG, .pin =  9}
#define GPIO_PIN_I2C1_SDA         (struct gpio_pin){.gpio = GPIOB, .pin =  7}
#define GPIO_PIN_I2C1_SCL         (struct gpio_pin){.gpio = GPIOB, .pin =  6}
#define GPIO_PIN_CAM_PWDN         (struct gpio_pin){.gpio = GPIOG, .pin =  4}
#define GPIO_PIN_CAM_RESET        (struct gpio_pin){.gpio = GPIOG, .pin =  5}
#define GPIO_PIN_CAM_XCLK         (struct gpio_pin){.gpio = GPIOB, .pin =  5}
#define GPIO_PIN_CAM_PWR_EN       (struct gpio_pin){.gpio = GPIOG, .pin =  2}

// Sensors
#define GPIO_PIN_I2C4_SDA         (struct gpio_pin){.gpio = GPIOF, .pin = 15}
#define GPIO_PIN_I2C4_SCL         (struct gpio_pin){.gpio = GPIOF, .pin = 14}
#define GPIO_PIN_I2C2_SDA         (struct gpio_pin){.gpio = GPIOB, .pin = 11}
#define GPIO_PIN_I2C2_SCL         (struct gpio_pin){.gpio = GPIOB, .pin = 10}

// Mechanical sensors
#define GPIO_PIN_ANEMOMETER       (struct gpio_pin){.gpio = GPIOG, .pin =  0}
#define GPIO_PIN_GAUGE            (struct gpio_pin){.gpio = GPIOG, .pin =  1}
#define GPIO_PIN_ADC2_WIND_DIR    (struct gpio_pin){.gpio = GPIOA, .pin =  3}

// PMS sensor
#define GPIO_PIN_CON_PMS_RESET    (struct gpio_pin){.gpio = GPIOA, .pin =  8}
#define GPIO_PIN_CON_PMS_SET      (struct gpio_pin){.gpio = GPIOA, .pin = 10}
#define GPIO_PIN_CON_UART5_RTS    (struct gpio_pin){.gpio = GPIOC, .pin =  8}
#define GPIO_PIN_CON_UART5_CTS    (struct gpio_pin){.gpio = GPIOC, .pin =  9}
#define GPIO_PIN_CON_UART5_RX     (struct gpio_pin){.gpio = GPIOD, .pin =  2}
#define GPIO_PIN_CON_UART5_TX     (struct gpio_pin){.gpio = GPIOB, .pin = 13}

// Analog feedback
#define GPIO_PIN_ADC3_BAT_VOLTAGE (struct gpio_pin){.gpio = GPIOF, .pin =  4}

// GSM/LTE
#define GPIO_PIN_UART4_RTS        (struct gpio_pin){.gpio = GPIOA, .pin = 15}
#define GPIO_PIN_UART4_CTS        (struct gpio_pin){.gpio = GPIOB, .pin =  0}
#define GPIO_PIN_UART4_RX         (struct gpio_pin){.gpio = GPIOD, .pin =  0}
#define GPIO_PIN_UART4_TX         (struct gpio_pin){.gpio = GPIOD, .pin =  1}
#define GPIO_PIN_LTE_PWRKEY       (struct gpio_pin){.gpio = GPIOE, .pin =  2}
#define GPIO_PIN_LTE_RESET        (struct gpio_pin){.gpio = GPIOB, .pin =  9}

// External serial device
#define GPIO_PIN_CON_UART7_RTS    (struct gpio_pin){.gpio = GPIOE, .pin =  9}
#define GPIO_PIN_CON_UART7_CTS    (struct gpio_pin){.gpio = GPIOE, .pin = 10}
#define GPIO_PIN_CON_UART7_RX     (struct gpio_pin){.gpio = GPIOF, .pin =  6}
#define GPIO_PIN_CON_UART7_TX     (struct gpio_pin){.gpio = GPIOF, .pin =  7}

// WiFi SPI
#define GPIO_PIN_ESP32_MISO       (struct gpio_pin){.gpio = GPIOB, .pin =  4}
#define GPIO_PIN_ESP32_MOSI       (struct gpio_pin){.gpio = GPIOD, .pin =  7}
#define GPIO_PIN_ESP32_SCK        (struct gpio_pin){.gpio = GPIOA, .pin =  5}
#define GPIO_PIN_ESP32_CS         (struct gpio_pin){.gpio = GPIOG, .pin = 10}

// Ethernet RMII
#define GPIO_PIN_RMII_REFCLK      (struct gpio_pin){.gpio = GPIOA, .pin =  1}
#define GPIO_PIN_RMII_MDIO        (struct gpio_pin){.gpio = GPIOA, .pin =  2}
#define GPIO_PIN_RMII_MDC         (struct gpio_pin){.gpio = GPIOC, .pin =  1}
#define GPIO_PIN_RMII_RXDATAVALID (struct gpio_pin){.gpio = GPIOA, .pin =  7}
#define GPIO_PIN_RMII_RXD0        (struct gpio_pin){.gpio = GPIOC, .pin =  4}
#define GPIO_PIN_RMII_RXD1        (struct gpio_pin){.gpio = GPIOC, .pin =  5}
#define GPIO_PIN_RMII_TXEN        (struct gpio_pin){.gpio = GPIOG, .pin = 11}
#define GPIO_PIN_RMII_TXD0        (struct gpio_pin){.gpio = GPIOG, .pin = 13}
#define GPIO_PIN_RMII_TXD1        (struct gpio_pin){.gpio = GPIOG, .pin = 14}

// Power management pins
#define GPIO_PIN_WIFI_EN          (struct gpio_pin){.gpio = GPIOG, .pin =  7}
#define GPIO_PIN_ETH_EN           (struct gpio_pin){.gpio = GPIOB, .pin =  1}
#define GPIO_PIN_CON_I2C4_3V3_EN  (struct gpio_pin){.gpio = GPIOD, .pin =  8}
#define GPIO_PIN_CON_UART5_5V_EN  (struct gpio_pin){.gpio = GPIOD, .pin =  9}
#define GPIO_PIN_CON_I2C2_5V_EN   (struct gpio_pin){.gpio = GPIOD, .pin = 10}
#define GPIO_PIN_CON_I2C2_3V3_EN  (struct gpio_pin){.gpio = GPIOD, .pin = 11}


/// Library

struct gpio_pin {
	GPIO_TypeDef *gpio;
	uint8_t pin;
};

enum gpio_pin_mode {
	GPIO_PIN_MODE_INPUT     = 0,
	GPIO_PIN_MODE_OUTPUT    = 1,
	GPIO_PIN_MODE_ALTERNATE = 2,
	GPIO_PIN_MODE_ANALOG    = 3
};

enum gpio_pin_output_type {
	GPIO_PIN_OUTPUT_TYPE_PUSH_PULL  = 0,
	GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN = 1
};

enum gpio_pin_output_speed {
	GPIO_PIN_OUTPUT_SPEED_LOW        = 0,
	GPIO_PIN_OUTPUT_SPEED_MEDIUM     = 1,
	GPIO_PIN_OUTPUT_SPEED_HIGH       = 2,
	GPIO_PIN_OUTPUT_SPEED_VERY_HIGH  = 3,
};

enum gpio_pin_pupd {
	GPIO_PIN_PUPD_NO        = 0,
	GPIO_PIN_PUPD_PULL_UP   = 1,
	GPIO_PIN_PUPD_PULL_DOWN = 2,
};

enum gpio_pin_state {
	GPIO_PIN_STATE_LOW = 0,
	GPIO_PIN_STATE_HIGH
};

enum gpio_pin_edge {
	GPIO_PIN_EDGE_RISING = 0,
	GPIO_PIN_EDGE_FALLING,
	GPIO_PIN_EDGE_BOTH
};

// All functions are thread safe
void gpio_init(void);
void gpio_pin_configure(struct gpio_pin pin, enum gpio_pin_mode mode, enum gpio_pin_output_type output_type, enum gpio_pin_output_speed output_speed, enum gpio_pin_pupd pupd, uint8_t alternate_function, enum gpio_pin_state state);
void gpio_pin_write(struct gpio_pin pin, enum gpio_pin_state state);
enum gpio_pin_state gpio_pin_read(struct gpio_pin pin);
void gpio_pin_toggle(struct gpio_pin pin);
void gpio_pin_attach_interrupt(struct gpio_pin pin, enum gpio_pin_edge edge, void(*callback)(struct gpio_pin pin));

#endif

