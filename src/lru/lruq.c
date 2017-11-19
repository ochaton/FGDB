#include "lruq.h"

lru_queue_t* new_queue(void) {
	lru_queue_t* lq = malloc(sizeof(lru_queue_t));
	lq->top    = NULL;
	lq->bottom = NULL;
	return lq;
}

void heat_page(lru_queue_t* q, page_header_t* p) {
	p->prev = q->top;
	p->next = NULL;
	q->top->next = p;
	q->top = p;
}

page_header_t* least_recent_page(lru_queue_t* q) {
	page_header_t* lr = q->bottom;
	q->bottom = lr->next;
	q->bottom->prev = NULL;
	return lr;
}

void destroy_lru_queue(lru_queue_t* q) {
	free(q);
}
