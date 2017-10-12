#ifndef MESSAE_H
#define MESSAE_H

#include <stdint.h>

typedef struct message msg_t;
typedef struct message_str str_t;

struct message_str {
	uint32_t size;
	char * ptr;
};

struct message {
	enum msg_command { GET, PUT, UPDATE, DELETE, PEEK } cmd;
	str_t key;
	str_t val; // Can be nil
};

msg_t * init_message(uint32_t bytes);
void destroy_message(msg_t * msg);
int message_set(str_t * mstr, const char * src, uint32_t size);
int message_command(msg_t *msg, const char * src, uint32_t size);


#endif // MESSAE_H
