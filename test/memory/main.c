#include <unity.h>
#include <assert.h>
#include <stdio.h>

#include "memory/hashmap.h"

hm_node_ptr hashmap;

void setUp(void) {
	hashmap = hashmap_new(); // ????
}

void tearDown() {
	hashmap_delete(hashmap);
}

void test1() {
	str_t key2insert = { 3, "key" };
	// int hashmap_insert_key(hashmap_t hmap, hashmap_key_t * new_key, hashmap_error_t *err);
	hashmap_error_t err;
	page_id_t page = 0;

	hashmap_key_t key = {
		&key2insert,
		0,
		page
	};

	int result = hashmap_insert_key(hashmap, &key, &err);
	TEST_ASSERT_MESSAGE(result == 0, "Must be 0");
	TEST_ASSERT_MESSAGE(err == HASHMAP_SUCCESS, "Must be HASHMAP_SUCCESS");

	hashmap_key_t * found = hashmap_lookup_key(hashmap, &key2insert, &err);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_MESSAGE(err == HASHMAP_SUCCESS, "Must be HASHMAP_SUCCESS");

	TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, key2insert.ptr, key2insert.size);
	TEST_ASSERT_MESSAGE(found->header_key_id == 0, "Offset must be 0");

	TEST_ASSERT_MESSAGE(found->page == page, "Save pointer to page");

	// hashmap_key_t * hashmap_delete_key(hashmap_t hmap, str_t * key, hashmap_error_t *err)
	hashmap_key_t * deleted = hashmap_delete_key(hashmap, &key2insert, &err);
	TEST_ASSERT_NOT_NULL(deleted);
	TEST_ASSERT_MESSAGE(err == HASHMAP_SUCCESS, "Must be HASHMAP_SUCCESS");
	free(deleted);
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test1);
	return UNITY_END();
}
