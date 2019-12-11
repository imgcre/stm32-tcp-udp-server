#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "lwip_comm.h"
#include "tcp_udp_server.h"

int main(void) {
	delay_init(168);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);
	KEY_Init();
	
	printf("lwIP initing...\r\n");
	while(lwip_comm_init() != 0) {
		printf("lwIP init failed\r\n");
		delay_ms(1200);
		printf("retrying...\r\n");
	}
	printf("lwIP init succeeded\r\n");

	printf("DHCP configing...\r\n");
	while((lwipdev.dhcpstatus != 2) && (lwipdev.dhcpstatus != 0XFF)) {
		lwip_periodic_handle();
	}

	printf("using %s ip: %d.%d.%d.%d\r\n", lwipdev.dhcpstatus == 2 ? "DHCP" : "static", lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	while(1) {
		tcp_udp_test();
	}
}








