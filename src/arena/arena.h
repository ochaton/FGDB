#ifndef ARENA_H
#define ARENA_H

#include "common.h"
#include "memory/hashmap.h"

typedef struct hashmap_key hashmap_key_t;

typedef struct arena_node {
	// reverse pointer to key
	hashmap_key_t * rev_key;
	enum {
		FREE_NODE,
		USED_NODE,
		DIRTY_NODE
	} state:8;

	// MAXIMUM = 8Mb
	uint32_t size:24;
	char ptr[1];
} arena_node_t; // sizeof = 16


typedef struct {
	char * end;
	uint32_t lru_lsn; uint32_t used;
	uint16_t fragmentated; uint16_t tail;
	uint8_t nodes:8;
	enum { UNLOCKED_PAGE, LOCKED_PAGE } lock:8;

	// We have 2 bytes more for something
} arena_page_header_t; // sizeof = 24


enum { PAGE_SIZE = 4 << 10 }; // 4 Kb
enum { PAGE_HEADER_SIZE = sizeof(arena_page_header_t) };
enum { PAGE_PAYLOAD_SIZE = PAGE_SIZE - PAGE_HEADER_SIZE };


typedef struct {
	arena_page_header_t header;
	char data[PAGE_PAYLOAD_SIZE]; // data section
} arena_page_t; // sizeof = PAGE_SIZE


typedef struct {
	uint32_t allocated_bytes, used_bytes, fragmentated_bytes;
	uint32_t npages;
	arena_page_t pages[1];
} arena_t; // sizeof = 48

/* 	page_alloc: find free space in allocated pages
	increment used and return pointer back
*/
arena_node_t * page_node_alloc(arena_t * arena, size_t bytes);

/*	page_free: find the beginning of page
	and free space
*/
void page_node_free(arena_t * arena, arena_node_t * node);

arena_t * arena_create(uint32_t pages);

arena_page_t * align2page(arena_node_t * node);
uint64_t offsetFromPage (arena_node_t * node);


// typedef struct {
// 	arena_page_t * page;
// 	lru_cache_node_t * left, * right;
// } lru_cache_node_t;

// typedef struct {
// 	lru_cache_node_t * last, * first;
// 	uint32_t lsn;
// } lru_cache_t;

#endif // ARENA_H