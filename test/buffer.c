#include "buffer.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test1(void) {
	char msg[] = "0123456789";
	buf_t * buf = init_buffer(sizeof(msg));
	assert(!buffer_push(buf, msg, sizeof(msg)));

	char ch;
	assert(!buffer_pop(buf, &ch, 1));
	assert(ch == 0);

	buffer_push(buf, &ch, 1);

	char msg_ret[sizeof(msg)];
	assert(!buffer_pop(buf, msg_ret, sizeof(msg_ret)));
	assert(!strcmp(msg_ret, msg));

	destroy_buffer(buf);
}

void test2(void) {
	buf_t * buf = init_buffer(10);
	for (int i = 0; i < 10; i++) {
		assert(!buffer_push(buf, (char *) &i, 1));
	}

	for (int i = 0; i < 10; i++) {
		char ch;
		assert(!buffer_shift(buf, &ch, 1));
		assert(ch == i);
	}

	destroy_buffer(buf);
}

void test3(void) {
	buf_t * buf = init_buffer(10);

	char nums[] = { 1, 0, 2, 0, 4, 0, 8, 0, 16, 0 };
	for (int i = 0, n = 0; i < sizeof(nums); i += 2, n++) {
		assert(!buffer_push(buf, (char *) &nums[i], 2));

		char ch;
		assert(!buffer_shift(buf, &ch, 1));
		assert(ch == nums[n]);
	}

	char ch = 1;
	assert(-1 == buffer_push(buf, &ch, 1));
	assert(buf->used == 5);

	for (int i = 0; i < 5; i++) {
		assert(!buffer_unshift(buf, (char *) &i, 1));
	}

	assert(buf->used == 10);

	destroy_buffer(buf);
}

int main(int argc, char const *argv[]) {
	test1();
	test2();
	test3();
	return 0;
}
