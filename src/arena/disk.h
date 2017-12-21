#ifndef FGDB_DISK_H
#define FGDB_DISK_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "wal/wal.h"

typedef struct disk {
	off_t arena_start;
	off_t keys_start;

	uint64_t pages;
	uint64_t nkeys;

	uint64_t keys_dumped;

	lsn_t lsn;

	int vfd;
	int kfd;
} disk_t;

#include "common.h"
#include "memory/hashmap.h"
#include "arena/meta.h"
#include "lib/hashmap/AVLNode.h"
#include "config.h"

disk_t * init_disk(config_t * config);
void destroy_disk(disk_t * disk);
void disk_upload_page(disk_t *disk, page_id_t disk_page_idx, arena_page_id_t arena_idx);
void disk_dump_page(page_id_t page_idx, arena_page_id_t arena_idx);
void disk_new_page(disk_t * disk);
int disk_dump_keys(disk_t * disk);

int disk_upload_header(disk_t * disk, page_header_t * header);
int disk_upload_key(disk_t * disk, hashmap_key_t * key);


#endif
