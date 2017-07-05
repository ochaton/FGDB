#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <netinet/in.h>
#include <sys/un.h>

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
};

typedef struct ev_server ev_server;

#endif
