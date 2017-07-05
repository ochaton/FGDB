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
#include "staff.h"


ev_server server;

static void accept_cb (EV_P_ ev_io *w, int revents) {
	puts("unix stream socket has become readable\n");

	// since ev_io is the first member,
	// watcher `w` has the address of the 
	// start of the ev_server struct
	// AND THAT IS FUCKING GENIOUS!!!
	struct ev_server * server_ = (struct ev_server *) w;

	int client_fd = accept(server_->fd, NULL, NULL);
	if (-1 == client_fd) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			fprintf(stderr, "accept() failed errno=%i (%s)\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "accepted a client\n");
		if (-1 == send(client_fd, "ololo\n", 7, 0)) {
			fprintf(stderr, "send() failed errno=%i (%s)\n", errno, strerror(errno));
		}
		shutdown(client_fd, 2);
		close(client_fd);
	}
}


static void not_blocked (EV_P_ ev_periodic *w, int revents) {
	fprintf(stderr, "Not_blocked!\n");
}

int main (int argc, char const *argv[]) {
	enum socket_type type = INET;
	server.type = type;

	server_inet_init(&server, 2016, 10);
	fprintf(stderr, "Server inited!\n");

	struct ev_periodic every_few_seconds;
	EV_P  = ev_default_loop(0);

	ev_periodic_init(&every_few_seconds, not_blocked, 0, 5, 0);
	ev_periodic_start(EV_A_ &every_few_seconds);

	ev_io_init(&server.io, accept_cb, server.fd, EV_READ);
	ev_io_start(EV_A_ &server.io);

	fprintf(stderr, "tcp-socket starting...\n");
	ev_loop(EV_A_ 0);

	// This point is only ever reached if the loop is manually exited
	close(server.fd);
	return EXIT_SUCCESS;
}