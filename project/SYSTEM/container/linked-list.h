#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct _PtrDomain {
	struct _PtrDomain* prev;
	struct _PtrDomain* next;
} PtrDomain;


typedef struct {
	PtrDomain ptr; //next指向首元节点, prev指向尾节点
	int len;
} HeadNode;

void list_init(HeadNode* head);
void list_append(HeadNode* head, PtrDomain* newNode);
int list_size(HeadNode* head);
int list_remove(HeadNode* head, PtrDomain* node);

#endif
