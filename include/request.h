#ifndef REQUEST_H
#define REQUEST_H

#include "log.h"
#include "tcp-server.h"
#include "message.h"
#include "buffer.h"

typedef struct request req_t;
typedef struct ev_server ev_server;

struct request {
	ev_io io;
	int fd;

	enum { READ, WRITE, PARSE, SERVICE } state;

	ev_server * server;

	struct log * log;
	buf_t * buf;
	msg_t * msg;
};

void init_request(int fd, ev_server * server);
void destroy_request(req_t * req);

#endif // REQUEST_H
