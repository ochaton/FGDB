// #include "memory/hashmap.h"
// #include "arena/disk.h"

#include "lib/buddy/memory.h"
#include "common.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include <stdlib.h>

#include "lib/vector/vector.h"
#include "arena/meta.h"
arena_t   * arena;
disk_t    * disk;
// hashmap_t * hashmap;

void start() {
	buddy_new(8192); // 8Mb
	arena = new_arena(1024);
	disk = init_disk("/home/ochaton/vcs/tcp-ev-server/db.snap");
	arena->headers = init_headers(1024);
}

void finish() {
	destroy_headers();
	destroy_arena(arena);
	destroy_disk(disk);

	buddy_destroy();
}

int main(int argc, char const *argv[]) {
	start();

	// Add page:
	// page_header_t * page_header = headers_alloc_page(100);

	str_t val = { 3, "val" };
	hashmap_key_t key = { { 3, "key" }, -1, NULL };
	page_header_t * header = page_value_set(&val, &key);

	str_t * retval = page_value_get(&key);
	fprintf(stderr, "%u\n", retval->size);

	free(retval);

	finish();
	return 0;
}
