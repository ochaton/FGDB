#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

typedef struct message msg_t;
typedef struct message_str str_t;

struct message_str {
	uint32_t size;
	char * ptr;
};

struct message {
	enum msg_command { GET = 0x1, PUT = 0x2, UPDATE = 0x3, DELETE = 0x4, PEEK = 0x5 } cmd;
	str_t key, val;
};

msg_t * init_message(uint32_t bytes);
void destroy_message(msg_t * msg);
int message_set(str_t * mstr, const char * src, uint32_t size);
int message_command(msg_t *msg, uint32_t num);

extern const char * message_cmd_str[];

#endif // MESSAGE_H
