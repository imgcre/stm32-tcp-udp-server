#include "linked_list.h"

/**
 * @brief �����ʼ��
 * 
 * @param head ͷ�ڵ�ָ��
 */
void list_init(HeadNode* head) {
	head->ptr.prev = (PtrDomain*)head;
	head->ptr.next = (PtrDomain*)head;
	head->len = 0;
}

/**
 * @brief ������ĩβ��ӽڵ�
 * 
 * @param head ͷ�ڵ�ָ��
 * @param newNode �½ڵ�ָ��
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
 * @brief ��ȡ����ߴ�(�ڵ���)
 * 
 * @param head ͷ�ڵ�ָ��
 * @return int ����ߴ�
 */
int list_size(HeadNode* head) {
	return head->len;
}

/**
 * @brief ��������ɾ���ڵ�
 * 
 * @param head ͷ�ڵ�ָ��
 * @param node ��ɾ���ڵ�
 * @return int �ɹ����ط���
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
