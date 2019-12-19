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

#define TCP_SERVER_PORT 8088
#define UDP_PORT0 1234
#define UDP_PORT1 1235
 
#define MAX_TCP_CLIENT_CNT 8

#define ARR_LEN(x) (sizeof(x)/sizeof((x)[0]))

//LWIP�ص�����ʹ�õĽṹ��
struct m_tcp_server_struct
{
	u8 state;               //��ǰ����״
	struct tcp_pcb *pcb;    //ָ��ǰ��pcb
	struct pbuf *p;         //ָ�����/�����pbuf
}; 

typedef struct {
	PtrDomain ptr;
	struct tcp_pcb* pcb;
	struct pbuf* sendBuf;
} TcpClientInfo;

typedef struct {
	short port;
	struct udp_pcb *pcb;
} UdpInfo;

void tcp_udp_test(void);//TCP Server���Ժ���
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
