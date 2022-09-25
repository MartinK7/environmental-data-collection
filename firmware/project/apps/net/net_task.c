
#include "net_task.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/apps/httpd.h"

#include "ethernetif.h"
#include <string.h>

#include "gpio.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static void ethernet_link_status_updated(struct netif *netif);

void Error_Handler(void)
{
	while(1);
}

// Variables Initialization
/*shared*/ struct netif gnetif;
static ip4_addr_t ipaddr;
static ip4_addr_t netmask;
static ip4_addr_t gw;
static uint8_t IP_ADDRESS[4];
static uint8_t NETMASK_ADDRESS[4];
static uint8_t GATEWAY_ADDRESS[4];

#define STACK_SIZE 1024
static StaticTask_t xTaskBuffer;
static StackType_t xStack[STACK_SIZE];

/**
  * LwIP initialization function
  */
void net_task_initialise(void)
{
	// IP addresses initialization
	IP_ADDRESS[0] = 0;
	IP_ADDRESS[1] = 0;
	IP_ADDRESS[2] = 0;
	IP_ADDRESS[3] = 0;
	NETMASK_ADDRESS[0] = 255;
	NETMASK_ADDRESS[1] = 255;
	NETMASK_ADDRESS[2] = 255;
	NETMASK_ADDRESS[3] = 255;
	GATEWAY_ADDRESS[0] = 0;
	GATEWAY_ADDRESS[1] = 0;
	GATEWAY_ADDRESS[2] = 0;
	GATEWAY_ADDRESS[3] = 0;

	// Initilialize the LwIP stack with RTOS
	tcpip_init( NULL, NULL );

	// IP addresses initialization without DHCP (IPv4)
	IP4_ADDR(&ipaddr, IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
	IP4_ADDR(&netmask, NETMASK_ADDRESS[0], NETMASK_ADDRESS[1] , NETMASK_ADDRESS[2], NETMASK_ADDRESS[3]);
	IP4_ADDR(&gw, GATEWAY_ADDRESS[0], GATEWAY_ADDRESS[1], GATEWAY_ADDRESS[2], GATEWAY_ADDRESS[3]);

	// add the network interface (IPv4/IPv6) with RTOS
	netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

	// Registers the default network interface
	netif_set_default(&gnetif);

	if (netif_is_link_up(&gnetif))
	{
		// When the netif is fully configured this function must be called
		netif_set_up(&gnetif);
	}
	else
	{
		// When the netif link is down this function must be called
		netif_set_down(&gnetif);
	}

	// Set the link callback function, this function is called on change of link status*/
	netif_set_link_callback(&gnetif, ethernet_link_status_updated);

	// Create the Ethernet link handler thread
	xTaskCreateStatic(ethernet_link_thread, "EthLink", STACK_SIZE, (void *)&gnetif, 1, xStack, &xTaskBuffer);

	dhcp_start(&gnetif);

	httpd_init();
}

/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
static void ethernet_link_status_updated(struct netif *netif)
{
	if (netif_is_up(netif)) {
		// netif is up
	} else {
		// netif is down
	}
}
