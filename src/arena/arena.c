#include "arena.h"
#include "lib/buddy/memory.h"
#include <assert.h>

static uint32_t blog (uint32_t num) {
	uint32_t rv = 0; for (; num >> rv; rv++);
	return (uint32_t) (1 << (rv - 1)) == num ? rv - 1 : rv;
}

arena_t * arena_create(uint32_t pages) {
	uint32_t bytes = pages * PAGE_SIZE + sizeof(arena_t);
	arena_t * arena = buddy_alloc(bytes);

	assert(arena);

	arena->allocated_bytes = pages * sizeof(arena_page_t);
	arena->used_bytes      = 0;

	arena->fragmentated_bytes = 0;
	arena->npages = pages;

	// allign by PAGE_SIZE
	uint64_t ptr = (uint64_t) &arena->pages;
	uint64_t bindegree = blog(PAGE_SIZE);
	ptr >>= bindegree;
	ptr += 1;
	ptr <<= bindegree;

	arena->pages = (arena_page_t *) ptr;

	for (uint32_t i = 0; i < pages; i++) {
		arena_page_t page = arena->pages[i];
		page.header.fragmentated = CLEAN;
		page.header.total = PAGE_SIZE;
		page.header.used  = 0;
	}

	return arena;
}

arena_node_t * page_alloc(arena_t * arena, size_t bytes) {
	for (uint32_t i = 0; i < arena->npages; i++) {
		arena_page_t page = arena->pages[i];
		if (page.header.total - page.header.used > bytes) {

			arena_node_t * node = (arena_node_t *) &page.data[0] + page.header.used;

			page.header.used    += sizeof(*node) + bytes;

			arena->used_bytes   += bytes;

			return node;
		}
	}

	return NULL;
}

void page_free(arena_t * arena, arena_node_t * node) {
	arena_page_t * page = align2page(node);
	page->header.fragmentated = DIRTY;
	arena->used_bytes -= node->size;
	arena->fragmentated_bytes += node->size;
}

inline arena_page_t * align2page(arena_node_t * node) {
	arena_node_t * _node = (arena_node_t *) node;
	return (arena_page_t *) ((uint32_t) ((void *) node) / PAGE_SIZE);
}

inline uint64_t offsetFromPage (arena_node_t * node) {
	arena_node_t * _node = (arena_node_t *) node;
	arena_page_t * _page = align2page(node);
	return (uint64_t) _node - (uint64_t) _page;
}