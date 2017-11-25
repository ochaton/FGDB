#include "lruq.h"

lru_queue_t* new_queue(void) {
	lru_queue_t* lq = malloc(sizeof(lru_queue_t));
	lq->top    = NULL;
	lq->bottom = NULL;
	return lq;
}

void touch_page(lru_queue_t* q, page_header_t* p) {
	p->lru_prev = q->top;
	p->lru_next = NULL;
	if (q->top) q->top->lru_next = p;
	if (!q->bottom) q->bottom = p;
	q->top = p;
}

page_header_t* least_recent_page(lru_queue_t* q) {
	page_header_t* lr = q->bottom;
	while (lr->state == PAGE_PROCESSING) {
		lr = lr->lru_next;
		if (!lr) lr = q->bottom;
	}

	if (lr == q->top) q->top = NULL;

	if (lr == q->bottom) {
		q->bottom = lr->lru_next;
		if (q->bottom) q->bottom->lru_prev = NULL;
		return lr;
	}

	if (lr->lru_next) {
		lr->lru_next->lru_prev = lr->lru_prev;
	}
	if (lr->lru_prev) {
		lr->lru_prev->lru_next = lr->lru_next;
	}

	return lr;
}

void destroy_lru_queue(lru_queue_t* q) {
	free(q);
}

