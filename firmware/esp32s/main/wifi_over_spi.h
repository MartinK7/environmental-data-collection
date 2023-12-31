
#ifndef MAIN_WIFI_OVER_SPI_H_
#define MAIN_WIFI_OVER_SPI_H_

#include <stdint.h>

/// Settings
#define ESP32_REGISTER_MAX_SIZE         (8 * 1024)
#define ESP32_SPI_MESSAGE_MAX_SIZE      (256)

/// Macros
#define ESP32_REGISTER_CALCUALTE_ADDRESS(name) ((size_t)(&(((struct wifi_over_spi_registers*)0)->name)))
#define ESP32_REGISTER_CALCUALTE_SIZE(name)    (sizeof(*(((struct wifi_over_spi_registers*)0)->name)))
#ifdef ESP_PLATFORM
	// Physically there is an offset
	#define ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET 8
#else
	// The physical offset is not present inside SPI communication
	#define ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET 0
#endif

/// Registers memory layout
struct __attribute__((packed, aligned(4))) wifi_over_spi_registers {
	uint8_t control_status;
	uint8_t wifi_ssid[32];
	uint8_t wifi_password[64];
	uint8_t tcp_domain[128];
	uint8_t tcp_port[2];
	uint8_t tcp_buffer_tx_len[2];
	uint8_t tcp_buffer_tx[ESP32_REGISTER_MAX_SIZE];
};

#define ESP32_REGISTER_ADDRESS_CONTROLSTATUS   ESP32_REGISTER_CALCUALTE_ADDRESS(control_status)
#define ESP32_REGISTER_ADDRESS_WIFI_SSID       ESP32_REGISTER_CALCUALTE_ADDRESS(wifi_ssid)
#define ESP32_REGISTER_ADDRESS_WIFI_PASSWORD   ESP32_REGISTER_CALCUALTE_ADDRESS(wifi_password)
#define ESP32_REGISTER_ADDRESS_TCP_DOMAIN      ESP32_REGISTER_CALCUALTE_ADDRESS(tcp_domain)
#define ESP32_REGISTER_ADDRESS_TCP_PORT        ESP32_REGISTER_CALCUALTE_ADDRESS(tcp_port)
#define ESP32_REGISTER_ADDRESS_BUFFER_TX_LEN   ESP32_REGISTER_CALCUALTE_ADDRESS(tcp_buffer_tx_len)
#define ESP32_REGISTER_ADDRESS_BUFFER_TX       ESP32_REGISTER_CALCUALTE_ADDRESS(tcp_buffer_tx)

#define ESP32_REGISTER_SIZE_CONTROLSTATUS      ESP32_REGISTER_CALCUALTE_SIZE(control)
#define ESP32_REGISTER_SIZE_WIFI_SSID          ESP32_REGISTER_CALCUALTE_SIZE(wifi_ssid)
#define ESP32_REGISTER_SIZE_WIFI_PASSWORD      ESP32_REGISTER_CALCUALTE_SIZE(wifi_password)
#define ESP32_REGISTER_SIZE_TCP_DOMAIN         ESP32_REGISTER_CALCUALTE_SIZE(tcp_domain)
#define ESP32_REGISTER_SIZE_TCP_PORT           ESP32_REGISTER_CALCUALTE_SIZE(tcp_port)
#define ESP32_REGISTER_SIZE_BUFFER_TX_LEN      ESP32_REGISTER_CALCUALTE_SIZE(tcp_buffer_tx_len)
#define ESP32_REGISTER_SIZE_BUFFER_TX          ESP32_REGISTER_CALCUALTE_SIZE(tcp_buffer_tx)

/// REGISTER_STATUS_CONTROL
// RSO - Control bits - Read and Set Only
#define ESP32_REGISTER_ADDRESS_CONTROL_WIFI_CONNECT_REQUEST_BIT   (1U <<  0U)
#define ESP32_REGISTER_ADDRESS_CONTROL_TCP_CONNECT_REQUEST_BIT    (1U <<  1U)
#define ESP32_REGISTER_ADDRESS_CONTROL_TCP_DISCONNECT_REQUEST_BIT (1U <<  2U)
#define ESP32_REGISTER_ADDRESS_CONTROL_TCP_SEND_REQUEST_BIT       (1U <<  3U)
// RO  - Status bits - Read Only
#define ESP32_REGISTER_ADDRESS_STATUS_WIFI_CONNECTED_BIT         (1U << (0U + ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET))
#define ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT              (1U << (1U + ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET))
#define ESP32_REGISTER_ADDRESS_STATUS_TCP_CONNECTED_BIT          (1U << (2U + ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET))
#define ESP32_REGISTER_ADDRESS_STATUS_TCP_DISCONNECTED_BIT       (1U << (3U + ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET))
#define ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT               (1U << (4U + ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET))
#define ESP32_REGISTER_ADDRESS_STATUS_TCP_SENT_BIT               (1U << (5U + ESP32_REGISTER_ADDRESS_CONTROL_BIT_OFFSET))



#endif /* MAIN_WIFI_OVER_SPI_H_ */
