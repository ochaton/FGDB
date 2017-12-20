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

#include "lib/buddy/memory.h"
#include "memory/hashmap.h"
#include "lru/lruq.h"
#include "operations/operations.h"

#include "transactions/queue.h"

arena_t      *arena;
disk_t       *disk;
lru_queue_t  *lru;
wal_logger_t *wal_logger;
hashmap_t     hashmap;
queue_t      *trans_queue;

void on_request (req_t *req) {
	req->log->info(req->log, "Starting processing request");
	req->log->info(req->log, "Cmd `%s` { key = `%s` }", message_cmd_str[req->msg->cmd], req->msg->key.ptr);

	switch(req->msg->cmd) {
		case PEEK: case SELECT: case INSERT: case DELETE: case UPDATE:
		{
			transaction_t * trans = convert_request(req);
			if (!trans) {
				proto_reply_t reply;
				req->log->error(req->log, "Transaction not created errno=%s", strerror(errno));
				reply.code = REPLY_FATAL;
				reply.err  = PROTO_ERROR_UNKNOWN;

				request_reply(req, &reply);
				return;
			}

			push_queue(trans_queue, trans);
			return;
		}
		default:
		{
			proto_reply_t reply;
			req->log->error(req->log, "Command %d not found", req->msg->cmd);
			reply.code = REPLY_FATAL;
			reply.err  = PROTO_ERROR_COMMAND;

			request_reply(req, &reply);
			return;
		}
	}
}

void * transaction_queue_worker (void * args) {
	pthread_detach(pthread_self());
	ignore_sigpipe();

	fprintf(stderr, "Queue Worker started\n");

	while (1) {

		transaction_t * trans = pop_queue(trans_queue, 0);
		req_t* req = trans->ancestor;
		if (!trans) {
			continue;
		}

		proto_reply_t* db_reply;

		switch(trans->msg->cmd) {
			case PEEK:
			{
				db_reply = operation_peek(trans, hashmap);
				break;
			}
			case SELECT:
			{
				db_reply = operation_select(trans, hashmap);
				break;
			}
			case INSERT:
			{
				db_reply = operation_insert(trans, hashmap);
				break;
			}
			case DELETE:
			{
				db_reply = operation_delete(trans, hashmap);
				break;
			}
			case UPDATE:
			{
				db_reply = operation_update(trans, hashmap);
				break;
			}
			default:
			{
				fprintf(stderr, "Unknown operation %d\n", trans->msg->cmd);
				db_reply = malloc(sizeof(proto_reply_t));
				db_reply->code = REPLY_ERROR;
				db_reply->err  = OPERATION_UNKNOWN;
				break;
			}
		}
		request_reply(trans->ancestor, db_reply);
		free(db_reply);
	}

}

int db_start(int argc, char const *argv[]) {
	buddy_new(8192);
	lru = new_lru_queue();
	arena = new_arena(1024);
	disk = init_disk("db.snap");
	// TODO: add some logic to start logger with actual LSNs
	wal_logger = new_wal_logger(0, 0, 1);
	arena->headers = init_headers(1024);
	hashmap = hashmap_new();
}

static void async_cb (EV_P_ ev_async *w, int revents) {
	// just used for the side effects
}

int start_server() {
	struct ev_loop *loop = ev_default_loop(0);


	ev_server server = server_init("0.0.0.0", 2016, INET);
	server.on_request = on_request;
	server_listen(loop, &server);
	ev_async_init(&server.trigger, async_cb);
	ev_async_start(loop, &server.trigger);
	ev_loop(loop, 0);

	// This point is only ever reached if the loop is manually exited
	server_close(&server);
	return EXIT_SUCCESS;
}

int main(int argc, char const *argv[]) {
	db_start(argc, argv);

	if (NULL == (trans_queue = init_queue())) {
		fprintf(stderr, "Queue not allocated errno=%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	pthread_t queue_worker;
	if (pthread_create(&queue_worker, NULL, transaction_queue_worker, NULL)) {
		fprintf(stderr, "Thread not created\n");
		exit(EXIT_FAILURE);
	}

	start_server();
}
