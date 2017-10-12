#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stdio.h>
#include <assert.h>

buf_t * init_buffer(size_t buflen) {
	buf_t * buf = (buf_t *) malloc(buflen + sizeof(buf_t));
	if (buf) {
		buf->free = buf->total = buflen;
		buf->used = 0;
	}
	return buf;
}

void destroy_buffer(buf_t * buf) {
	free(buf);
}

void buffer_push(buf_t * buf, const char *src, size_t bytes) {
	if (!src) return;
	assert(bytes <= buf->free);
	strncpy(buf->start + buf->used, src, bytes);
	buf->free -= bytes;
	buf->used += bytes;
	return;
}

void buffer_free(buf_t * buf) {
	buf->used = 0;
	buf->free = buf->total;
	return;
}
