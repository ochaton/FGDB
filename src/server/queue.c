#include "queue.h"
#include "heap.h"
#include "message.h"
#include <unistd.h>
#include <stdlib.h>

transaction_t* convert_request(req_t* req) {
	transaction_t* ret_trans = malloc(sizeof(transaction_t));
	ret_trans->ancestor = req;

	msg_t* msg = (msg_t*) malloc(sizeof(msg_t));
	msg->cmd = req->msg->cmd;
	message_set(&(msg->key), req->msg->key.ptr, req->msg->key.size);
	message_set(&(msg->val), req->msg->val.ptr, req->msg->val.size);
	ret_trans->msg = msg;
	return ret_trans;
}

queue_t* init_queue(void) {
	heap* h = (heap*) malloc(sizeof(heap));
	heap_create(h, 0, NULL);
	queue_t* ptr = (queue_t*) malloc(sizeof(queue_t));
	ptr->h = h;
	return ptr;
}

void destroy_transaction(transaction_t* transaction) {
	if (transaction) {
		destroy_message(transaction->msg);
		free(transaction);
	}
	return;
}

void destroy_queue(queue_t* queue) {
	uint32_t* u;
	transaction_t* t;
	while (heap_delmin(queue->h, (void **) &u, (void **) &t)) {
		destroy_transaction(t);
	}
	return;
}

void push_queue(queue_t* queue, transaction_t* transaction) {
	heap_insert(
		queue->h,
		&(transaction->msg->cmd),
		transaction
	);
}

transaction_t* pop_queue(queue_t* queue) {
	uint32_t* u;
	transaction_t* t;
	if (heap_delmin(queue->h, (void**) &u, (void**) &t)) {
		return t;
	}
	return NULL;
}

