#include "arena/disk.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

extern arena_t * arena;


disk_t * init_disk(char * path) {
	disk_t * disk = malloc(sizeof(disk_t));
	assert(disk);

	if (-1 == (disk->fd = open(path, O_RDWR | O_CREAT))) {
		fprintf(stderr, "Error open snapshot-file: %s\n", strerror(errno));
		exit(errno);
	}

	disk->pages = lseek(disk->fd, 0, SEEK_END) / PAGE_SIZE;
	lseek(disk->fd, 0, SEEK_SET);

	disk->arena_start = 0;

	return disk;
}

void destroy_disk(disk_t * disk) {
	close(disk->fd);
	free(disk);
}

void disk_upload_page(disk_t *disk, uint32_t disk_page_idx, uint32_t arena_idx) {
	off_t page_pos = disk->arena_start + disk_page_idx * PAGE_SIZE;

	lseek(disk->fd, page_pos, SEEK_SET);

	int bytes, to_read = PAGE_SIZE, readed = 0;
	while (to_read) {
		bytes = read(disk->fd, &arena->pages[arena_idx] + readed, to_read);
		if (-1 == bytes) {
			if (errno == EAGAIN || errno == EINTR) {
			} else {
				fprintf(stderr, "Error on reading from disk: %s\n", strerror(errno));
			}
		} else {
			to_read -= bytes;
			readed  += bytes;
		}
	}
	return;
}

void disk_dump_page(uint32_t page_idx, uint32_t arena_idx) {
	off_t page_pos = disk->arena_start + page_idx * PAGE_SIZE;

	lseek(disk->fd, page_pos, SEEK_SET);
	int bytes, to_write = PAGE_SIZE, written = 0;
	while(to_write) {
		bytes = write(disk->fd, &arena->pages[arena_idx] + written, to_write);
		if (-1 == bytes) {
			if (errno == EAGAIN || errno == EINTR) {
			} else {
				fprintf(stderr, "Error on writing to disk: %s\n", strerror(errno));
			}
		} else {
			to_write -= bytes;
			written  += bytes;
		}
	}
	return;
}

void disk_new_page(disk_t * disk) {
	disk->pages++;
}
