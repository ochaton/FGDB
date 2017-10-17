#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stdio.h>

buf_t * init_buffer(size_t buflen) {
	buf_t * buf = (buf_t *) malloc(buflen + sizeof(buf_t));
	if (buf) {
		buf->total = buflen;
		buf->end = buf->start = &buf->ptr[0];
		buf->used = 0;
	}
	return buf;
}

void destroy_buffer(buf_t * buf) {
	free(buf);
}

int buffer_push(buf_t * buf, const char *src, size_t bytes) {
	if (buf->end + bytes > &buf->ptr[buf->total]) {
		return -1;
	}
	memcpy(buf->end, src, bytes);
	buf->end  += bytes;
	buf->used += bytes;
	return 0;
}

int buffer_pop(buf_t * buf, char * dest, size_t bytes) {
	if (buf->end - bytes < buf->start) {
		return -1;
	}

	memcpy(dest, buf->end - bytes, bytes);
	buf->end  -= bytes;
	buf->used -= bytes;
	return 0;
}

int buffer_shift(buf_t * buf, char * dest, size_t bytes) {
	if (buf->start + bytes > buf->end) {
		return -1;
	}

	memcpy(dest, buf->start, bytes);
	buf->start += bytes;
	buf->used  -= bytes;
	return 0;
}

int buffer_unshift(buf_t * buf, const char *src, size_t bytes) {
	if (&buf->ptr[0] + bytes > buf->start) {
		return -1;
	}

	memcpy(&buf->ptr[0], src, bytes);
	buf->start -= bytes;
	buf->used  += bytes;
	return 0;
}

void buffer_free(buf_t * buf) {
	buf->used = 0;
	buf->start = buf->end = &buf->ptr[0];
	return;
}
