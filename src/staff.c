#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h> // inet_aton


#include <ev.h>
#include <tcp-server.h> // struct ev_server

static inline int setnonblock (int fd);
static inline int reuse_addr(int fd);
int unix_socket_init (struct sockaddr_un* socket_un, char* sock_path);
int server_unix_init (struct ev_server * server, char * sock_path, int max_queue);
int inet_socket_init (struct sockaddr_in * socket_in, uint16_t port);
int server_inet_init (struct ev_server * server, uint16_t port, int max_queue);


static inline int setnonblock(int fd) {
	int flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags);
}

static inline int reuse_addr(int fd) {
	int one = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (int *) &one, sizeof(one));	
}


int unix_socket_init (struct sockaddr_un * socket_un, char * sock_path) {
	unlink(sock_path);

	// Setup a unix socket listener.
	int fd = socket(AF_UNIX, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == fd) {
		perror("Socket Create: ");
		exit(EXIT_FAILURE);
	}

	// Set it non-blocking
	if (-1 == setnonblock(fd)) {
		perror("Server socket Nonblock: ");
		exit(EXIT_FAILURE);
	}

	// Set it as unix socket
	socket_un->sun_family = AF_UNIX;
	strcpy(socket_un->sun_path, sock_path);

	return fd;
}


int server_unix_init (struct ev_server * server, char * sock_path, int max_queue) {
	assert(server->type == UNIX);

	server->fd = unix_socket_init(&server->socket_un, sock_path);
	size_t socket_len = sizeof(server->socket_un.sun_family) + strlen(server->socket_un.sun_path);

	if (-1 == bind(server->fd, (struct sockaddr*) &server->socket_un, socket_len)) {
		perror("Server bind: ");
		exit(EXIT_FAILURE);
	}

	if (-1 == listen(server->fd, max_queue)) {
	  perror("Server listen: ");
	  exit(EXIT_FAILURE);
	}

	return 0;
}


int inet_socket_init (struct sockaddr_in * socket_in, uint16_t port) {

	// Setup a inet socket listener.
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == fd) {
		perror("Socket Create: ");
		exit(EXIT_FAILURE);
	}

	// Set it non-blocking
	if (-1 == setnonblock(fd)) {
		perror("Server socket Nonblock: ");
		exit(EXIT_FAILURE);
	}

	// Reuse_Addr
	if (-1 == reuse_addr(fd)) {
		perror("Server reuse_addr: ");
		exit(EXIT_FAILURE);
	}

	socket_in->sin_family = AF_INET;
	socket_in->sin_port = htons(port);
	if (-1 == inet_aton("0.0.0.0", &socket_in->sin_addr)) {
		perror("Server socket inet_aton: ");
		exit(EXIT_FAILURE);
	}

	return fd;
}


int server_inet_init (struct ev_server * server, uint16_t port, int max_queue) {
	assert(server->type == INET);

	server->fd = inet_socket_init(&server->socket_in, port);
	if (-1 == bind(server->fd, (struct sockaddr *) &server->socket_in, sizeof server->socket_in)) {
		perror("Server bind: ");
		exit(EXIT_FAILURE);
	}

	if (-1 == listen(server->fd, max_queue)) {
		perror("Server listen: ");
		exit(EXIT_FAILURE);
	}

	return 0;
}








