#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include "common.h"
#include "lib/hashmap/HashMap.h"

typedef struct key_meta_t {
	// str_t * key;                               /* This is a pointer to key stored inside heavy struct HashMap */
	page_header_key_id_t header_key_id;        /* Pointer to offset inside page (stored inside headers of arena-pages) */
	page_id_t page;                            /* Page identificator. Storing this we can find the page, where stored value */
} key_meta_t;

typedef struct HashMap * hashmap_t;

hashmap_t hashmap_new(void);
void hashmap_delete(hashmap_t hashmap);

typedef enum {
	HASHMAP_SUCCESS,
	HASHMAP_KEY_FOUND,
	HASHMAP_NO_SPACE_LEFT,
	HASHMAP_INTERNAL_ERROR,
} hashmap_error_t;

int hashmap_insert_key(hashmap_t hmap, key_meta_t * key_meta, str_t * key, hashmap_error_t *err);
key_meta_t * hashmap_lookup_key(hashmap_t hmap, str_t * key, hashmap_error_t *err);
key_meta_t * hashmap_delete_key(hashmap_t hmap, str_t * key, hashmap_error_t *err);

extern const char * hashmap_error[];

#endif // HASHMAP_H
