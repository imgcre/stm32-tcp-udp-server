#ifndef __TCP_UDP_SERVER_H
#define __TCP_UDP_SERVER_H
#include "sys.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip_comm.h"
#include "linked_list.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//TCP Server 测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
 
#define M_TCP_SERVER_RX_BUFSIZE	200		//定义tcp server最大接收数据长度
#define M_TCP_SERVER_PORT			8088	//定义tcp server的端口
#define UDP_PORT 1234
 
#define MAX_TCP_CLIENT_CNT 8


/*
//tcp服务器连接状态
enum m_tcp_server_states
{
	ES_TCPSERVER_NONE = 0,		//没有连接
	ES_TCPSERVER_ACCEPTED,		//有客户端连接上了 
	ES_TCPSERVER_CLOSING,		//即将关闭连接
}; 

*/
//LWIP回调函数使用的结构体
struct m_tcp_server_struct
{
	u8 state;               //当前连接状
	struct tcp_pcb *pcb;    //指向当前的pcb
	struct pbuf *p;         //指向接收/或传输的pbuf
}; 

typedef struct {
	PtrDomain ptr;
	struct tcp_pcb* pcb;
	struct pbuf* sendBuf;
} TcpClientInfo;

void tcp_udp_test(void);//TCP Server测试函数
err_t m_tcp_server_accept(void *arg,struct tcp_pcb *newpcb,err_t err);
err_t m_tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void m_tcp_server_error(void *arg,err_t err);
err_t m_tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
err_t m_tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
void m_tcp_server_senddata(struct tcp_pcb *tpcb, struct m_tcp_server_struct *es);
void m_tcp_server_connection_close(struct tcp_pcb *tpcb, struct m_tcp_server_struct *es);
void m_tcp_server_remove_timewait(void);
void tcp_sendData(TcpClientInfo* clientInfo, char* data, int len);
void tcp_sendBuf(TcpClientInfo* clientInfo);
void m_udp_demo_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
#endif 
