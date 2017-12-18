#ifndef MESSAGE_H
#define MESSAGE_H

enum msg_command_t {
	PEEK   = 0x1,
	SELECT = 0x2,
	INSERT = 0x3,
	UPDATE = 0x4,
	DELETE = 0x5,
} __attribute__ ((__packed__));

struct msg_t;
typedef struct msg_t msg_t;

#include <stdint.h>
#include "common.h"

typedef struct msg_t {
	enum msg_command_t cmd;
	str_t key, val;
} msg_t;

msg_t * init_message(uint32_t bytes);
void destroy_message(msg_t * msg);
int message_set(str_t * mstr, const char * src, uint32_t size);
int message_command(msg_t *msg, uint32_t num);

extern const char * message_cmd_str[];

#endif // MESSAGE_H
