#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "lib/heap/heap.h"
#include "transactions/queue.h"
#include "server/request.h"

const int TEST_AMOUNT = 10;


void init_dummy(req_t * dummy, uint32_t n) {
	char k[10] = {};
	char v[15] = {};

	snprintf(&k[0], sizeof("key_%04d"), "key_%04d", n);
	snprintf(&v[0], sizeof("request_%04d"), "request_%04d", n);

	msg_t* msg = (msg_t*) malloc(sizeof(msg_t));
	dummy->fd = 0;
	msg->key.ptr = &k[0];
	msg->val.ptr = &v[0];
	msg->cmd = 1 + rand() % 5;
	dummy->msg = msg;
	return;
}

int main(int argc, char const *argv[]) {
	queue_t* queue = init_queue();

	req_t * dummy_req = (req_t *) calloc(sizeof(req_t), TEST_AMOUNT);

	for (uint32_t i = 0; i <= TEST_AMOUNT; i++) {
		init_dummy(&dummy_req[i], i);
		transaction_t* t = convert_request(&dummy_req[i]);
		push_queue(queue, t);
	}
	uint32_t curr_prior = 0;
	transaction_t* cur_t;
	while (cur_t = pop_queue(queue, 1)) {
		printf("TRANSACTION ");
		write(0, cur_t->msg->key.ptr, cur_t->msg->key.size);
		printf(" => ");
		write(0, cur_t->msg->val.ptr, cur_t->msg->val.size);
		printf(" (priority %d)\n", cur_t->msg->cmd);

		if (cur_t->msg->cmd < curr_prior) {
			printf(
				"FAILURE: last prior was %d and current is %d\n",
				curr_prior,
				cur_t->msg->cmd
			);
		}
		destroy_transaction(cur_t);
	}

	for (uint32_t i = 0; i < sizeof(dummy_req); i++) {
		free(dummy_req[i].msg);
	}

	free(dummy_req);
	destroy_queue(queue);
	return 0;
}
