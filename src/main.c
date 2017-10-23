// default headers:
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

// network headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

// pthreads:
#include <pthread.h>

// libev:
#include <ev.h>

// project-staff:
#include "server/tcp-server.h" // ev_server
#include "server/request.h" // client request
#include "server/proto.h" // protocol
#include "server/staff.h"

#include "arena/arena.h"
#include "memory/hashmap.h"
#include "lib/buddy/memory.h"

#include "transactions/queue.h"

arena_t * arena;
hashmap_t * hashmap;
queue_t * trans_queue;
pthread_rwlock_t hashmap_lock;

extern void operation_peek(req_t * req, hashmap_t * hashmap, arena_t * arena);
extern void operation_select(req_t * req, hashmap_t * hashmap, arena_t * arena);
extern void operation_delete(req_t * req, hashmap_t * hashmap, arena_t * arena);
extern void operation_insert(req_t * req, hashmap_t * hashmap, arena_t * arena);
extern void operation_update(req_t * req, hashmap_t * hashmap, arena_t * arena);


void on_request (req_t *req) {
	req->log->info(req->log, "Starting processing request");
	req->log->info(req->log, "Cmd `%s` { key = `%s` }", message_cmd_str[req->msg->cmd], req->msg->key.ptr);

	switch(req->msg->cmd) {
		case PEEK:
		{
			operation_peek(req, hashmap, arena);
			break;
		}
		case SELECT:
		{
			operation_select(req, hashmap, arena);
			break;
		}
		case INSERT:
		{
			operation_insert(req, hashmap, arena);
			break;
		}
		case DELETE:
		{
			operation_delete(req, hashmap, arena);
			break;
		}
		case UPDATE:
		{
			operation_update(req, hashmap, arena);
			break;
		}
	}

	// switch(req->msg->cmd) {
	// 	case PEEK: case SELECT: case INSERT: case DELETE: case UPDATE:
	// 	{
	// 		transaction_t * trans = convert_request(req);
	// 		if (!trans) {
	// 			proto_reply_t reply;
	// 			req->log->error(req->log, "Transaction not created errno=%s", strerror(errno));
	// 			reply.code = REPLY_FATAL;
	// 			reply.err  = PROTO_ERROR_UNKNOWN;

	// 			request_reply(req, &reply);
	// 			return;
	// 		}

	// 		// push_queue(trans_queue, trans);
	// 		return;
	// 	}
	// 	default:
	// 	{
	// 		proto_reply_t reply;
	// 		req->log->error(req->log, "Command %d not found", req->msg->cmd);
	// 		reply.code = REPLY_FATAL;
	// 		reply.err  = PROTO_ERROR_COMMAND;

	// 		request_reply(req, &reply);
	// 		return;
	// 	}
	// }
}

static void idle_cb(EV_P_ ev_periodic *w, int revents) {
	// fprintf(stderr, "Not blocked\n");
}

void * transaction_queue_worker (void * args) {

	pthread_detach(pthread_self());
	ignore_sigpipe();

	fprintf(stderr, "Queue Worker started\n");

	while (1) {

		transaction_t * trans = pop_queue(trans_queue, 0);
		if (!trans) {
			continue;
		}

		switch(trans->msg->cmd) {
			case PEEK:
			{
				operation_peek(trans->ancestor, hashmap, arena);
				break;
			}
			case SELECT:
			{
				operation_select(trans->ancestor, hashmap, arena);
				break;
			}
			case INSERT:
			{
				operation_insert(trans->ancestor, hashmap, arena);
				break;
			}
			case DELETE:
			{
				operation_delete(trans->ancestor, hashmap, arena);
				break;
			}
			case UPDATE:
			{
				operation_update(trans->ancestor, hashmap, arena);
				break;
			}
		}
	}

}

int init_hashmap (size_t max_keys) {
	hashmap = hashmap_new(max_keys);
	if (!hashmap) {
		fprintf(stderr, "FATAL: Hashmap not reated\n");
		return errno;
	}
	return 0;
}

int start_server() {
	struct ev_loop *loop = ev_default_loop(0);

	// ev_idle idle_watcher;
	// ev_idle_init(&idle_watcher, idle_cb);
	// ev_idle_start(loop, &idle_watcher);

	struct ev_periodic every_few_seconds;
	ev_periodic_init(&every_few_seconds, idle_cb, 0, 0.01, 0);
	ev_periodic_start(EV_A_ &every_few_seconds);

	ev_server server = server_init("0.0.0.0", 2016, INET);
	server.on_request = on_request;
	server_listen(loop, &server);
	ev_loop(loop, 0);

	// This point is only ever reached if the loop is manually exited
	server_close(&server);
	return EXIT_SUCCESS;
}

int main(int argc, char const *argv[]) {
	int status;
	status = init_hashmap(1024);
	if (status) {
		return status;
	}

	uint32_t killobytes = 1 << 10; // 1Mb
	buddy_new(killobytes);

	arena = arena_create(killobytes >> 3);

	if (NULL == (trans_queue = init_queue())) {
		fprintf(stderr, "Queue not allocated errno=%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// pthread_t queue_worker;
	// if (pthread_create(&queue_worker, NULL, transaction_queue_worker, NULL)) {
	// 	fprintf(stderr, "Thread not created\n");
	// 	exit(EXIT_FAILURE);
	// }

	// if (pthread_rwlock_init(&hashmap_lock, NULL)) {
	// 	fprintf(stderr, "Creation of rwlock failed\n");
	// 	exit(EXIT_FAILURE);
	// }

	start_server();
}
