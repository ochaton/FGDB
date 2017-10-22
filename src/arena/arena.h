#ifndef ARENA_H
#define ARENA_H

#include "common.h"
#include "memory/hashmap.h"


typedef struct{
	hashmap_key_t * rev_key; // reverse pointer to key
	size_t size;
	char ptr[1];
} arena_node_t;

typedef struct {
	uint32_t total, used;
	uint32_t lsn; // last touched
	enum { CLEAN, DIRTY } fragmentated;
} arena_page_header_t;

enum { PAGE_SIZE = 4 << 10 }; // 4 Kb
enum { PAGE_HEADER_SIZE = sizeof(arena_page_header_t) };
enum { PAGE_PAYLOAD_SIZE = PAGE_SIZE - PAGE_HEADER_SIZE };

typedef struct {
	arena_page_header_t header;
	char data[PAGE_PAYLOAD_SIZE]; // data section
} arena_page_t;

typedef struct {
	uint32_t allocated_bytes, used_bytes, fragmentated_bytes;
	uint32_t lsn;
	uint32_t npages;
	arena_page_t * pages;
} arena_t;

/* 	page_alloc: find free space in allocated pages
	increment used and return pointer back
*/
arena_node_t * page_alloc(arena_t * arena, size_t bytes);

/*	page_free: find the beginning of page
	and free space
*/
void page_free(arena_t * arena, arena_node_t * node);

arena_t * arena_create(uint32_t pages);

arena_page_t * align2page(arena_node_t * node);
uint64_t offsetFromPage (arena_node_t * node);

#endif // ARENA_H