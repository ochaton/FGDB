#include "common.h"

str_t * new_string(size_t bytes) {
	assert(bytes < 1 << (8 * sizeof(uint16_t)));

	str_t * ret = (str_t *) malloc(sizeof(str_t));
	ret->ptr = (char *) malloc(bytes);
	ret->size = bytes;
	return ret;
}

void destroy_string(str_t * string) {
	if (!string) {
		return;
	}
	if (string->ptr && string->size > 0) {
		free(string->ptr);
	}
	free(string);
}

str_t * string_copy(str_t * src) {
	assert(src);
	if (!src) {
		return NULL;
	}

	str_t * copy = new_string(src->size);
	memcpy(copy->ptr, src->ptr, src->size);

	return copy;
}

str_t * char2string(const char *ptr, size_t bytes) {
	str_t * ret = new_string(bytes);
	if (ret) {
		memcpy(ret->ptr, ptr, ret->size);
	}
	return ret;
}
