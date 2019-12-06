#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "linked_list.h"

typedef struct {
	PtrDomain ptr;
	int value;
} TestNode;

HeadNode list;

void setUp(void) {
	list_init(&list);
}

void tearDown(void) {
	//DO NOTHING
}

void testListInit(void) {
	TEST_ASSERT_EQUAL(&list, list.ptr.next);
	TEST_ASSERT_EQUAL(&list, list.ptr.prev);
	TEST_ASSERT_EQUAL(list.len, 0);
}

void testAppendSingleStackNode(void) {
	TestNode myNode;
	myNode.value = 233;
	list_append(&list, &myNode.ptr);
	
	TEST_ASSERT_EQUAL(list.ptr.next, &myNode.ptr);
	TEST_ASSERT_EQUAL(myNode.ptr.prev, &list);
	TEST_ASSERT_EQUAL(myNode.ptr.next, &list);
	TEST_ASSERT_EQUAL(list.ptr.prev, &myNode.ptr);
	TEST_ASSERT_EQUAL(list.len, 1);
	TEST_ASSERT_EQUAL(myNode.value, 233);
}

void testAppendSingleHeapNode(void) {
	TestNode* myNode;
	myNode = malloc(sizeof(*myNode));
	myNode->value = 233;
	list_append(&list, &myNode->ptr);
	
	TEST_ASSERT_EQUAL(list.ptr.next, &myNode->ptr);
	TEST_ASSERT_EQUAL(myNode->ptr.prev, &list);
	TEST_ASSERT_EQUAL(myNode->ptr.next, &list);
	TEST_ASSERT_EQUAL(list.ptr.prev, &myNode->ptr);
	TEST_ASSERT_EQUAL(list.len, 1);
	TEST_ASSERT_EQUAL(myNode->value, 233);
	
	free(myNode);
}

void testAppendMultiHeapNode(void) {
	const int kNodeCnt = 5;
	int i;
	TestNode* curNode;
	PtrDomain *curPtr, *nextPtr;
	for(i= 0; i < kNodeCnt; i++) {
		curNode = malloc(sizeof(*curNode));
		curNode->value = i;
		list_append(&list, &curNode->ptr);
	}
	
	TEST_ASSERT_EQUAL(list.len, kNodeCnt);
	curPtr = &list.ptr;
	for(i = -1; i < kNodeCnt; i++) {
		nextPtr = curPtr->next;
		TEST_ASSERT_EQUAL(nextPtr->prev, curPtr);
		if(i >= 0) {
			TEST_ASSERT_EQUAL(((TestNode*)curPtr)->value, i);
		}
		curPtr = nextPtr;
	}
}

void testRemoveSingleStackNode(void) {
	TestNode myNode;
	myNode.value = 233;
	list_append(&list, &myNode.ptr);
	list_remove(&list, &myNode.ptr);
	
	TEST_ASSERT_EQUAL(&list, list.ptr.next);
	TEST_ASSERT_EQUAL(&list, list.ptr.prev);
	TEST_ASSERT_EQUAL(list.len, 0);
}

void testRemoveMultiHeapNode(void) {
	const int kNodeCnt = 5;
	int i;
	TestNode* curNode;
	PtrDomain* curPtr;
	for(i= 0; i < kNodeCnt; i++) {
		curNode = malloc(sizeof(*curNode));
		curNode->value = i;
		list_append(&list, &curNode->ptr);
	}
	
	for(i = 0; i < kNodeCnt; i++) {
		TEST_ASSERT_EQUAL(((TestNode*)list.ptr.next)->value, i);
		curPtr = list.ptr.next;
		list_remove(&list, curPtr);
		free(curPtr);
		TEST_ASSERT_EQUAL(list.len, 4 - i);
	}
	TEST_ASSERT_EQUAL(&list, list.ptr.next);
	TEST_ASSERT_EQUAL(&list, list.ptr.prev);
}
