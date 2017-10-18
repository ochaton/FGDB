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

// msgpack configure:
#define MSGPACK_ZONE_CHUNK_SIZE 8192

// libev:
#include <ev.h>

// project-staff:
#include "tcp-server.h" // ev_server
#include "request.h" // client request
#include "proto.h" // protocol


static void not_blocked (EV_P_ ev_periodic *w, int revents) {
	fprintf(stderr, "Not_blocked!\n");
}

void on_request (req_t *req) {
	req->log->info(req->log, "Starting processing request");
	req->log->info(req->log, "Cmd `%s` { key = `%s` }", message_cmd_str[req->msg->cmd], req->msg->key.ptr);

	proto_reply_t reply;
	reply.code = REPLY_OK;
	reply.cmd  = req->msg->cmd;

	const char reply_msg[] = "0123456789";
	reply.val.size = sizeof(reply_msg);
	reply.val.ptr  = (char *) &reply_msg[0];

	request_reply(req, &reply);
}

int main (int argc, char const *argv[]) {


	// struct ev_periodic every_few_seconds;
	struct ev_loop *loop = ev_default_loop(0);

	// ev_periodic_init(&every_few_seconds, not_blocked, 0, 5, 0);
	// ev_periodic_start(loop, &every_few_seconds);

	ev_server server = server_init("0.0.0.0", 2016, INET);
	server.on_request = on_request;
	// fprintf(stderr, "Server inited!\n");
	// fprintf(stderr, "tcp-socket starting...\n");

	server_listen(loop, &server);

	ev_loop(loop, 0);

	// This point is only ever reached if the loop is manually exited
	server_close(&server);
	return EXIT_SUCCESS;
}