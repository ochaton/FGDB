#include "lib/hashmap/HashMap.h"

/* Test Suite Unity */
#include <unity.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


hm_node_ptr hashmap;
const int32_t MAX_N = 100;
int STEP = 0;
int DD = 0;
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
	str_t * key2find = char2string("key", 3);
	str_t * key2insert = char2string("key", 3);

	TEST_ASSERT_MESSAGE(hash_search(hashmap, key2find) == NULL, "Must be not found");

	int meta = 1;
	TEST_ASSERT_MESSAGE(hash_insert(hashmap, key2insert, &meta) == 1, "Must inserted with status = 1");

	avlnode_ptr found = hash_search(hashmap, key2find);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, key2find->ptr, key2find->size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	found = hash_search(hashmap, key2find);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, key2find->ptr, key2find->size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	TEST_ASSERT_MESSAGE(hash_delete(hashmap, key2insert) == 1, "Must be deleted with status = 1");

	found = hash_search(hashmap, key2find);
	TEST_ASSERT_NULL(found);

	destroy_string(key2find);
}

void test2() {
	avlnode_ptr found;
	int meta = 1;

	/* Insert first key */
	str_t * firstkey = char2string("key", 3);
	TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(firstkey), &meta) == 1, "Must inserted with status = 1");

	/* Insert second key */
	str_t * secondkey = char2string("second", 6);
	TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(secondkey), &meta) == 1, "Must inserted with status = 1");

	/* Find first key */
	found = hash_search(hashmap, firstkey);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, firstkey->ptr, firstkey->size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	/* Find second key */
	found = hash_search(hashmap, secondkey);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, secondkey->ptr, secondkey->size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	/* Delete second key */
	TEST_ASSERT_MESSAGE(hash_delete(hashmap, secondkey) == 1, "Must be deleted with status = 1");

	/* Find first key */
	found = hash_search(hashmap, firstkey);
	TEST_ASSERT_NOT_NULL(found);
	TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, firstkey->ptr, firstkey->size);
	TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

	destroy_string(firstkey);
	destroy_string(secondkey);
}

void test3() {

	avlnode_ptr found, not_found;
	int meta = 1;

	for (uint try = 0; try < 2; try++) {
		/* Insert key */
		str_t * insertKey = char2string("key", 3);
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(insertKey), &meta) == 1, "Must inserted with status = 1");

		/* Find key */
		found = hash_search(hashmap, insertKey);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, insertKey->ptr, insertKey->size);
		TEST_ASSERT_MESSAGE(found->meta == &meta, "Save pointer to meta");

		/* Delete key */
		TEST_ASSERT_MESSAGE(hash_delete(hashmap, insertKey) == 1, "Must be deleted with status = 1");

		/* Find key */
		not_found = hash_search(hashmap, insertKey);
		TEST_ASSERT_NULL(not_found);
		destroy_string(insertKey);
	}
}

void test4() {

	avlnode_ptr found, not_found;
	str_t * keys[] = {
		char2string("first" , 5),
		char2string("second", 6),
		char2string("third" , 5),
		char2string("fourth", 6)
	};
	int metas[] = { 1, 2, 3, 4, 5 };

	/* Insert keys */
	for (int num = 0; num < sizeof keys / sizeof *keys; num++) {
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(keys[num]), &metas[num]) == 1, "Must inserted with status = 1");
	}

	/* Select keys */
	for (int num = 0; num < sizeof keys / sizeof *keys; num++) {
		found = hash_search(hashmap, keys[num]);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, keys[num]->ptr, keys[num]->size);
		TEST_ASSERT_MESSAGE(found->meta == &metas[num], "Save pointer to meta");
	}

	/* Delete keys 1,2 */
	for (int num = 0; num < 2; num++) {
		TEST_ASSERT_MESSAGE(hash_delete(hashmap, keys[num]) == 1, "Must be deleted with status = 1");
	}

	/* Insert 5th key */
	{
		str_t * fifth = char2string("fifth", 5);
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(fifth), &metas[5]) == 1, "Must inserted with status = 1");
		found = hash_search(hashmap, fifth);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, fifth->ptr, fifth->size);
		TEST_ASSERT_MESSAGE(found->meta == &metas[5], "Save pointer to meta");
		destroy_string(fifth);
	}

	for (int num = 0; num < sizeof keys / sizeof *keys; num++) {
		destroy_string(keys[num]);
	}
}

void test5() {
	str_t keys[10000];
	int pages[10000];
	for (int i = 1; i < 1000; i++) {
		pages[i] = i;
		keys[i].size = i;
		keys[i].ptr = (char *) malloc( (i + 1) * sizeof(char));
		memset(keys[i].ptr, 'a', i);
		keys[i].ptr[i]++;
	}
	for (int i = 1; i < 1000; i++) {
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(&keys[i]), &pages[i]) == 1, "Must inserted with status = 1");
	}
	for (int i = 1; i < 1000; i++) {
		avlnode_ptr found;
		found = hash_search(hashmap, &keys[i]);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, keys[i].ptr, keys[i].size);
		TEST_ASSERT_MESSAGE(found->meta == &pages[i], "Save pointer to meta");
	}
	for (int i = 1; i < 1000; i++) {
		free(keys[i].ptr);
	}
}

void test6() {
	str_t keys[10000];
	int pages[10000];
	for (int i = 1; i < 1000; i++) {
		pages[i] = i;
		keys[i].size = i;
		keys[i].ptr = (char *) malloc( (i + 1) * sizeof(char));
		memset(keys[i].ptr, 'a', i);
		keys[i].ptr[i]++;
	}
	for (int i = 1; i < 1000; i++) {
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(&keys[i]), &pages[i]) == 1, "Must inserted with status = 1");
	}
	for (int i = 1; i < 1000; i++) {
		avlnode_ptr found;
		found = hash_search(hashmap, &keys[i]);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, keys[i].ptr, keys[i].size);
		TEST_ASSERT_MESSAGE(found->meta == &pages[i], "Save pointer to meta");
	}
	for (int i = 1; i < 1000; i++) {
		TEST_ASSERT_MESSAGE(hash_delete(hashmap, &keys[i]) == 1, "Must be deleted with status = 1");
	}
	for (int i = 1; i < 1000; i++) {
		free(keys[i].ptr);
	}
}

void test7() {
	str_t keys[10000];
	int pages[10000];
	for (int i = 1; i < 1000; i++) {
		pages[i] = i;
		keys[i].size = i;
		keys[i].ptr = (char *) malloc( (i + 1) * sizeof(char));
		memset(keys[i].ptr, 'a', i);
		keys[i].ptr[i]++;
	}
	for (int i = 1; i < 1000; i++) {
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(&keys[i]), &pages[i]) == 1, "Must inserted with status = 1");
	}
	for (int i = 1; i < 1000; i++) {
		avlnode_ptr found;
		found = hash_search(hashmap, &keys[i]);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, keys[i].ptr, keys[i].size);
		TEST_ASSERT_MESSAGE(found->meta == &pages[i], "Save pointer to meta");
	}
	for (int i = 1; i < 1000; i++) {
		TEST_ASSERT_MESSAGE(hash_delete(hashmap, &keys[i]) == 1, "Must be deleted with status = 1");
	}
	for (int i = 1; i < 1000; i++) {
		avlnode_ptr found;
		found = hash_search(hashmap, &keys[i]);
		TEST_ASSERT_NULL(found);
	}
	for (int i = 1; i < 1000; i++) {
		free(keys[i].ptr);
	}
}

void test8() {
	const int MAX_M = 100;
	str_t keys[MAX_M + 1];
	int pages[MAX_M + 1];
	int flag[MAX_M + 1];
	for (int i = 1; i < MAX_M; i++) {
		pages[i] = i;
		keys[i].size = (i * 7 + 57) % 7 + (MAX_M - i + 9) % 5 + 4 ;
		keys[i].ptr = (char *) malloc(keys[i].size);
		for (int j = 0; j < keys[i].size; j++) {
			keys[i].ptr[j] =(char) ((13 * j + 27 * i + 7) % 100 + 38 );
		}
	}
	for (int i = 1; i < MAX_M; i++) {
		avlnode_ptr found;
		TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(&keys[i]), &pages[i]) == 1, "Must inserted with status = 1");
		found = hash_search(hashmap, &keys[i]);
		TEST_ASSERT_NOT_NULL(found);
		TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, keys[i].ptr, keys[i].size);
		TEST_ASSERT_MESSAGE(found->meta == &pages[i], "Save pointer to meta");
	}
	for (int i = 1; i < MAX_M; i++) {
		free(keys[i].ptr);
	}
}

void test9() {
	str_t keys[MAX_N + 1];
	int pages[MAX_N + 1];
	int flag[MAX_N + 1];
	for (int i = 1; i < MAX_N; i++) {
		pages[i] = i;
		keys[i].size = (i * 7 + 57) % 7 + (MAX_N - i + 9) % 5 + 4 ;
		keys[i].ptr = (char *) malloc(keys[i].size);
		for (int j = 0; j < keys[i].size; j++) {
			keys[i].ptr[j] =(char) ((13 * j + 27 * i + 7) % 100 + 38 );
		}
	}
	for (int i = 1; i < MAX_N; i++) {
		avlnode_ptr found;
		found = hash_search(hashmap, &keys[i]);
		if (!found) {
			TEST_ASSERT_MESSAGE(hash_insert(hashmap, string_copy(&keys[i]), &pages[i]) == 1, "Must inserted with status = 1");
			flag[i] = 0;
		} else {
			flag[i] = 1;
		}
	}
	for (int i = 1; i < MAX_N; i++) {
		avlnode_ptr found;
		found = hash_search(hashmap, &keys[i]);
		if (!flag[i]) {
			TEST_ASSERT_NOT_NULL(found);
			TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, keys[i].ptr, keys[i].size);
			TEST_ASSERT_MESSAGE(found->meta == &pages[i], "Save pointer to meta");
		}
	}
	int lim = (MAX_N * 2) / 3;
	for (int i = MAX_N / 3; i < lim; i++) {
		avlnode_ptr found;
		found = hash_search(hashmap, &keys[i]);
		if (!flag[i]) {
			//printf("DEL;\n");
			TEST_ASSERT_NOT_NULL(found);
			TEST_ASSERT_EQUAL_STRING_LEN(found->key->ptr, keys[i].ptr, keys[i].size);
			TEST_ASSERT_MESSAGE(found->meta == &pages[i], "Save pointer to meta");
			TEST_ASSERT_MESSAGE(hash_delete(hashmap, &keys[i]) == 1, "Must be deleted with status = 1");
		}
	}
	for (int i = 1; i < MAX_N; i++) {
		if (keys[i].ptr) {
			free(keys[i].ptr);
		}
	}
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test1);
	RUN_TEST(test2);
	RUN_TEST(test3);
	RUN_TEST(test4);
	RUN_TEST(test5);
	RUN_TEST(test6);
	RUN_TEST(test7);
	RUN_TEST(test8);
	RUN_TEST(test9);
	return UNITY_END();
}
