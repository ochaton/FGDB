#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include "common.h"

typedef struct {
	str_t key;
	void * page;
	uint32_t offset;
	enum { FREE, INMEMORY, OUTMEMORY } location;
	enum { CLEAN, DIRTY } fragmentated;
} hashmap_key_t;

typedef struct {
	size_t size, used;
	hashmap_key_t top[1];
} hashmap_t;

typedef enum {
	HASHMAP_SUCCESS,
	HASHMAP_KEY_FOUND,
	HASHMAP_NO_SPACE_LEFT,
} hashmap_error_t;

hashmap_t * hashmap_new(size_t max_keys);
void hashmap_delete(hashmap_t * hmap);
hashmap_key_t * hashmap_insert_key(hashmap_t * hmap, hashmap_key_t * new_key, hashmap_error_t *err);
hashmap_key_t * hashmap_lookup_key(hashmap_t * hmap, str_t * key);
hashmap_key_t * hashmap_delete_key(hashmap_t * hmap, str_t * key);

extern const char * hashmap_error[];

#endif // HASHMAP_H
