#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

// Public methods:
void buddy_new(size_t size);
void buddy_destroy();

void * buddy_alloc(size_t bytes);
void buddy_free(void *ptr);

void buddy_dump(void);

#endif
