#ifndef FGDB_META_PAGES_H
#define FGDB_META_PAGES_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "memory/hashmap.h"
#include "lib/vector/vector.h"

typedef struct page_header_key_t {
	uint16_t offset;
	struct key_meta_t * key_meta_ptr;
} page_header_key_t;

enum { PAGE_HEADER_KEYS_INIT_COUNT = 4 };

enum {
	// If page can keep more than FGDB_FRAGMENTATION_FACTOR values
	// of average size of values on this page
	// then we should defragmentate it
	FGDB_FRAGMENTATION_FACTOR = 4,
};

typedef struct page_header {
	uint64_t lsn;
	uint16_t fragmentated_bytes;
	uint16_t tail_bytes;

	struct vector *keys;
	struct page_header *lru_next, *lru_prev;

	page_id_t page_id;
	arena_page_id_t arena_id;
	enum { PAGE_DIRTY, PAGE_CLEAN, PAGE_PROCESSING  } state:8;
	enum { PAGE_FREE, PAGE_INMEMORY, PAGE_INDISK } location:8;
} page_header_t;

enum { PAGE_SIZE = 4096 };
typedef char arena_page_t[PAGE_SIZE];

typedef struct vector page_headers_vector_t;

typedef struct arena {
	size_t allocated_pages;
	size_t uploaded_pages;
	page_headers_vector_t * headers;
	arena_page_t * pages;
} arena_t;

arena_t * new_arena(size_t pages);
void destroy_arena(arena_t * arena);
arena_page_id_t arena_get_next_page(void);
void arena_defragmentate_page(arena_page_id_t page_id, page_header_t * header);

typedef struct key_meta_t key_meta_t;

page_headers_vector_t * init_headers(size_t pages);
void destroy_headers(void);
page_header_t * headers_new_page(void);
page_header_t * headers_alloc_page(size_t value_size);
page_header_t * page_value_set(str_t * value, key_meta_t * key);
page_header_t * page_value_get(key_meta_t * key, str_t * retval);
page_header_t * page_value_unset(key_meta_t * key, str_t * value);


#endif //FGDB_META_PAGES_H
