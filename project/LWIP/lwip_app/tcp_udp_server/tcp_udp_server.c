#include "tcp_udp_server.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"  

HeadNode m_tcpClientList;
char sprintBuff[128];

UdpInfo m_udpInfos[] = {
	{UDP_PORT0}, 
	{UDP_PORT1},
};

void tcp_udp_test(void) {
	struct tcp_pcb *tcp, *conn;
	struct ip_addr remoteIp;
	struct pbuf *buf;
	int i;

	list_init(&m_tcpClientList);
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
	
	for(i = 0; i < ARR_LEN(m_udpInfos); i++) {
		UdpInfo* curInfo = &m_udpInfos[i];
		printf("udp%d connecting...\r\n", i);
		if(!(curInfo->pcb = udp_new())) {
			printf("udp%d create failed\r\n", i);
			return;
		}
		if(udp_bind(curInfo->pcb, IP_ADDR_ANY, curInfo->port) != ERR_OK) {
			printf("udp%d bind failed\r\n", i);
			return;
		}
		printf("udp%d bound on port %d\r\n", i,  curInfo->port);
		udp_recv(curInfo->pcb, m_udp_demo_recv, curInfo);
	}

	while(1) {
		TcpClientInfo* node;
		switch(KEY_Scan(0)) {
			case KEY0_PRES: //TCP����
				//�����еĿͻ��˷�����Ϣ
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
	
	for(i = 0; i < ARR_LEN(m_udpInfos); i++) {
		struct udp_pcb* curPcb = m_udpInfos[i].pcb;
		udp_disconnect(curPcb); 
		udp_remove(curPcb);
	}
}


//TCP�ͻ����ӻص�
err_t m_tcp_server_accept(void *arg, struct tcp_pcb *clientPcb, err_t err) {
	TcpClientInfo *info; 

	//if(list_size(&m_tcpClientList) >= MAX_TCP_CLIENT_CNT) {
	//	printf("too many tcp clients\r\n");
	//	return ERR_MEM;
	//}
	
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

//TCP���ջص�
err_t m_tcp_server_recv(void *arg, struct tcp_pcb *clientPcb, struct pbuf *p, err_t err)
{
	u32 data_len = 0;
	struct pbuf *q;
  TcpClientInfo* info = arg;
	static char *dataToSend = "Hello TCP\r\n";

	if(p == NULL) { //TCP���Ӵ��ر�
		printf("tcp client disconnected: %s:%d\r\n", ipaddr_ntoa(&clientPcb->remote_ip), clientPcb->remote_port);
		list_remove(&m_tcpClientList, &info->ptr);
		tcp_close(clientPcb); //�Ͽ�����
		return ERR_OK;
	}
	
	//printf("tcp received(%s:%d):\r\n", ipaddr_ntoa(&clientPcb->remote_ip), clientPcb->remote_port);
	//for(q = p; q != NULL; q = q->next) {
	//	int i;
	//	for(i = 0; i < q->len; i++) {
	//		putchar(((char*)q->payload)[i]);
	//	}
	//}
	
	tcp_sendData(info, dataToSend, strlen(dataToSend));
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

//TCP���ݷ��ͻص�
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
		ptr=clientInfo->sendBuf; //�տ�ʼptr����es��p
		wr_err=tcp_write(clientInfo->pcb,ptr->payload,ptr->len,1); //��Ҫ���͵����ݼ��뷢�ͻ��������
		if(wr_err==ERR_OK)
		{ 
			plen=ptr->len;
			clientInfo->sendBuf=ptr->next; //ָ����һ��pbuf
			if(clientInfo->sendBuf)pbuf_ref(clientInfo->sendBuf); //pbuf��ref��һ
			pbuf_free(ptr);
			tcp_recved(clientInfo->pcb,plen); //����tcp���ڴ�С
		}else if(wr_err==ERR_MEM)clientInfo->sendBuf=ptr;
		tcp_output(clientInfo->pcb); //�����ͻ�������е����ݷ��ͳ�ȥ
	 }
}



extern void tcp_pcb_purge(struct tcp_pcb *pcb);
extern struct tcp_pcb *tcp_active_pcbs;
extern struct tcp_pcb *tcp_tw_pcbs;
//ǿ��ɾ��TCP Server�����Ͽ�ʱ��time wait
void m_tcp_server_remove_timewait(void) {
	struct tcp_pcb *pcb,*pcb2; 
	while(tcp_active_pcbs!=NULL)
	{
		lwip_periodic_handle(); //������ѯ
		delay_ms(10); //�ȴ�tcp_active_pcbsΪ��  
	}
	pcb=tcp_tw_pcbs;
	while(pcb!=NULL) //����еȴ�״̬��pcbs
	{
		tcp_pcb_purge(pcb); 
		tcp_tw_pcbs=pcb->next;
		pcb2=pcb;
		pcb=pcb->next;
		memp_free(MEMP_TCP_PCB,pcb2);	
	}
}

//udp���ջص�
void m_udp_demo_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	struct pbuf *q, *bufToSend;
	char* dataToSend;
	UdpInfo *info = arg;
	
	if(p == NULL) {
		udp_disconnect(upcb);
		return;
	}
	
	/*
	printf("udp received(%s:%d):\r\n", ipaddr_ntoa(addr), port);
	for(q = p; q != NULL; q = q->next) {
		int i;
		for(i = 0; i < q->len; i++) {
			putchar(((char*)q->payload)[i]);
		}
	}
	*/
	
	//sprintBuff
	
	if(info == &m_udpInfos[0]) {
		dataToSend = "Hello from udp 0\r\n";
	} else if(info == &m_udpInfos[1]) {
		dataToSend = "Hello from another udp\r\n";
	} else {
		dataToSend = "Hello from unknown udp\r\n";
	}
	
	bufToSend = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)dataToSend), PBUF_POOL);
	if(bufToSend) {
		bufToSend->payload = dataToSend; 
		udp_sendto(upcb, bufToSend, addr, port);
		pbuf_free(bufToSend);
	} 
	
	pbuf_free(p);
} 
