#ifndef REQUEST_H
#define REQUEST_H

struct request;
typedef struct request req_t;

#include "server/log.h"
#include "server/tcp-server.h"
#include "server/message.h"
#include "server/buffer.h"
#include "server/proto.h"

struct request {
	ev_io io;
	int fd;

	enum { INIT, READ, WRITE, PARSE, SERVICE } state;
	ev_server * server;

	struct log * log;
	buf_t * buf;
	msg_t * msg;
};

void init_request(int fd, ev_server * server);
void destroy_request(req_t * req);
void request_reply(req_t * req, proto_reply_t * reply);

#endif // REQUEST_H
