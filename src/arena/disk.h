#ifndef FGDB_DISK_H
#define FGDB_DISK_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct disk {
	char * path;

	off_t arena_start;
	uint64_t pages;

	int fd;
	// Mutex here:
} disk_t;

#include "common.h"

disk_t * init_disk(char * path);
void destroy_disk(disk_t * disk);
void disk_upload_page(disk_t *disk, page_id_t disk_page_idx, arena_page_id_t arena_idx);
void disk_dump_page(page_id_t page_idx, arena_page_id_t arena_idx);
void disk_new_page(disk_t * disk);


#endif
