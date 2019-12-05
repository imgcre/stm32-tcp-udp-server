#include "linked-list.h"

//数据域放在后面, 这样可以方便外面的程序操作那啥

//头节点用于记录

typedef struct _PtrDomain {
	struct _PtrDomain* prev;
	struct _PtrDomain* next;
} PtrDomain;


typedef struct {
	PtrDomain ptr; //next指向首元节点, prev指向尾节点
	int len;
} HeadNode;

void list_init(HeadNode* head) {
	head->ptr.prev = (PtrDomain*)head;
	head->ptr.next = (PtrDomain*)head;
	head->len = 0;
}

//在尾节点后添加一个节点
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

//将节点从链表中移除，不负责节点的释放工作
//返回1表示成功, 返回0表示链表已为空
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


