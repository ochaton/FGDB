#include "queue.h"
#include "lib/heap/heap.h"
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>


transaction_t* convert_request(req_t* req) {
	transaction_t* ret_trans = malloc(sizeof(transaction_t));
	if (!ret_trans) {
		return NULL;
	}
	ret_trans->ancestor = req;
	ret_trans->msg = req->msg;
	return ret_trans;
}

queue_t* init_queue(void) {
	heap* h = (heap*) malloc(sizeof(heap));
	if (!h) {
		return NULL;
	}
	heap_create(h, 0, NULL);
	queue_t* queue = (queue_t*) malloc(sizeof(queue_t));
	if (!queue) {
		return NULL;
	}
	if (pthread_mutex_init(&queue->mutex, NULL)) {
		fprintf(stderr, "Queue Mutex init failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	sem_init(&queue->ntrans_sem, 0, 0); // Set value to 0
	queue->h = h;
	return queue;
}

void destroy_transaction(transaction_t* transaction) {
	if (transaction) {
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
	pthread_mutex_destroy(&queue->mutex);
	sem_destroy(&queue->ntrans_sem);
	heap_destroy(queue->h);
	free(queue);
	return;
}

void push_queue(queue_t* queue, transaction_t* transaction) {
	// Try to lock mutex
	pthread_mutex_lock(&queue->mutex);

	// Insert new transaction
		heap_insert(
			queue->h,
			&transaction->msg->cmd,
			transaction
		);

	// Increase counter of transactions
		sem_post(&queue->ntrans_sem);

	// Unlock semaphore to make pop available
	pthread_mutex_unlock(&queue->mutex);
}

transaction_t* pop_queue(queue_t* queue, uint8_t force) {
	uint32_t* u;
	transaction_t* t;

	if (!force) {
		// Take semaphore to know that we actually have some transactions
		sem_wait(&queue->ntrans_sem);
	}

	// Lock mutex, to avoid pushing to queue
	pthread_mutex_lock(&queue->mutex);

	// Pop transaction from queue
		if (heap_delmin(queue->h, (void**) &u, (void**) &t)) {
		} else {
			t = NULL;
		}

	// Unlock mutex
	pthread_mutex_unlock(&queue->mutex);

	/*  Q: Why we don't post semaphore back?
		A: Because we use semaphore as a counter of transactions in our queue.
		We could not use semaphores at all, but then we would have 100% CPU Usage
	*/

	return t;
}

