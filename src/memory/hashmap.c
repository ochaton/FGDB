#include "hashmap.h"
#include "common.h"

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

hashmap_t hashmap_new(void);
void hashmap_delete(hashmap_t hashmap);

int hashmap_insert_key(hashmap_t hmap, hashmap_key_t * new_key, hashmap_error_t *err);
hashmap_key_t * hashmap_lookup_key(hashmap_t hmap, str_t * key, hashmap_error_t *err);
hashmap_key_t * hashmap_delete_key(hashmap_t hmap, str_t * key, hashmap_error_t *err);

hashmap_t hashmap_new();
void hashmap_delete(hashmap_t hashmap);

extern hashmap_t hashmap;

hashmap_t hashmap_new(void) {
	assert(hash_new_node(&hashmap, 0) == 0);
	return hashmap;
}

void hashmap_delete(hashmap_t hashmap) {
	assert(hash_erase(hashmap) == 1);
}

int hashmap_insert_key(hashmap_t hmap, hashmap_key_t * new_key, hashmap_error_t *err) {
	*err = HASHMAP_SUCCESS;
	avlnode_ptr found = hash_search(hmap, *new_key->key);

	if (found) {
		*err = HASHMAP_KEY_FOUND;
		return -1;
	}

	int result = hash_insert(hmap, *new_key->key, new_key);
	if (result != 1) {
		*err = HASHMAP_INTERNAL_ERROR;
		return -1;
	}

	return 0;
}

hashmap_key_t * hashmap_lookup_key(hashmap_t hmap, str_t * key, hashmap_error_t *err) {
	*err = HASHMAP_SUCCESS;
	avlnode_ptr found = hash_search(hmap, *key);

	if (found) {
		return (hashmap_key_t *) found->page;
	}

	return NULL;
}

hashmap_key_t * hashmap_delete_key(hashmap_t hmap, str_t * key, hashmap_error_t *err) {
	*err = HASHMAP_SUCCESS;
	avlnode_ptr found = hash_search(hmap, *key);

	if (!found) {
		return NULL;
	}

	hashmap_key_t * rv = calloc(1, sizeof(hashmap_key_t));
	rv->key = key;
	rv->header_key_id = ((hashmap_key_t *) found->page)->header_key_id;
	rv->page = ((hashmap_key_t *) found->page)->page;

	int result = hash_delete(hmap, *key);
	assert(result == 1);

	return rv;
}

const char * hashmap_error[] = {
	"HASHMAP_SUCCESS",
	"HASHMAP_KEY_FOUND",
	"HASHMAP_NO_SPACE_LEFT",
	"HASHMAP_INTERNAL_ERROR",
};
