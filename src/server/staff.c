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

#include <pthread.h> // pthread_sigmask

#include <arpa/inet.h> // inet_aton


#include <ev.h>
#include <tcp-server.h> // struct ev_server

static inline int reuse_addr(int fd);
int unix_socket_init (struct sockaddr_un* socket_un, char* sock_path);
int server_unix_init (struct ev_server * server, char * sock_path, int max_queue);
int inet_socket_init (struct sockaddr_in * socket_in, char * ip_addr_str, uint16_t port);
int server_inet_init (struct ev_server * server, char * ip_addr_str, uint16_t port, int max_queue);

int setnonblock (int fd);
unsigned int staff_random();

/* Implementation: */

int setnonblock(int fd) {
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


int inet_socket_init (struct sockaddr_in * socket_in, char * ip_addr_str, uint16_t port) {

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
	if (-1 == inet_aton((const char *) ip_addr_str, &socket_in->sin_addr)) {
		perror("Server socket inet_aton: ");
		exit(EXIT_FAILURE);
	}

	return fd;
}


int server_inet_init (struct ev_server * server, char * ip_addr_str, uint16_t port, int max_queue) {
	assert(server->type == INET);

	server->fd = inet_socket_init(&server->socket_in, ip_addr_str, port);
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

static int rfd = -1;

unsigned int staff_random() {
	if (rfd == -1) {
		if (-1 == (rfd = open("/dev/random", O_RDONLY))) {
			fprintf(stderr, "Openning /dev/random failed: %s\n", strerror(errno));
			return rand();
		}
	}

	unsigned int rv;
	if (-1 == read(rfd, &rv, sizeof rv)) {
		rv = rand();
	}
	return rv;
}

void hexdump(void * ptr, size_t bytes) {
	assert(bytes < 4096);
	static const char alph[256] = {'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'};
	char * hd_buf = (char *) malloc(1 << 14);
	memset(hd_buf, 0, bytes);
	char * hd_text = (char *) malloc(bytes);
	memset(hd_text, 0, bytes);

	// hexdump goes here:
	int counter = 1;
	int len = 0, txt_len = 0;
	uint8_t * p = (uint8_t *) ptr;
	for (uint8_t * byte = p; byte < p + bytes; byte++, counter++) {
		if (counter % 8) {
			len += sprintf(hd_buf + len, "%02x ", *byte);
			txt_len += sprintf(hd_text + txt_len, "%c", alph[*byte]);
		} else {
			len += sprintf(hd_buf + len, "%02x", *byte);
			txt_len += sprintf(hd_text + txt_len, "%c|\n", alph[*byte]);

			len += sprintf(hd_buf + len, "  |%s", hd_text);
			txt_len = 0;
		}
	}
	fprintf(stderr, "%s\n", hd_buf);
	fprintf(stderr, "%d\n", len);
	free(hd_text);
	free(hd_buf);
	return;
}

void ignore_sigpipe() {
	sigset_t msk;
	sigemptyset(&msk);
	sigaddset(&msk, SIGPIPE);
	if (pthread_sigmask(SIG_BLOCK, &msk, NULL)) {
		fprintf(stderr, "sigmask failed! errno=%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}
