#ifndef STAFF_H
#define STAFF_H

#include "tcp-server.h"

int setnonblock (int fd);
int unix_socket_init (struct sockaddr_un* socket_un, char* sock_path);
int server_unix_init (struct ev_server * server, char * sock_path, int max_queue);
int inet_socket_init (struct sockaddr_in * socket_in, char * ip_addr_str, uint16_t port);
int server_inet_init (struct ev_server * server, char * ip_addr_str, uint16_t port, int max_queue);
int setnonblock (int fd);
unsigned staff_random();
void hexdump(void * p, size_t bytes);
#endif
