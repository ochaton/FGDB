#ifndef HASHMAP_H
#define HASHMAP_H

struct hashmap_key_t;
struct key_meta_t;

typedef struct hashmap_key_t hashmap_key_t;
typedef struct key_meta_t key_meta_t;

#include <stddef.h>
#include <stdint.h>

typedef uint16_t page_header_key_id_t;
typedef uint64_t page_id_t;

#include "common.h"
#include "lib/hashmap/HashMap.h"

typedef struct key_meta_t {
	str_t * weak_key;
	page_header_key_id_t header_key_id;        /* Pointer to offset inside page (stored inside headers of arena-pages) */
	page_id_t page;                            /* Page identificator. Storing this we can find the page, where stored value */
} key_meta_t;

typedef struct HashMap * hashmap_t;

typedef struct hashmap_key_t {
	str_t * key;
	uint16_t offset;
	page_id_t page_id;
} hashmap_key_t;

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
