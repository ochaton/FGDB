#ifndef FGDB_META_PAGES_H
#define FGDB_META_PAGES_H

struct key_meta_t;
struct page_header_key_t;
struct page_header;

typedef struct page_header_key_t page_header_key_t;
typedef struct page_header page_header_t;

enum { PAGE_SIZE = 4096 };
typedef char arena_page_t[PAGE_SIZE];

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "memory/hashmap.h"
#include "lib/vector/vector.h"
#include "wal/wal.h"

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

	// pLSN - the first LSN that made page DIRTY
	lsn_t pLSN;

	page_id_t page_id;
	arena_page_id_t arena_id;
	enum { PAGE_DIRTY, PAGE_CLEAN, PAGE_PROCESSING  } state:8;
	enum { PAGE_FREE, PAGE_INMEMORY, PAGE_INDISK } location:8;
} page_header_t;

typedef struct vector page_headers_vector_t;

typedef struct arena {
	size_t allocated_pages;
	size_t uploaded_pages;
	page_headers_vector_t * headers;
	arena_page_t * pages;
} arena_t;

typedef struct val_t {
	uint16_t size;
	char ptr[1];
} val_t;

arena_t * new_arena(size_t pages);
void destroy_arena(arena_t * arena);
arena_page_id_t arena_get_next_page(void);
void arena_defragmentate_page(arena_page_id_t page_id, page_header_t * header);

page_headers_vector_t * init_headers(size_t pages);
void destroy_headers(void);
void update_lsn(page_header_t* header, lsn_t LSN);

page_header_t * new_header(void);
void destroy_header(page_header_t * header);

page_header_t * headers_new_page(void);
page_header_t * headers_alloc_page(size_t value_size);
page_header_t * page_value_set(str_t * value, key_meta_t * key);
page_header_t * page_value_get(key_meta_t * key, str_t * retval);
page_header_t * page_value_unset(key_meta_t * key, str_t * value);

page_header_key_t * headers_push_key(page_header_t * header, key_meta_t * key, off_t page_offset);

void snapshot(void);

#endif //FGDB_META_PAGES_H
