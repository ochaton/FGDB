#include "lib/buddy/memory.h"
#include "common.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include <stdlib.h>

#include "lib/vector/vector.h"
#include "arena/meta.h"
#include "lru/lruq.h"

#include "unity.h"

arena_t   * arena;
disk_t    * disk;
lru_queue_t * lru;
// hashmap_t * hashmap;

void setUp() {
	lru = new_lru_queue();
	buddy_new(8192); // 8Mb
	arena = new_arena(1024);
	disk = init_disk("db.snap");
	arena->headers = init_headers(1024);
}

void tearDown() {
	destroy_headers();
	destroy_arena(arena);
	destroy_lru_queue(lru);
	destroy_disk(disk);

	buddy_destroy();
}

void test1(void) {
	str_t val = { 3, "val" };
	str_t key_val = { 3, "key" };

	hashmap_key_t key = { &key_val, 0, 0 };
	page_header_t * header = page_value_set(&val, &key);

	str_t retval;
	page_value_get(&key, &retval);
	fprintf(stderr, "%u\n", retval.size);

	page_value_unset(&key, &retval);
	fprintf(stderr, "%u\n", retval.size);
	for (size_t i = 0; i < retval.size; i++) {
		fprintf(stderr, "%c", retval.ptr[i]);
	}
	fprintf(stderr, "\n");

	str_t newval;
	fprintf(stderr, "%d\n", page_value_get(&key, &newval) == NULL);
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test1);
	return UNITY_END();
}
