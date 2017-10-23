#include "arena.h"
#include "lib/buddy/memory.h"
#include <assert.h>
#include <stddef.h>


arena_t * arena_create(uint32_t pages) {
	uint32_t bytes = pages * PAGE_SIZE + sizeof(arena_t);
	char * mem_ptr = buddy_alloc(bytes);

	assert(mem_ptr);

	mem_ptr = (char *) (((uint64_t) mem_ptr + PAGE_SIZE) & ~(PAGE_SIZE - 1));

	arena_t * arena = (arena_t *) mem_ptr;

	arena->allocated_bytes = pages * sizeof(arena_page_t);
	arena->used_bytes      = 0;

	arena->fragmentated_bytes = 0;
	arena->npages = pages;

	for (uint32_t i = 0; i < arena->npages; i++) {
		arena_page_t * page = &arena->pages[i];
		page->header.end = (char *) &page->header + sizeof(page->header);
		// page->header.lru_lsn = 0;
		page->header.used = 0;
		page->header.fragmentated = 0;
		page->header.tail = PAGE_SIZE - sizeof(page->header);
		page->header.nodes = 0;
		page->header.lock = UNLOCKED_PAGE;
	}

	return arena;
}

arena_node_t * page_node_alloc(arena_t * arena, size_t bytes) {

	bytes += sizeof(arena_node_t); // overhead

	for (uint32_t i = 0; i < arena->npages; i++) {
		arena_page_t * page = &arena->pages[i];
		if (page->header.tail > bytes) {

			arena_node_t * node = (arena_node_t *) page->header.end;
			node->state = USED_NODE;

			page->header.end  += bytes;
			page->header.used += bytes;
			page->header.tail -= bytes;

			page->header.nodes++;

			arena->used_bytes += bytes;

			return node;
		}
	}

	return NULL;
}

void page_node_free(arena_t * arena, arena_node_t * node) {
	arena_page_t * page = align2page(node);

	page->header.nodes--;

	register uint32_t add_fragmentated = 0;

	// If this is last taken node in page:
	if (&node->ptr[0] + node->size == page->header.end) {

		// Move end of page back at the beginning on node
		page->header.end = (char *) node;
		node->state = FREE_NODE;
	} else {
		page->header.fragmentated += add_fragmentated = sizeof(*node) + node->size;
		node->state = DIRTY_NODE;
		node->rev_key = NULL;
	}

	arena->used_bytes -= node->size;
	arena->fragmentated_bytes += add_fragmentated;
}

inline arena_page_t * align2page(arena_node_t * node) {
	arena_node_t * _node = (arena_node_t *) node;
	return (arena_page_t *) ((~(PAGE_SIZE - 1) & (uint64_t) _node) + offsetof(arena_t, pages));
}

inline uint64_t offsetFromPage (arena_node_t * node) {
	arena_node_t * _node = (arena_node_t *) node;
	arena_page_t * _page = align2page(node);
	return (uint64_t) _node - (uint64_t) _page;
}