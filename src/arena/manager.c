#include "arena/meta.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"

// uint16_t * page_tail_bytes_container;

static page_id_t page_value_alloc(size_t bytes);
static arena_page_id_t heat_page(page_id_t page_id);

void destroy_header(page_header_t * header);

page_headers_vector_t * init_headers(size_t pages) {
	arena->headers = malloc(sizeof(struct vector));
	vector_init(arena->headers, pages);

	assert(arena->headers);
	return arena->headers;
}

void destroy_headers(void) {
	for (size_t i = 0; i < arena->headers->total; i++) {
		destroy_header(arena->headers->items[i]);
	}

	vector_free(arena->headers);
	free(arena->headers);
	return;
}

void destroy_header(page_header_t * header) {
	// Destroy reverse keys:
	for (size_t i = 0; i < header->keys->total; i++) {
		free(header->keys->items[i]);
	}

	vector_free(header->keys);
	free(header);
	return;
}

page_header_t * headers_new_page(void) {
	page_header_t * header = (page_header_t *) malloc(sizeof(page_header_t));
	if (!header) {
		return NULL;
	}
	bzero(header, sizeof(*header));

	header->keys = malloc(sizeof(struct vector));
	vector_init(header->keys, PAGE_HEADER_KEYS_INIT_COUNT);

	vector_add(arena->headers, header);
	return header;
}

page_header_t * headers_alloc_page(size_t value_size) {
	page_header_t * prefered = NULL;
	for (size_t page_id = 0; page_id < arena->headers->total; page_id++) {
		register page_header_t * header;
		if (header->tail_bytes >= value_size) {
			return header;
		} else if (
			((PAGE_SIZE - header->fragmentated_bytes) / header->keys->total >= value_size) &&
			(header->fragmentated_bytes / PAGE_SIZE * header->keys->total >= FGDB_FRAGMENTATION_FACTOR)) {
			prefered = header;
		}
	}

	if (!prefered) {
		return headers_new_page();
	}

	arena_defragmentate_page(prefered->arena_id, prefered);

	// We can be more polite here
	// But after defragmentation memory should be enough for new value
	assert(prefered->tail_bytes == value_size);

	return prefered;
}

int headers_push_key(page_header_t * header, hashmap_key_t * key, off_t page_offset) {
	page_header_key_t * key_reverse = (page_header_key_t *) malloc(sizeof(page_header_key_t));
	if (!key_reverse) {
		return -1;
	}
	key_reverse->key = key;
	key_reverse->offset = page_offset;

	vector_add(header->keys, key_reverse);
	return 0;
}

// // Returns offset in page
// page_header_t * page_value_set(str_t * value, hashmap_key_t * key) {
// 	page_id_t page_id = page_value_alloc(value->size);

// 	page_header_t * header = &arena->headers->items[page_id];
// 	arena_page_t  * page   = &arena->pages[ header->arena_id ];

// 	size_t start_from = PAGE_SIZE - header->tail_bytes;
// 	assert(sizeof(value->size) + value->size <= header->tail_bytes);

// 	memcpy(&value->size, page, sizeof(value->size));
// 	memcpy(value->ptr, page + start_from + sizeof(value->size), value->size);

// 	header->tail_bytes -= value->size;
// 	// header->lsn = get_lsn();

// 	key->page   = page_id;
// 	key->offset = start_from;

// 	return header;
// }

// str_t * page_value_get(page_id_t page_id, off_t offset) {
// 	str_t * retval = malloc(sizeof(str_t));
// 	page_header_t * header = &arena->headers->items[page_id];

// 	if (header->location == PAGE_INDISK) {
// 		header->arena_id = heat_page(page_id);
// 		header->location = PAGE_INMEMORY;
// 	}

// 	if (header->location == PAGE_INMEMORY) {
// 		arena_page_t * page = &arena->pages[ header->arena_id ];

// 		memcpy(page[offset], &retval->size, sizeof(retval->size));
// 		retval->ptr = (char *) malloc(retval->size);

// 		memcpy(page[offset + sizeof(retval->size)], retval->ptr, retval->size);

// 		return retval;
// 	}

// 	return NULL;
// }

// static arena_page_id_t heat_page(page_id_t page_id) {
// 	arena_page_id_t arena_page_id = arena_get_next_page(arena);
// 	disk_upload_page(disk, page_id, arena_page_id);
// 	return arena_page_id;
// }

// static page_id_t page_value_alloc(size_t bytes) {
// 	assert(bytes < PAGE_SIZE);

// 	for (int32_t page_id = 0; page_id < arena->headers->len; page_id++) {
// 		page_header_t * header = &arena->headers->items[page_id];

// 		if (header->tail_bytes >= bytes) {
// 			if (header->location == PAGE_INMEMORY) {
// 				return page_id;
// 			}

// 			if (header->location == PAGE_INDISK) {
// 				header->arena_id = heat_page(page_id);
// 				header->location = PAGE_INMEMORY;
// 				return page_id;
// 			}
// 		}

// 		if (header->location == PAGE_FREE) {
// 			page_id_t arena_page_id = arena_get_next_page(arena);
// 			disk_new_page(disk);
// 			header->arena_id = arena_page_id;
// 			header->location = PAGE_INMEMORY;

// 			return page_id;
// 		}
// 	}

// 	// Call defragmentator:
// }
