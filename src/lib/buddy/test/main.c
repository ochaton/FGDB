#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#include "memory.h"


enum { Kb = 1 << 10 };

void test1(void) {
	int blocks[] = { 32*Kb, 16*Kb, 1*Kb, 8*Kb };
	for (uint32_t i = 0; i < 4; i++) {
		fprintf(stderr, "Allocating: %d\n", blocks[i]);
		void * ptr = buddy_alloc(blocks[i]);
		assert(ptr != NULL);
		buddy_dump();
	}

	return;
}

void test2(void) {
	void * ptr1 = buddy_alloc(8*Kb);
	buddy_dump();

	assert(ptr1 != NULL);
	buddy_free(ptr1);
	buddy_dump();

	void * ptr2 = buddy_alloc(8*Kb);
	buddy_dump();
	
	assert(ptr2 != NULL);
	assert(ptr1 == ptr2);
	
	buddy_free(ptr2);
	buddy_dump();

	return;
}

void test3(void) {
	void *ptr = buddy_alloc(64*Kb);
	buddy_dump();

	assert(ptr != NULL);
	buddy_free(ptr);
	buddy_dump();
}

void test4(void) {
	int blocks[] = { 8*Kb, 8*Kb, 16*Kb, 32*Kb };
	void * mems[sizeof(blocks)] = { NULL };

	for (int i = 0; i < (int) COUNT_OF(blocks); i++) {
		mems[i] = buddy_alloc(blocks[i]);
		assert(mems[i] != NULL);
		buddy_dump();
	}
	fprintf(stderr, "============================================ Test 4 Free  =============================================\n");
	for (int i = COUNT_OF(blocks) - 1; i >= 0; i--) {
		buddy_free(mems[i]);
		buddy_dump();
	}
}

int main(void)
{
	fprintf(stderr, "=========================================== Test 1 Started  ===========================================\n");
	buddy_new(128);
	test1();
	buddy_destroy();
	fprintf(stderr, "=========================================== Test 1 Finished ===========================================\n");

	fprintf(stderr, "=========================================== Test 2 Started  ===========================================\n");
	buddy_new(64);
	test2();
	buddy_destroy();
	fprintf(stderr, "=========================================== Test 2 Finished ===========================================\n");

	fprintf(stderr, "=========================================== Test 3 Started  ===========================================\n");
	buddy_new(64);
	test3();
	buddy_destroy();
	fprintf(stderr, "=========================================== Test 3 Finished ===========================================\n");

	fprintf(stderr, "=========================================== Test 4 Started  ===========================================\n");
	buddy_new(64);
	test4();
	buddy_destroy();
	fprintf(stderr, "=========================================== Test 4 Finished ===========================================\n");

	return 0;
}
