#ifndef QUEUE_H
#define QUEUE_H

#include "message.h"
#include "request.h"
#include "heap.h"

typedef struct {
	req_t* ancestor;
	msg_t* msg;
} transaction_t;

typedef struct {
	heap* h;
} queue_t;

transaction_t* convert_request(req_t* req);

queue_t* init_queue(void);
void destroy_transaction(transaction_t*);
void destroy_queue(queue_t*);

void push_queue(queue_t*, transaction_t*);
transaction_t* pop_queue(queue_t* queue);


#endif
