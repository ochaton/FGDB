#include "lib/buddy/memory.h"
#include "common.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include <stdlib.h>
#include <string.h>

#include "lib/vector/vector.h"
#include "memory/hashmap.h"
#include "arena/meta.h"
#include "lru/lruq.h"
#include "wal/wal.h"

#include "unity.h"

arena_t   * arena;
disk_t    * disk;
lru_queue_t * lru;
hashmap_t hashmap;

void setUp() {
	buddy_new(8192); // 8Mb
	lru = new_lru_queue();
	arena = new_arena(1024);
	disk = init_disk("db.snap");
	arena->headers = init_headers(1024);
	hashmap = hashmap_new(); // ????
}

void tearDown() {
	hashmap_delete(hashmap);
	destroy_headers();
	destroy_arena(arena);
	destroy_lru_queue(lru);
	destroy_disk(disk);

	buddy_destroy();
}

/* Looks like key-val */
/* Smell like key-val */
/* Named by key-val */
/* But not a real key-val */
struct keyval {
	str_t key;
	str_t val;
};

void test1(void) {

	struct keyval kv = {
		{ 3, "key" },
		{ 5, "value" }
	};

	/* 1. Memcopy key */

	str_t * insert_key = (str_t *) malloc(sizeof(str_t));
	insert_key->ptr = (char *) malloc(kv.key.size);
	memcpy(insert_key->ptr, kv.key.ptr, kv.key.size);
	insert_key->size = kv.key.size;

	/* 2. Memcopy value */

	str_t * insert_val = (str_t *) malloc(sizeof(str_t));
	insert_val->ptr = (char *) malloc(kv.val.size);
	memcpy(insert_val->ptr, kv.val.ptr, kv.val.size);
	insert_val->size = kv.val.size;

	/* 3. Allocate page for value */

	key_meta_t key_meta = { -1, -1 };
	page_header_t * header = page_value_set(insert_val, &key_meta);

	TEST_ASSERT_NOT_NULL(header);
	TEST_ASSERT_MESSAGE(key_meta.header_key_id == 0, "First key should be the zero one inside header");
	TEST_ASSERT_MESSAGE(key_meta.page == 0, "First key should be inserted into 0-page");

	/* 4. Insert key into hashmap */

	hashmap_error_t err;
	int result = hashmap_insert_key(hashmap, &key_meta, insert_key, &err);
	TEST_ASSERT_MESSAGE(result == 0, "Key must be inserted into hashmap with status 0");
	TEST_ASSERT_MESSAGE(err == HASHMAP_SUCCESS, "Key must be inserted into hashmap with HASHMAP_SUCCESS");

	/* 5. Free temporary memory */

	free(insert_key->ptr);
	free(insert_key);
	free(insert_val->ptr);
	free(insert_val);

	/* 6. Select inserted key */

	str_t key2find = { 3, "key" };
	key_meta_t * found = hashmap_lookup_key(hashmap, &key2find, &err);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_MESSAGE(err == HASHMAP_SUCCESS, "Key must be found inside hashmap with HASHMAP_SUCCESS");

	/* 7. Get value from arena (or disk, whatever) */

	str_t retval;
	page_header_t * found_value_header = page_value_get(found, &retval);
	TEST_ASSERT_NOT_NULL(found_value_header);
	TEST_ASSERT_MESSAGE(found_value_header->page_id == found->page, "Header must be from the same page as the key");
	TEST_ASSERT_MESSAGE(found_value_header->arena_id == 0, "Header must heat page to the 0-page inside arena");

	TEST_ASSERT_EQUAL_STRING_LEN(retval.ptr, kv.val.ptr, retval.size);
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test1);
	return UNITY_END();
}
