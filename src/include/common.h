#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint32_t size;
	char * ptr;
} str_t;

#define max(a,b) ({ \
	typeof (a) _a = (a); \
	typeof (b) _b = (b); \
	_a > _b ? _a : _b; })

#define min(a,b) ({ \
	typeof (a) _a = (a); \
	typeof (b) _b = (b); \
	_a < _b ? _a : _b; })

#endif // COMMON_H