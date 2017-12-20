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

#include "transactions/queue.h"

arena_t      *arena;
disk_t       *disk;
lru_queue_t  *lru;
wal_logger_t *binary_logger;
hashmap_t     hashmap;
queue_t      *trans_queue;

extern void operation_peek(req_t * req, hashmap_t hashmap);
extern void operation_select(req_t * req, hashmap_t hashmap);
extern void operation_delete(req_t * req, hashmap_t hashmap);
extern void operation_insert(req_t * req, hashmap_t hashmap);
extern void operation_update(req_t * req, hashmap_t hashmap);


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

		lsn_t LSN = write_log(binary_logger, trans);

		switch(trans->msg->cmd) {
			case PEEK:
			{
				operation_peek(trans->ancestor, hashmap);
				break;
			}
			case SELECT:
			{
				operation_select(trans->ancestor, hashmap);
				break;
			}
			case INSERT:
			{
				operation_insert(trans->ancestor, hashmap);
				break;
			}
			case DELETE:
			{
				operation_delete(trans->ancestor, hashmap);
				break;
			}
			case UPDATE:
			{
				operation_update(trans->ancestor, hashmap);
				break;
			}
			default:
			{
				fprintf(stderr, "Unknown operation %d\n", trans->msg->cmd);
				break;
			}
		}
	}

}

int db_start(int argc, char const *argv[]) {
	buddy_new(8192);
	lru = new_lru_queue();
	arena = new_arena(1024);

	config_t config;
	sprintf(config.disk.snap_dir, ".");
	sprintf(config.disk.key_file, "key.key");
	sprintf(config.wal.wal_dir, "wal");

	config.arena.size = 1024;

	disk = init_disk(&config);
	arena->headers = init_headers(disk->nkeys);
	hashmap = hashmap_new();

	uint64_t keys;
	for (keys = 0; keys < disk->nkeys; ++keys) {
		hashmap_key_t key;
		if (-1 == disk_upload_key(disk, &key)) {
			break;
		}

		hashmap_error_t err;
		if (-1 == hashmap_insert_key(hashmap, key.meta, key.key, &err)) {
			fprintf(stderr, "Failed on inserting key: %s. (%s)\n", key.key->ptr, hashmap_error[err]);
			exit(EXIT_FAILURE);
		}
	}

	assert(keys == disk->nkeys);

	// TODO: add some logic to start logger with actual LSNs
	binary_logger = new_wal_logger(0, 0, 1);
}

int start_server() {
	struct ev_loop *loop = ev_default_loop(0);

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
