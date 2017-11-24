#include "arena/meta.h"
#include "lib/buddy/memory.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

extern arena_t * arena;

arena_t * new_arena(size_t pages) {
	arena_t * arena = malloc(sizeof(arena_t));

	uint32_t bytes = pages * PAGE_SIZE + sizeof(arena_t);
	char * mem_ptr = buddy_alloc(bytes);

	assert(mem_ptr);

	register char * alligned = (char *) (((uint64_t) mem_ptr + PAGE_SIZE) & ~((uint64_t) PAGE_SIZE - 1));
	arena->pages = (arena_page_t *) alligned;

	arena->allocated_pages = pages;
	arena->uploaded_pages = 0;

	return arena;
}

void destroy_arena(arena_t * arena) {
	buddy_free(arena->pages);
	free(arena);
	return;
}

arena_page_id_t arena_get_next_page(void) {
	if (arena->allocated_pages > arena->uploaded_pages) {
		return arena->uploaded_pages++;
	}

	// call lru_cache
}

void arena_page_touch(arena_page_id_t page_id) {
	// Call to LRU:
}

void arena_defragmentate_page(arena_page_id_t page_id, page_header_t * header) {
	// TODO: Lock page in LRU
	arena_page_t * page = &arena->pages[page_id];
	off_t offset = 0;
	for (int block_id = 0; block_id < header->keys->total; block_id++) {
		page_header_key_t * key = VECTOR_GET(header->keys[0], page_header_key_t *, block_id);
		str_t * value = (str_t *) &page[key->offset];
		memmove(&page[key->offset], &page[offset], value->size);

		key->offset = offset;
		offset += value->size;
	}

	header->fragmentated_bytes = 0;
	header->tail_bytes = offset;
	header->state = PAGE_DIRTY;

	memset((char *) page + header->tail_bytes, 0, PAGE_SIZE - header->tail_bytes);
	// TODO: unlock page
}
