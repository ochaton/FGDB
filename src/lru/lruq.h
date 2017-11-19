#ifndef LRUQ_H
#define LRUQ_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "common.h"
#include "arena/arena.h"

// 1:1 matching index with arena pages
typedef struct {
	page_header_t* top;
	page_header_t* bottom;
} lru_queue_t;

lru_queue_t* new_queue(void);

void heat_page(lru_queue_t* q, page_header_t* p);

page_header_t* least_recent_page(lru_queue_t* q);

void destroy_lru_queue(lru_queue_t* lq);

#endif
