#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t arena_page_id_t;

typedef struct {
	uint16_t size;
	char * ptr;
} str_t;

str_t * new_string(size_t bytes);
void destroy_string(str_t * string);
str_t * string_copy(str_t * src);
str_t * char2string(const char *ptr, size_t bytes);

#define max(a,b) ({ \
	typeof (a) _a = (a); \
	typeof (b) _b = (b); \
	_a > _b ? _a : _b; })

#define min(a,b) ({ \
	typeof (a) _a = (a); \
	typeof (b) _b = (b); \
	_a < _b ? _a : _b; })

#include "arena/disk.h"
#include "arena/meta.h"
#include "memory/hashmap.h"
#include "wal/wal.h"

extern struct arena *arena;
extern struct disk  *disk;
extern struct lru_queue_t  *lru;

struct FGBD_t;
typedef struct FGDB_t FGDB_t;

// TODO: start using this structure instead of numerous "extern struct" in every single file that needs them
struct FGDB_t {
	struct arena        *arena;
	struct disk         *disk;
	struct lru_queue_t  *lru;
	struct wal_logger_t *wal;
};

// extern hashmap_t *hashmap;

#endif // COMMON_H
