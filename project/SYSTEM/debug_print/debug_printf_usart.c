#include "debug_printf_usart.h"

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
FILE __stdin;

//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) { 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f) { 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}

void _ttywrch(int ch) {
	USART1->DR = (u8) ch;  
}
#endif
