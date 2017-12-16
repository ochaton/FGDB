#include "lib/hashmap/HashMap.h"

/* Test Suite Unity */
#include <unity.h>
#include <assert.h>
#include <stdio.h>

hm_node_ptr hashmap;

void setUp(void) {
	uint32_t result = hash_new_node(&hashmap, 0);
	assert(hashmap);
	assert(result == 0);
}

void tearDown() {
	uint32_t result = hash_erase(hashmap);
	assert(result == 1);
	TEST_ASSERT_MESSAGE(result == 1, "Hashmap must been erased with 1 status");
}

void test1() {
	str_t key2find = { 3, "key" };
	TEST_ASSERT_MESSAGE(hash_search(hashmap, key2find) == NULL, "Must be not found");

	int meta = 1;
	TEST_ASSERT_MESSAGE(hash_insert(hashmap, key2find, &meta) == 1, "Must inserted with status = 1");

	avlnode_ptr found = hash_search(hashmap, key2find);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, key2find.ptr, key2find.size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	found = hash_search(hashmap, key2find);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, key2find.ptr, key2find.size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	TEST_ASSERT_MESSAGE(hash_delete(hashmap, key2find) == 1, "Must be deleted with status = 1");

	found = hash_search(hashmap, key2find);
	TEST_ASSERT_NULL(found);
}

void test2() {
	avlnode_ptr found;
	int meta = 1;

	/* Insert first key */
	str_t firstkey = { 3, "key" };
	TEST_ASSERT_MESSAGE(hash_insert(hashmap, firstkey, &meta) == 1, "Must inserted with status = 1");

	/* Insert second key */
	str_t secondkey = { 6, "second" };
	TEST_ASSERT_MESSAGE(hash_insert(hashmap, secondkey, &meta) == 1, "Must inserted with status = 1");

	/* Find first key */
	found = hash_search(hashmap, firstkey);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, firstkey.ptr, firstkey.size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	/* Find second key */
	found = hash_search(hashmap, secondkey);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, secondkey.ptr, secondkey.size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	/* Delete second key */
	TEST_ASSERT_MESSAGE(hash_delete(hashmap, secondkey) == 1, "Must be deleted with status = 1");

	/* Find first key */
	found = hash_search(hashmap, firstkey);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, firstkey.ptr, firstkey.size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");
}

void test3() {

	str_t insertKey = { 3, "key" };
	avlnode_ptr found, not_found;
	int meta = 1;

	for (uint try = 0; try < 2; try++) {
		/* Insert key */
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, insertKey, &meta) == 1, "Must inserted with status = 1");

		/* Find key */
		found = hash_search(hashmap, insertKey);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, insertKey.ptr, insertKey.size);
		TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

		/* Delete key */
		TEST_ASSERT_MESSAGE(hash_delete(hashmap, insertKey) == 1, "Must be deleted with status = 1");

		/* Find key */
		not_found = hash_search(hashmap, insertKey);
		TEST_ASSERT_NULL(not_found);
	}
}

void test4() {

	avlnode_ptr found, not_found;
	str_t keys[] = {
		{ 5, "first"  },
		{ 6, "second" },
		{ 5, "third"  },
		{ 6, "fourth" }
	};
	int metas[] = { 1, 2, 3, 4, 5 };

	/* Insert keys */
	for (int num = 0; num < sizeof keys / sizeof *keys; num++) {
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, keys[num], &metas[num]) == 1, "Must inserted with status = 1");
	}

	/* Select keys */
	for (int num = 0; num < sizeof keys / sizeof *keys; num++) {
		found = hash_search(hashmap, keys[num]);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, keys[num].ptr, keys[num].size);
		TEST_ASSERT_MESSAGE(found->meta == &metas[num], "Save pointer to meta");
	}

	/* Delete keys 1,2 */
	for (int num = 0; num < 2; num++) {
		TEST_ASSERT_MESSAGE(hash_delete(hashmap, keys[num]) == 1, "Must be deleted with status = 1");
	}

	/* Insert 5th key */
	str_t fifth = { 5, "fifth" };
	{
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, fifth, &metas[5]) == 1, "Must inserted with status = 1");
		found = hash_search(hashmap, fifth);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key.ptr, fifth.ptr, fifth.size);
		TEST_ASSERT_MESSAGE(found->meta == &metas[5], "Save pointer to meta");
	}
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test1);
	RUN_TEST(test2);
	RUN_TEST(test3);
	RUN_TEST(test4);
	return UNITY_END();
}
