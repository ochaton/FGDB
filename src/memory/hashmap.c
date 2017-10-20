#include "hashmap.h"
#include "common.h"

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

hashmap_t * hashmap_new(size_t max_keys);
void hashmap_delete(hashmap_t * hmap);
hashmap_key_t * hashmap_insert_key(hashmap_t * hmap, hashmap_key_t * new_key, hashmap_error_t *err);
hashmap_key_t * hashmap_lookup_key(hashmap_t * hmap, str_t * key);
hashmap_key_t * hashmap_delete_key(hashmap_t * hmap, str_t * key);

static hashmap_key_t * hashmap_find(hashmap_t * hmap, str_t * key);

hashmap_t * hashmap_new(size_t max_keys) {
	hashmap_t * hmap = (hashmap_t *) calloc(max_keys, sizeof(hashmap_key_t));
	if (!hmap) {
		if (errno) fprintf(stderr, "Calloc: %s\n", strerror(errno));
		return NULL;
	}
	hmap->size = max_keys;
	hmap->used = 0;
	return hmap;
}

void hashmap_delete(hashmap_t * hmap) {
	free(hmap);
}

hashmap_key_t * hashmap_insert_key(hashmap_t * hmap, hashmap_key_t * new_key, hashmap_error_t *err) {
	*err = HASHMAP_SUCCESS;
	if (hmap->used == hmap->size) {
		*err = HASHMAP_NO_SPACE_LEFT;
		return NULL;
	}

	hashmap_key_t * found = hashmap_find(hmap, &new_key->key);
	if (found->location != FREE) {
		*err = HASHMAP_KEY_FOUND;
		return NULL;
	}

	memcpy(found, new_key, sizeof(*new_key));
	return found;
}

// Linear search (just to write code fast)
hashmap_key_t * hashmap_lookup_key(hashmap_t * hmap, str_t * key) {
	hashmap_key_t * found = hashmap_find(hmap, key);
	if (!found || found->location == FREE) {
		return NULL;
	}

	return found;
}

// Delete key if exists
hashmap_key_t * hashmap_delete_key(hashmap_t * hmap, str_t * key) {
	hashmap_key_t * found = hashmap_lookup_key(hmap, key);
	if (found->location == FREE) {
		return NULL;
	}

	found->location = FREE;
	found->fragmentated = DIRTY;

	return found;
}

static hashmap_key_t * hashmap_find(hashmap_t * hmap, str_t * key) {
	hashmap_key_t * found = &hmap->top[0];
	hashmap_key_t * end = &hmap->top[hmap->size];

	hashmap_key_t * free = NULL;

	for (; found < end; found += sizeof(*found)) {
		if (!free && (found->location == FREE)) {
			free = found;
			continue;
		}
		if (found->key.size != key->size) {
			continue;
		}
		if (!memcmp(found->key.ptr, key->ptr, key->size)) {
			return found;
		}
	}
	return free;
}

const char * hashmap_error[] = {
	"HASHMAP_SUCCESS",
	"HASHMAP_KEY_FOUND",
	"HASHMAP_NO_SPACE_LEFT",
};