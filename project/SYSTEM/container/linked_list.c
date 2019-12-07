#include "linked_list.h"

/**
 * @brief 链表初始化
 * 
 * @param head 头节点指针
 */
void list_init(HeadNode* head) {
	head->ptr.prev = (PtrDomain*)head;
	head->ptr.next = (PtrDomain*)head;
	head->len = 0;
}

/**
 * @brief 向链表末尾添加节点
 * 
 * @param head 头节点指针
 * @param newNode 新节点指针
 */
void list_append(HeadNode* head, PtrDomain* newNode) {
	PtrDomain* rear = head->ptr.prev;
	rear->next = newNode;
	newNode->prev = rear;
	newNode->next = (PtrDomain*)head;
	head->ptr.prev = newNode;
	++head->len;
}

/**
 * @brief 获取链表尺寸(节点数)
 * 
 * @param head 头节点指针
 * @return int 链表尺寸
 */
int list_size(HeadNode* head) {
	return head->len;
}

/**
 * @brief 从链表中删除节点
 * 
 * @param head 头节点指针
 * @param node 欲删除节点
 * @return int 成功返回非零
 */
int list_remove(HeadNode* head, PtrDomain* node) {
	PtrDomain* prev = node->prev;
	PtrDomain* next = node->next;
	if(head->len <= 0)
		return 0;
	if(node->prev == node && node == node->next)
		return 0;
	prev->next = next;
	next->prev = prev;
	node->next = node;
	node->prev = node;
	--head->len;
	return 1;
}
