#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "usmart.h"
#include "timer.h"
#include "lcd.h"
#include "sram.h"
#include "rtc.h"
#include "beep.h"
#include "adc.h"
#include "temperature.h"
#include "sram.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "tcp_server_demo.h"
#include "udp_demo.h"
#include "tcp_udp_server.h"

extern void Adc_Temperate_Init(void);	//�����ڲ��¶ȴ�������ʼ������
//����UI
//mode:
//bit0:0,������;1,����ǰ�벿��UI
//bit1:0,������;1,���غ�벿��UI
void lwip_test_ui(u8 mode)
{
	u8 speed;
	u8 buf[30]; 
	POINT_COLOR=RED;
	if(mode&1<<0)
	{
		LCD_Fill(30,30,lcddev.width,110,WHITE);	//�����ʾ
		LCD_ShowString(30,50,200,16,16,"Ethernet lwIP Test");
	}
	if(mode&1<<1)
	{
		LCD_Fill(30,110,lcddev.width,lcddev.height,WHITE);	//�����ʾ
		LCD_ShowString(30,110,200,16,16,"lwIP Init Successed");
		if(lwipdev.dhcpstatus==2)sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//��ӡ��̬IP��ַ
		else sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//��ӡ��̬IP��ַ
		LCD_ShowString(30,130,210,16,16,buf); 
		speed=LAN8720_Get_Speed();//�õ�����
		if(speed&1<<1)LCD_ShowString(30,150,200,16,16,"Ethernet Speed:100M");
		else LCD_ShowString(30,150,200,16,16,"Ethernet Speed:10M");
		LCD_ShowString(30,170,200,16,16,"KEY0:TCP UDP Test");
		//LCD_ShowString(30,190,200,16,16,"KEY2:UDP Test");
	}
}

int main(void)
{
	u8 t;
	u8 key;
	delay_init(168);       	//��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	uart_init(115200);   	//���ڲ���������
	usmart_dev.init(84); 	//��ʼ��USMART
	LED_Init();  			//LED��ʼ��
	KEY_Init();  			//������ʼ��
	LCD_Init(); 			//LCD��ʼ��
	FSMC_SRAM_Init();		//��ʼ���ⲿSRAM  
	BEEP_Init();			//��������ʼ��
	My_RTC_Init();  		//RTC��ʼ��
	Adc_Init();  			//ADC��ʼ�� 
	Adc_Temperate_Init(); 	//�ڲ��¶ȴ�������ʼ��
	TIM3_Int_Init(999,839); //100khz��Ƶ��,����1000Ϊ10ms
	mymem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	mymem_init(SRAMEX);		//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM);	//��ʼ��CCM�ڴ��
	POINT_COLOR = RED; 		//��ɫ����
	lwip_test_ui(1);		//����ǰ�벿��UI
	
	//�ȳ�ʼ��lwIP(����LAN8720��ʼ��),��ʱ�����������,�����ʼ����ʧ��!! 
	LCD_ShowString(30,110,200,16,16,"lwIP Initing...");
	while(lwip_comm_init()!=0)
	{
		LCD_ShowString(30,110,200,16,16,"lwIP Init failed!");
		delay_ms(1200);
		LCD_Fill(30,110,230,110+16,WHITE);//�����ʾ
		LCD_ShowString(30,110,200,16,16,"Retrying...");  
	}
	LCD_ShowString(30,110,200,16,16,"lwIP Init Successed");
	//�ȴ�DHCP��ȡ 
 	LCD_ShowString(30,130,200,16,16,"DHCP IP configing...");
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
	{
		lwip_periodic_handle();
	}
	lwip_test_ui(2);//���غ�벿��UI 
	while(1)
	{
		key=KEY_Scan(0);
		switch(key)
		{
			case KEY0_PRES:
				tcp_udp_test();
				lwip_test_ui(3);
				break;
		}
		lwip_periodic_handle();
		delay_ms(2);
		t++;
		if(t==100)LCD_ShowString(30,230,200,16,16,"Please choose a mode!");
		if(t==200)
		{ 
			t=0;
			LCD_Fill(30,230,230,230+16,WHITE);
			LED0=!LED0;
		} 
	}
}








