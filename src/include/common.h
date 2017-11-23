#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t page_id_t;
typedef uint64_t arena_page_id_t;

typedef struct {
	uint16_t size;
	char * ptr;
} str_t;

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

extern struct arena *arena;
extern struct disk  *disk;
// extern hashmap_t *hashmap;

#endif // COMMON_H