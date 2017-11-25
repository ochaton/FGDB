#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stddef.h>

#define VECTOR_INIT_CAPACITY 4
#define VECTOR_CAPACITY_RESIZE 1.5

#define VECTOR_INIT(vec) vector vec; vector_init(&vec, 0)
#define VECTOR_ADD(vec, item) vector_add(&vec, (void *) item)
#define VECTOR_SET(vec, id, item) vector_set(&vec, (int) id, (void *) item)
#define VECTOR_GET(vec, type, id) (type) vector_get(&vec, (int) id)
#define VECTOR_DELETE(vec, id) vector_delete(&vec, (int) id)
#define VECTOR_TOTAL(vec) vector_total(&vec)
#define VECTOR_FREE(vec) vector_free(&vec)

typedef struct vector {
    void **items;
    int capacity;
    int total;
} vector;

void vector_init(vector *, size_t capacity);
int vector_total(vector *);
static void vector_resize(vector *, int);
void vector_add(vector *, void *);
void vector_set(vector *, int, void *);
void *vector_get(vector *, int);
void vector_delete(vector *, int);
void vector_free(vector *);

#endif
