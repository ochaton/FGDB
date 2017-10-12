#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <string.h>

typedef struct buf {
	char * start;
	size_t total, used, free;
} buf_t;

buf_t * init_buffer(size_t buflen);
void destroy_buffer(buf_t * buf);
void buffer_push(buf_t * buf, const char *src, size_t bytes);
void buffer_free(buf_t * buf);

#endif // BUFFER_H
