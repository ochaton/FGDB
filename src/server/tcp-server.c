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

// libev:
#include <ev.h>

// project-staff:
#include "tcp-server.h" // ev_server
#include "request.h"    // client request
#include "staff.h"

static void accept_cb (EV_P_ ev_io *w, int revents) {
	// since ev_io is the first member,
	// watcher `w` has the address of the
	// start of the ev_server struct
	// AND THAT IS FUCKING GENIOUS!!!
	struct ev_server * server = (struct ev_server *) w;

	int client_fd = accept(server->fd, NULL, NULL);
	if (-1 == client_fd) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			fprintf(stderr, "accept() failed errno=%i (%s)\n", errno, strerror(errno));
			return;
		}
	} else {
		init_request(client_fd, server);
	}
}

ev_server server_init(char * ip_addr, uint16_t port, enum socket_type sock_type) {
	ev_server new_server;
	new_server.type = sock_type;

	server_inet_init(&new_server, ip_addr, port, 10);
	ev_io_init(&new_server.io, accept_cb, new_server.fd, EV_READ);

	return new_server;
}

void server_listen (struct ev_loop *loop, ev_server * server) {
	server->loop = loop;
	ev_io_start(loop, &server->io);
	return;
}

void server_close (ev_server * server) {
	close(server->fd);
	return;
}
