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
#include "message/message.h"
#include "recovery/recovery.h"

#include "unity.h"

arena_t   * arena;
disk_t    * disk;
lru_queue_t * lru;
hashmap_t hashmap;
wal_logger_t * wal_logger;

void setUp() {
	buddy_new(8192); // 8Mb
	lru = new_lru_queue();
	arena = new_arena(1024);

	config_t config;
	sprintf(config.disk.snap_dir, ".");
	sprintf(config.disk.key_file, "key.key");
	sprintf(config.wal.wal_dir, "wal");
	config.arena.size = 1024;

	disk = init_disk(&config);

	arena->headers = init_headers(1024);
	assert(hashmap_new(&hashmap)); // ????
	unlink("log/0000000000000000000001.log");
}

void tearDown() {
	destroy_headers();
	destroy_arena(arena);
	destroy_lru_queue(lru);
	hashmap_delete(hashmap);
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

	str_t * insert_key = string_copy(&kv.key);

	/* 2. Memcopy value */

	str_t * insert_val = string_copy(&kv.val);

	/* 3. Allocate page for value */

	key_meta_t * key_meta = (key_meta_t *) malloc(sizeof(key_meta_t));
	key_meta->weak_key = insert_key;
	key_meta->header_key_id = -1;
	key_meta->page = -1;
	page_header_t * header = page_value_set(insert_val, key_meta);

	TEST_ASSERT_NOT_NULL(header);
	TEST_ASSERT_MESSAGE(key_meta->header_key_id == 0, "First key should be the zero one inside header");
	TEST_ASSERT_MESSAGE(key_meta->page == 0, "First key should be inserted into 0-page");

	/* 4. Insert key into hashmap */

	hashmap_error_t err;
	int result = hashmap_insert_key(hashmap, key_meta, insert_key, &err);
	TEST_ASSERT_MESSAGE(result == 0, "Key must be inserted into hashmap with status 0");
	TEST_ASSERT_MESSAGE(err == HASHMAP_SUCCESS, "Key must be inserted into hashmap with HASHMAP_SUCCESS");

	/* 5. Free temporary memory */

	destroy_string(insert_val);

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

void binary_structure_test(void) {
	msg_t m = {
		INSERT,
		{ 3, "key" },
		{ 5, "value" }
	};
	transaction_t     t = { NULL, &m };
	wal_logger_t*     l = new_wal_logger(1234);
	wal_log_record_t* r = to_wal_record(l, &t);
	TEST_ASSERT_MESSAGE(r->LSN == 1235, "LSN must be old_LSN + 1");
	binary_record_t*  b = to_binary(r);
	TEST_ASSERT_MESSAGE(*((lsn_t*)    &b->ptr[8])  == 1235,   "LSN is not gone");
	TEST_ASSERT_MESSAGE(*((char*)     &b->ptr[16])  == INSERT, "OPERATION is not gone");
	TEST_ASSERT_MESSAGE(*((uint16_t*) &b->ptr[17])  == 3,      "KEY SIZE is not gone");
	TEST_ASSERT_EQUAL_STRING_LEN(m.key.ptr, &b->ptr[19], 3);
	TEST_ASSERT_MESSAGE(*((uint16_t*) &b->ptr[22]) == 5,      "VAL SIZE is not gone");
	TEST_ASSERT_EQUAL_STRING_LEN(m.val.ptr, &b->ptr[24], 5);

	destroy_binary_record(b);
	destroy_wal_record(r);
	destroy_wal_logger(l);
}

void binary_logs_writing_test(void) {
	msg_t m = {
		INSERT,
		{ 3, "key" },
		{ 5, "value" }
	};

	transaction_t t = { NULL, &m };
	wal_logger_t* l = new_wal_logger(0);
	write_log(l, &t);
	fprintf(stderr, "OPA\n");

	msg_t m2 = {
		UPDATE,
		{ 4, "key1" },
		{ 6, "value1" }
	};

	transaction_t t2 = { NULL, &m2 };
	write_log(l, &t2);
	wal_unlogger_t* u = new_unlogger("log/0000000000000000000000.log");
	lsn_t lastLSN = get_latest_log_LSN(u);
	transaction_t* rt = recover_transaction(u);
	TEST_ASSERT_MESSAGE(lastLSN == 2, "Last logged LSN is correct");
	TEST_ASSERT_EQUAL_STRING_LEN("key", rt->msg->key.ptr, rt->msg->key.size);
	TEST_ASSERT_EQUAL_STRING_LEN("value", rt->msg->val.ptr, rt->msg->val.size);
	TEST_ASSERT_MESSAGE(rt->msg->cmd == INSERT, "OPERATIONS are the same 1");

	destroy_transaction(rt);

	rt = recover_transaction(u);
	TEST_ASSERT_EQUAL_STRING_LEN("key1",   rt->msg->key.ptr, rt->msg->key.size);
	TEST_ASSERT_EQUAL_STRING_LEN("value1", rt->msg->val.ptr, rt->msg->val.size);
	TEST_ASSERT_MESSAGE(rt->msg->cmd == UPDATE, "OPERATIONS are the same 2");

	destroy_transaction(rt);
	destroy_wal_unlogger(u);
	destroy_wal_logger(l);
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test1);
	RUN_TEST(binary_structure_test);
	RUN_TEST(binary_logs_writing_test);
	wal_recovery(1);
	return UNITY_END();
}
