#include "linked-list.h"

//��������ں���, �������Է�������ĳ��������ɶ

//ͷ�ڵ����ڼ�¼

typedef struct _PtrDomain {
	struct _PtrDomain* prev;
	struct _PtrDomain* next;
} PtrDomain;


typedef struct {
	PtrDomain ptr; //nextָ����Ԫ�ڵ�, prevָ��β�ڵ�
	int len;
} HeadNode;

void list_init(HeadNode* head) {
	head->ptr.prev = (PtrDomain*)head;
	head->ptr.next = (PtrDomain*)head;
	head->len = 0;
}

//��β�ڵ�����һ���ڵ�
void list_append(HeadNode* head, PtrDomain* newNode) {
	PtrDomain* rear = head->ptr.prev;
	rear->next = newNode;
	newNode->prev = rear;
	newNode->next = (PtrDomain*)head;
	head->ptr.prev = newNode;
	++head->len;
}

int list_size(HeadNode* head) {
	return head->len;
}

//���ڵ���������Ƴ���������ڵ���ͷŹ���
//����1��ʾ�ɹ�, ����0��ʾ������Ϊ��
int list_remove(HeadNode* head, PtrDomain* node) {
	PtrDomain* prev = node->prev;
	PtrDomain* next = node->next;
	if(head->len <= 0)
		return 0;
	prev->next = next;
	next->prev = prev;
	node->next = node;
	node->prev = node;
	--head->len;
	return 1;
}


