// default headers:
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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
#include "tcp-server.h" // ev_server
#include "request.h" // client request
#include "proto.h" // protocol

#include "memory.h"

#include "hashmap.h"

hashmap_t * hashmap;

static void not_blocked (EV_P_ ev_periodic *w, int revents) {
	fprintf(stderr, "Not_blocked!\n");
}

extern void operation_peek(req_t * req, hashmap_t * hashmap);
extern void operation_select(req_t * req, hashmap_t * hashmap);
extern void operation_delete(req_t * req, hashmap_t * hashmap);
extern void operation_insert(req_t * req, hashmap_t * hashmap);
extern void operation_update(req_t * req, hashmap_t * hashmap);


void on_request (req_t *req) {
	req->log->info(req->log, "Starting processing request");
	req->log->info(req->log, "Cmd `%s` { key = `%s` }", message_cmd_str[req->msg->cmd], req->msg->key.ptr);

	switch(req->msg->cmd) {
		case PEEK:
		{
			operation_peek(req, hashmap);
			return;
		}
		case SELECT:
		{
			operation_select(req, hashmap);
			return;
		}
		case INSERT:
		{
			operation_insert(req, hashmap);
			return;
		}
		case DELETE:
		{
			operation_delete(req, hashmap);
			return;
		}
		case UPDATE:
		{
			operation_update(req, hashmap);
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

	ev_server server = server_init("0.0.0.0", 2016, INET);
	server.on_request = on_request;
	server_listen(loop, &server);
	ev_loop(loop, 0);

	// This point is only ever reached if the loop is manually exited
	server_close(&server);
	return EXIT_SUCCESS;
}

void init_buddy(size_t kilobytes) {
	buddy_new(kilobytes);
}

int main(int argc, char const *argv[]) {
	int status;
	status = init_hashmap(1024);
	if (status) {
		return status;
	}
	init_buddy(1024);
	start_server();
}
