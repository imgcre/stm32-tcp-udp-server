#include "tcp_udp_server.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"  
#include "linked_list.h"

#define UDP_PORT 1234

HeadNode m_tcpClientList;

void tcp_udp_test(void) {
	struct tcp_pcb *tcp, *conn;
	struct udp_pcb *udp;
	struct ip_addr remoteIp;

	list_init(&m_tcpClientList);

	//TODO: ���´������Ҫ��д
	printf("tcp listening on %d.%d.%d.%d:%d\r\n", lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3], M_TCP_SERVER_PORT);
	
	if(!(tcp = tcp_new())) {
		printf("tcp create failed\r\n");
		return;
	}

	if(tcp_bind(tcp,IP_ADDR_ANY,M_TCP_SERVER_PORT) != ERR_OK) {
		printf("tcp bind failed\r\n");
		return;
	}

	conn = tcp_listen(tcp);
	tcp_accept(conn, m_tcp_server_accept);

	//TODO: UDP remote ip and port
	if(!(udp = udp_new())) {
		printf("udp create failed\r\n");
		return;
	}

	IP4_ADDR(&remoteIp, 192, 168, 1, 1);
	if(udp_connect(udp, &remoteIp, UDP_PORT) != ERR_OK) {
		printf("udp connect failed\r\n");
		return;
	}

	if(udp_bind(udp, IP_ADDR_ANY, UDP_PORT) != ERR_OK) {
		printf("udp bind failed\r\n");
		return;
	}
	
	udp_recv(udp, m_udp_demo_recv, NULL);

	while(1) {
		lwip_periodic_handle();
		delay_ms(2);
	}
	tcp_close(tcp);
	tcp_close(conn);
	m_tcp_server_remove_timewait();
	udp_disconnect(udp); 
	udp_remove(udp);
}

typedef struct {
	PtrDomain ptr;
	struct tcp_pcb* pcb;
} TcpClientInfo;

err_t m_tcp_server_accept(void *arg, struct tcp_pcb *clientPcb, err_t err) {
	TcpClientInfo *info; 

	if(list_size(&m_tcpClientList) >= MAX_TCP_CLIENT_CNT) {
		printf("too many tcp clients\r\n");
		return ERR_MEM;
	}

	tcp_setprio(clientPcb,TCP_PRIO_MIN);

	info = mem_malloc(sizeof(*info));
	if(info == NULL)
		return ERR_MEM;

	info->pcb = clientPcb;
	list_append(&m_tcpClientList, &info->ptr);

	tcp_arg(clientPcb, info);
	tcp_recv(clientPcb, m_tcp_server_recv);
	tcp_err(clientPcb, m_tcp_server_error);
	tcp_poll(clientPcb, m_tcp_server_poll, 1);
	tcp_sent(clientPcb, m_tcp_server_sent);
	return ERR_OK;
}

err_t m_tcp_server_recv(void *arg, struct tcp_pcb *clientPcb, struct pbuf *p, err_t err)
{
	u32 data_len = 0;
	struct pbuf *q;
  	TcpClientInfo* info = arg;

	if(p == NULL) { //TCP���Ӵ��ر�
		return ERR_OK;
	}

	tcp_recved(clientPcb,p->tot_len);
	pbuf_free(p);
	return ERR_OK;
}

void m_tcp_server_error(void *arg, err_t err) {
	TcpClientInfo* info = arg;
	printf("tcp error: %x\r\n", (u32)arg);
	if(arg != NULL) {
		list_remove(&m_tcpClientList, &info->ptr);
		mem_free(info);
	}
} 

err_t m_tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
	err_t ret_err;
	TcpClientInfo* info = arg;

	if(info == NULL) {
		tcp_abort(tpcb);
		list_remove(&m_tcpClientList, &info->ptr);
		return ERR_ABRT;
	}

	//TODO: �����﷢������
	return ERR_OK;
}

//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
err_t m_tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
	return ERR_OK;
} 

extern void tcp_pcb_purge(struct tcp_pcb *pcb);	//�� tcp.c���� 
extern struct tcp_pcb *tcp_active_pcbs;			//�� tcp.c���� 
extern struct tcp_pcb *tcp_tw_pcbs;				//�� tcp.c����  
//ǿ��ɾ��TCP Server�����Ͽ�ʱ��time wait
void m_tcp_server_remove_timewait(void) {
	struct tcp_pcb *pcb,*pcb2; 
	while(tcp_active_pcbs!=NULL)
	{
		lwip_periodic_handle();//������ѯ
		delay_ms(10);//�ȴ�tcp_active_pcbsΪ��  
	}
	pcb=tcp_tw_pcbs;
	while(pcb!=NULL)//����еȴ�״̬��pcbs
	{
		tcp_pcb_purge(pcb); 
		tcp_tw_pcbs=pcb->next;
		pcb2=pcb;
		pcb=pcb->next;
		memp_free(MEMP_TCP_PCB,pcb2);	
	}
}

void m_udp_demo_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	/*
	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//���յ���Ϊ�յ�����ʱ
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
		for(q=p;q!=NULL;q=q->next)  //����������pbuf����
		{
			//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
			//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//��������
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
		}
		upcb->remote_ip=*addr; 				//��¼Զ��������IP��ַ
		upcb->remote_port=port;  			//��¼Զ�������Ķ˿ں�
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_demo_flag|=1<<6;	//��ǽ��յ�������
		pbuf_free(p);//�ͷ��ڴ�
	}else
	{
		udp_disconnect(upcb); 
	} 
	*/
} 

































