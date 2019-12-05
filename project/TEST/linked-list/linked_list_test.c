#include <stdio.h>
#include "unity.h"

void setUp() {
  printf("setUp my_test\n");
}

void tearDown() {
  printf("tearDown my_test\n");
}

void testWillAlwaysPass() {
	TEST_ASSERT_EQUAL_INT(42, 42);
}
