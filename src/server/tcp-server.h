#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <netinet/in.h>
#include <sys/un.h>
#include <ev.h>

struct ev_server;
typedef struct ev_server ev_server;

#include "request.h"

struct ev_server {
	ev_io io;
	int fd;
	int reqid;
	enum socket_type {
		UNIX,
		INET
	} type;
	union {
		struct sockaddr_in socket_in;
		struct sockaddr_un socket_un;
	};
	void (*on_request)(req_t *);
};

ev_server server_init(char * ip_addr, uint16_t port, enum socket_type sock_type);
void server_listen (struct ev_loop *loop, ev_server * server);
void server_close (ev_server * server);

#endif
