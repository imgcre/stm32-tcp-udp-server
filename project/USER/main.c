#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "timer.h"
#include "sram.h"
#include "sram.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "tcp_udp_server.h"

int main(void)
{
	u8 t;
	u8 key;
	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	uart_init(115200);   	//串口波特率设置
	KEY_Init();  			//按键初始化
	FSMC_SRAM_Init();		//初始化外部SRAM  
	TIM3_Int_Init(999,839); //100khz的频率,计数1000为10ms
	mymem_init(SRAMIN);		//初始化内部内存池
	mymem_init(SRAMEX);		//初始化外部内存池
	mymem_init(SRAMCCM);	//初始化CCM内存池
	
	//先初始化lwIP(包括LAN8720初始化),此时必须插上网线,否则初始化会失败!! 
	printf("lwIP Initing...\r\n");
	while(lwip_comm_init()!=0)
	{
		printf("lwIP Init failed!\r\n");
		delay_ms(1200);
		printf("retrying...\r\n");
	}
	
	printf("lwIP Init Successed\r\n");
	//等待DHCP获取 
	printf("DHCP IP configing...\r\n");
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF)) {
		lwip_periodic_handle();
	}
	
	if(lwipdev.dhcpstatus==2)printf("DHCP IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//打印动态IP地址
	else printf("Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//打印静态IP地址
	
	tcp_udp_test();
}








