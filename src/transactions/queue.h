#ifndef QUEUE_H
#define QUEUE_H

#include "server/message.h"
#include "server/request.h"
#include "lib/heap/heap.h"

#include <pthread.h>
#include <semaphore.h>

typedef struct {
	req_t* ancestor;
	msg_t* msg;
} transaction_t;

typedef struct {
	heap* h;
	sem_t ntrans_sem;
	pthread_mutex_t mutex;
	uint32_t transactions;
} queue_t;

transaction_t* convert_request(req_t* req);

queue_t* init_queue(void);
void destroy_transaction(transaction_t*);
void destroy_queue(queue_t*);

void push_queue(queue_t*, transaction_t*);
transaction_t* pop_queue(queue_t* queue);


#endif
