#ifndef ARENA_H
#define ARENA_H

#include "common.h"
#include "memory/hashmap.h"

typedef struct{
	hashmap_key_t * rev_key; // reverse pointer to key
	size_t size;
	char ptr[1];
} arena_node_t;

#endif // ARENA_H