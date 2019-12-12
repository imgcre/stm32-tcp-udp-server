#include "tcp_udp_server.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"  


#define UDP_PORT 1234

HeadNode m_tcpClientList;

char *udpDataToSend = "Hello UDP\r\n";
char *tcpDataToSend = "Hello TCP\r\n";

char sprintBuff[128];

void tcp_udp_test(void) {
	struct tcp_pcb *tcp, *conn;
	struct udp_pcb *udp;
	struct ip_addr remoteIp;
	struct pbuf *buf;
	int i;

	list_init(&m_tcpClientList);

	//TODO: 以下代码或需要重写
	printf("tcp binding...\r\n");
	
	if(!(tcp = tcp_new())) {
		printf("tcp create failed\r\n");
		return;
	}

	if(tcp_bind(tcp,IP_ADDR_ANY,TCP_SERVER_PORT) != ERR_OK) {
		printf("tcp bind failed\r\n");
		return;
	}
	
	printf("tcp listening on %d.%d.%d.%d:%d\r\n", lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3], TCP_SERVER_PORT);

	conn = tcp_listen(tcp);
	tcp_accept(conn, m_tcp_server_accept);

	
	printf("udp connecting...\r\n");
	if(!(udp = udp_new())) {
		printf("udp create failed\r\n");
		return;
	}

	if(udp_bind(udp, IP_ADDR_ANY, UDP_PORT) != ERR_OK) {
		printf("udp bind failed\r\n");
		return;
	}
	
	printf("udp bound on port %d\r\n", UDP_PORT);
	udp_recv(udp, m_udp_demo_recv, NULL);
	


	while(1) {
		TcpClientInfo* node;
		switch(KEY_Scan(0)) {
			case KEY0_PRES: //TCP发送
				//给所有的客户端发送消息
				if(list_size(&m_tcpClientList) == 0) {
					printf("tcp no client connected\r\n");
					break;
				} 
				
				for(node = (TcpClientInfo*)m_tcpClientList.ptr.next; &node->ptr != &m_tcpClientList.ptr; node = (TcpClientInfo*)node->ptr.next) {
					sprintf(sprintBuff, "Your addr is %s:%d\r\n", ipaddr_ntoa(&node->pcb->remote_ip), node->pcb->remote_port);
					tcp_sendData(node, sprintBuff, strlen(sprintBuff));
				}
				
				break;
		}
		lwip_periodic_handle();
		delay_ms(2);
	}
	tcp_close(tcp);
	tcp_close(conn);
	m_tcp_server_remove_timewait();
	udp_disconnect(udp); 
	udp_remove(udp);
	
}


//TCP客户连接回调
err_t m_tcp_server_accept(void *arg, struct tcp_pcb *clientPcb, err_t err) {
	TcpClientInfo *info; 

	if(list_size(&m_tcpClientList) >= MAX_TCP_CLIENT_CNT) {
		printf("too many tcp clients\r\n");
		return ERR_MEM;
	}
	
	printf("tcp client connected: %s:%d\r\n", ipaddr_ntoa(&clientPcb->remote_ip), clientPcb->remote_port);

	tcp_setprio(clientPcb,TCP_PRIO_MIN);

	info = mem_malloc(sizeof(*info));
	if(info == NULL)
		return ERR_MEM;

	info->pcb = clientPcb;
	info->sendBuf = NULL;
	list_append(&m_tcpClientList, &info->ptr);

	tcp_arg(clientPcb, info);
	tcp_recv(clientPcb, m_tcp_server_recv);
	tcp_err(clientPcb, m_tcp_server_error);
	tcp_poll(clientPcb, m_tcp_server_poll, 1);
	tcp_sent(clientPcb, m_tcp_server_sent);
	return ERR_OK;
}

//TCP接收回调
err_t m_tcp_server_recv(void *arg, struct tcp_pcb *clientPcb, struct pbuf *p, err_t err)
{
	u32 data_len = 0;
	struct pbuf *q;
  TcpClientInfo* info = arg;

	if(p == NULL) { //TCP连接待关闭
		printf("tcp client disconnected: %s:%d\r\n", ipaddr_ntoa(&clientPcb->remote_ip), clientPcb->remote_port);
		list_remove(&m_tcpClientList, &info->ptr);
		tcp_close(clientPcb); //断开连接
		return ERR_OK;
	}
	
	printf("tcp received(%s:%d):\r\n", ipaddr_ntoa(&clientPcb->remote_ip), clientPcb->remote_port);
	for(q = p; q != NULL; q = q->next) {
		int i;
		for(i = 0; i < q->len; i++) {
			putchar(((char*)q->payload)[i]);
		}
	}
	
	tcp_sendData(info, tcpDataToSend, strlen(tcpDataToSend));
	

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
		return ERR_ABRT;
	}

	return ERR_OK;
}

//TCP数据发送回调
err_t m_tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
	TcpClientInfo* info = arg;
	LWIP_UNUSED_ARG(len); 
	if(info->sendBuf)tcp_sendBuf(info);
	return ERR_OK;
}

void tcp_sendData(TcpClientInfo* clientInfo, char* data, int len) {
	clientInfo->sendBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	pbuf_take(clientInfo->sendBuf, data, len);
	tcp_sendBuf(clientInfo);
}

void tcp_sendBuf(TcpClientInfo* clientInfo) {
	struct pbuf *ptr;
	u16 plen;
	err_t wr_err=ERR_OK;
	 while((wr_err==ERR_OK)&&clientInfo->sendBuf&&(clientInfo->sendBuf->len<=tcp_sndbuf(clientInfo->pcb)))
	 {
		ptr=clientInfo->sendBuf; //刚开始ptr就是es的p
		wr_err=tcp_write(clientInfo->pcb,ptr->payload,ptr->len,1); //将要发送的数据加入发送缓冲队列中
		if(wr_err==ERR_OK)
		{ 
			plen=ptr->len;
			clientInfo->sendBuf=ptr->next; //指向下一个pbuf
			if(clientInfo->sendBuf)pbuf_ref(clientInfo->sendBuf); //pbuf的ref加一
			pbuf_free(ptr);
			tcp_recved(clientInfo->pcb,plen); //更新tcp窗口大小
		}else if(wr_err==ERR_MEM)clientInfo->sendBuf=ptr;
		tcp_output(clientInfo->pcb); //将发送缓冲队列中的数据发送出去
	 }
}



extern void tcp_pcb_purge(struct tcp_pcb *pcb);
extern struct tcp_pcb *tcp_active_pcbs;
extern struct tcp_pcb *tcp_tw_pcbs;
//强制删除TCP Server主动断开时的time wait
void m_tcp_server_remove_timewait(void) {
	struct tcp_pcb *pcb,*pcb2; 
	while(tcp_active_pcbs!=NULL)
	{
		lwip_periodic_handle(); //继续轮询
		delay_ms(10); //等待tcp_active_pcbs为空  
	}
	pcb=tcp_tw_pcbs;
	while(pcb!=NULL) //如果有等待状态的pcbs
	{
		tcp_pcb_purge(pcb); 
		tcp_tw_pcbs=pcb->next;
		pcb2=pcb;
		pcb=pcb->next;
		memp_free(MEMP_TCP_PCB,pcb2);	
	}
}

//udp接收回调
void m_udp_demo_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	struct pbuf *q, *bufToSend;
	
	if(p == NULL) {
		udp_disconnect(upcb);
		return;
	}
	
	printf("udp received(%s:%d):\r\n", ipaddr_ntoa(addr), port);
	for(q = p; q != NULL; q = q->next) {
		int i;
		for(i = 0; i < q->len; i++) {
			putchar(((char*)q->payload)[i]);
		}
	}
	
	bufToSend = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)udpDataToSend), PBUF_POOL);
	if(bufToSend) {
		bufToSend->payload = udpDataToSend; 
		udp_sendto(upcb, bufToSend, addr, port);
		pbuf_free(bufToSend);
	} 
	
	pbuf_free(p);
} 
