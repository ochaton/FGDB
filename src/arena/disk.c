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

#include "wal/wal.h"

disk_t * init_disk(config_t * config) {
	disk_t * disk = calloc(1, sizeof(disk_t));
	assert(disk);

	char path[2048];
	snprintf(path, sizeof(path), "%s/db.snap", config->disk.snap_dir);

	if (-1 == (disk->vfd = open(path, O_RDWR | O_CREAT, 0644))) {
		fprintf(stderr, "Error while openning value snapshot-file: %s\n", strerror(errno));
		exit(errno);
	}

	char keypath[2048];
	snprintf(keypath, sizeof(keypath), "%s/%s", config->disk.snap_dir, config->disk.key_file);

	if (-1 == (disk->kfd = open(keypath, O_RDWR | O_CREAT, 0644))) {
		fprintf(stderr, "Error while openning keys snapshot-file: %s\n", strerror(errno));
		exit(errno);
	}

	disk->arena_start = sizeof(disk->lsn);

	if (read(disk->vfd, &disk->lsn, sizeof(disk->lsn))) {
		disk->pages = (lseek(disk->vfd, 0, SEEK_END) - disk->arena_start) / PAGE_SIZE;
	} else {
		fprintf(stderr, "Reading from empty snapshot\n");
		disk->lsn = 0;
	}

	lseek(disk->vfd, disk->arena_start, SEEK_SET);

	disk->keys_start = sizeof(disk->nkeys);

	if (read(disk->kfd, &disk->nkeys, sizeof(disk->nkeys))) {
	} else {
		fprintf(stderr, "Starting from empty DB\n");
		disk->nkeys = 0;
	}

	lseek(disk->kfd, disk->keys_start, SEEK_SET);

	return disk;
}

void destroy_disk(disk_t * disk) {

	lseek(disk->kfd, 0, SEEK_SET);
	write(disk->kfd, &disk->keys_dumped, sizeof(disk->keys_dumped));

	lseek(disk->vfd, 0, SEEK_SET);
	write(disk->vfd, &disk->lsn, sizeof(disk->lsn));

	close(disk->vfd);
	close(disk->kfd);
	free(disk);
}

void disk_upload_page(disk_t *disk, page_id_t disk_page_idx, arena_page_id_t arena_idx) {
	off_t page_pos = disk->arena_start + disk_page_idx * PAGE_SIZE;

	lseek(disk->vfd, page_pos, SEEK_SET);

	int bytes, to_read = PAGE_SIZE, readed = 0;
	while (to_read) {
		bytes = read(disk->vfd, &arena->pages[arena_idx] + readed, to_read);
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

// TODO: do not forget about dumping pLSNs when we are dumping page headers
void disk_dump_page(page_id_t page_idx, arena_page_id_t arena_idx) {
	off_t page_pos = disk->arena_start + page_idx * PAGE_SIZE;

	page_header_t * header = VECTOR_GET(arena->headers[0], page_header_t*, page_idx);
	disk->lsn = max(disk->lsn, header->pLSN);

	lseek(disk->vfd, page_pos, SEEK_SET);
	int bytes, to_write = PAGE_SIZE, written = 0;
	arena_page_t * page = &arena->pages[arena_idx];

	char * start = (char *) page;
	while(to_write) {
		bytes = write(disk->vfd, start + written, to_write);
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

/**
 * length of record (8 bytes)
 * page_id (8 bytes)
 * offset inside page (2 bytes)
 * length of key (2 bytes)
 * key (...)
 **/

int disk_dump_keys(disk_t * disk) {

	for (page_id_t page_id = 0; page_id < arena->headers->total; page_id++) {
		page_header_t * header = VECTOR_GET(arena->headers[0], page_header_t*, page_id);

		for (size_t key_id = 0; key_id < header->keys->total; key_id++) {
			page_header_key_t * page_key = VECTOR_GET(header->keys[0], page_header_key_t*, key_id);

			key_meta_t * meta = page_key->key_meta_ptr;
			size_t length = sizeof(meta->page)
				+ sizeof(page_key->offset)
				+ sizeof(meta->weak_key->size)
				+ meta->weak_key->size;

			char * buffer;
			char * p = buffer = malloc(length + sizeof(length));

			memcpy(p, &length, sizeof(length));                             p += sizeof(length);
			memcpy(p, &meta->page, sizeof(meta->page));                     p += sizeof(meta->page);
			memcpy(p, &page_key->offset, sizeof(page_key->offset));         p += sizeof(page_key->offset);
			memcpy(p, &meta->weak_key->size, sizeof(meta->weak_key->size)); p += sizeof(meta->weak_key->size);
			memcpy(p, meta->weak_key->ptr, meta->weak_key->size);           p += meta->weak_key->size;

			length += sizeof(length);
			write(disk->kfd, buffer, length);
			free(buffer);

			disk->keys_dumped++;
		}
	}
}

int disk_upload_key(disk_t * disk, hashmap_key_t * ret) {

	size_t length;
	size_t readed = read(disk->kfd, &length, sizeof(length));

	if (!readed) {
		return -1;
	}

	length -= sizeof(length);

	binary_key_t * record = malloc(length);
	record->length = length;

	read(disk->kfd, record + sizeof(record->length), record->length);

	ret->key = (str_t *) malloc(sizeof(str_t));
	ret->key->size = record->key.size;
	ret->key->ptr = (char *) malloc(ret->key->size);

	memcpy(ret->key->ptr, &record->key.ptr[0], record->key.size);
	ret->offset = record->offset;
	ret->page_id = record->page_id;

	free(record);
	return 0;
}

void disk_new_page(disk_t * disk) {
	disk->pages++;
}
