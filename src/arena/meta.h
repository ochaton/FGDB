#ifndef FGDB_META_PAGES_H
#define FGDB_META_PAGES_H

typedef struct {
	uint64_t lsn;
	uint64_t fragmentated_bytes;

	enum { PAGE_DIRTY, PAGE_CLEAN } state:8;
	enum { PAGE_FREE, PAGE_INMEMORY, PAGE_INDISK } location:8;
} page_header_t;


extern page_header_t page_headers_container[];
extern uint16_t page_tail_bytes_container[];
extern lru_cache_node_t lru_nodes_container[];


#endif //FGDB_META_PAGES_H
