#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static inline int is_pow_of_2(uint32_t x) {
	return !(x & (x-1));
}

static uint32_t bsqrt (uint32_t num) {
	uint32_t rv = 0; for (; num >> rv; rv++);
	return (uint32_t) (1 << (rv - 1)) == num ? rv - 1 : rv;
}

enum { Kb = 1 << 10, Mb = 1 << 20, Gb = 1 << 30 };
enum { MIN_CHUNK = 1 * Kb };
enum { FREE, USED, SPLIT };

static void *memory;

struct buddy {
	uint32_t level;
	uint32_t leaves;
	uint32_t used;
	uint8_t  tree[1]; // pseudo-binary tree
};

void * self;
struct buddy * buddy_info;

void buddy_new(size_t size) { // in Kb
	uint32_t alloc_size = 1 << bsqrt(size);
	uint32_t hight = bsqrt(size) - bsqrt(MIN_CHUNK / Kb) + 1;

	uint32_t buddy_size = sizeof(struct buddy) + ((1 << hight) - 1) * sizeof(uint8_t);
	void * ptr = malloc(buddy_size + alloc_size * 1*Kb);
	assert(ptr != NULL);


	self = buddy_info = ptr;
	memory = (void *)((char *) ptr + buddy_size);

	memset(buddy_info, 0, buddy_size);

	buddy_info->used = 0;
	buddy_info->level = hight;
	buddy_info->leaves = (1 << hight) - 1;

#ifdef DEBUG
	fprintf(stderr, "alloc_size = %u\n", alloc_size);
	fprintf(stderr, "min_chunk = %u\n", MIN_CHUNK);
	fprintf(stderr, "hight = %u\n", hight);
	fprintf(stderr, "buddy_size = %u\n", buddy_size);
	fprintf(stderr, "leaves = %u\n", buddy_info->leaves);
	fprintf(stderr, "memory = %lx\n", (long) memory);
#endif
	return;
}

void buddy_destroy(void) {
	free(self);
	return;
}

void buddy_dump(void);

static inline uint32_t _level_from_pos(uint32_t pos) {
	return bsqrt(pos + 2);
}

static inline uint32_t _capasity_from_pos(uint32_t pos) {
	/* Position structure:
		1: 2
		2: 3          4
		3: 5    6     7     8
		4: 9 10 11 12 13 14 15 16
	*/
	uint32_t level = _level_from_pos(pos);
	return (1 << (buddy_info->level - level)) * MIN_CHUNK / Kb;
}

static void * _buddy_ack_memory (size_t kbytes, uint32_t pos) {
	if (pos > buddy_info->leaves) {
		return NULL;
	}

	// Note! capasity mesures in kilobytes
	uint32_t capasity = _capasity_from_pos(pos);

	if (capasity < kbytes) {
		return NULL;
	}

	switch(buddy_info->tree[pos]) {
		case FREE:
			// We must try to split big block and take only one of parts
			if (capasity / 2 >= kbytes) {
				void * ptr = _buddy_ack_memory(kbytes, 2 * pos + 1);
				if (ptr) {
					buddy_info->tree[pos] = SPLIT;
					return ptr;
				}
				ptr = _buddy_ack_memory(kbytes, 2 * pos + 2);
				if (ptr) {
					buddy_info->tree[pos] = SPLIT;
					return ptr;
				}
			}
			// But, it still can be no free blocks. So we allocate hole big block
			buddy_info->tree[pos] = USED;
			buddy_info->used += capasity;

			// Expression bellow allocates capasity of position on current tree-level
			// Note, that memory is a list, not a BTree of course

			uint32_t level  = _level_from_pos(pos);
			uint32_t offset = (pos + 1) % (1 << (level - 1));

		#ifdef DEBUG
				fprintf(stderr, "Pos=%u\n", pos);
				fprintf(stderr, "Offset=%u\n", offset);
		#endif
			return (void *) ((char *) memory + offset * capasity * Kb);

		case SPLIT:
			if (capasity / 2 < kbytes) {
				return NULL;
			}

			char * ptr = _buddy_ack_memory(kbytes, 2 * pos + 1);
			if (!ptr) {
				ptr = _buddy_ack_memory(kbytes, 2 * pos + 2);
				if (!ptr) {
					return NULL;
				}
			}
			return ptr;

		case USED:
			return NULL;

		default:
			fprintf(stderr, "Fatal Buddy Error: UNKNOWN TYPE = %d\n", buddy_info->tree[pos]);
			buddy_dump();
			exit(255);
	}
}

void * buddy_alloc(size_t bytes) {
	uint32_t alloc_size = 1 << bsqrt(bytes / Kb);
	return _buddy_ack_memory(alloc_size, 0);
}

void _buddy_merge(uint32_t pos) {
	for (uint32_t parent = pos / 2;; parent /= 2) {
		if (buddy_info->tree[parent] == SPLIT &&
			buddy_info->tree[2 * parent + 1] == FREE &&
			buddy_info->tree[2 * parent + 2] == FREE) {
			buddy_info->tree[parent] = FREE;
		} else {
			break;
		}
		if (parent == 0) {
			break;
		}
	}
}

void buddy_free(void *ptr) {
	uint64_t offset = (char *) ptr - (char *) memory;
	uint32_t pos = (1 << (buddy_info->level - 1)) + offset / MIN_CHUNK - 1;

#ifdef DEBUG
	fprintf(stderr, "free: offset=%lu\n", offset);
	fprintf(stderr, "free: pos=%u\n", pos);
#endif

	assert(pos < buddy_info->leaves);

	switch(buddy_info->tree[pos]) {
		case USED:
			break;
		case FREE:
			// Find position which used for parent
			for(; pos > 0 && buddy_info->tree[pos] == FREE; pos /= 2);
			break;
		case SPLIT:
			// Find left-child position
			for (; pos < buddy_info->leaves && buddy_info->tree[pos] == SPLIT; pos = pos * 2 + 1);
			break;
		default:
			fprintf(stderr, "Fatal Buddy Error: UNKOWN TYPE = %d\n", buddy_info->tree[pos]);
			buddy_dump();
			exit(255);
	}

	switch (buddy_info->tree[pos]) {
		case FREE: case SPLIT:
			fprintf(stderr, "Double free on %lx offset=%" PRId64 " position=%u\n", (long) ptr, offset, pos);
			exit(255);
		case USED:
			buddy_info->tree[pos] = FREE;
			buddy_info->used -= _capasity_from_pos(pos);
			break;
		default:
			fprintf(stderr, "Fatal Buddy Error: UNKOWN TYPE = %d\n", buddy_info->tree[pos]);
			buddy_dump();
			exit(255);
	}


	_buddy_merge(pos);
	return;
}

void buddy_dump(void) {
	fprintf(stderr, "Level = %u\n", buddy_info->level);

	uint32_t level = 1;
	for (uint32_t pos = 0; pos < buddy_info->leaves; pos++) {
		for (int i = 0; i < 1 << (buddy_info->level + 1 - level); i++) {
			fprintf(stderr, "   ");
		}
		switch(buddy_info->tree[pos]) {
			case SPLIT:
				fprintf(stderr, "SPLT");
				break;
			case FREE:
				fprintf(stderr, "FREE");
				break;
			case USED:
				fprintf(stderr, "USED");
				break;
			default: break;
		}
		for (int i = 0; i < 1 << (buddy_info->level + 1 - level); i++) {
			fprintf(stderr, "   ");
		}
		if (is_pow_of_2(pos + 2)) {
			fprintf(stderr, "\n");
			level++;
		}
	}

	fprintf(stderr, "\n");
	return;
}
