#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <string.h>

typedef struct buf {
	size_t total, used;
	char *start, *end;
	char ptr[1];
} buf_t;

buf_t * init_buffer(size_t buflen);
void destroy_buffer(buf_t * buf);
void buffer_free(buf_t * buf);

int buffer_push(buf_t * buf, const char *src, size_t bytes);
int buffer_pop(buf_t * buf, char * dest, size_t bytes);
int buffer_shift(buf_t * buf, char * dest, size_t bytes);
int buffer_unshift(buf_t * buf, const char *src, size_t bytes);

#endif // BUFFER_H
