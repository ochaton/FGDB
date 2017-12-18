#ifndef PROTO_H
#define PROTO_H

struct proto_reply_t;
typedef struct proto_reply_t proto_reply_t;

#include <stdint.h>
#include <msgpack.h>
#include "message.h"

enum proto_reply_error_t {
	PROTO_ERROR_UNKNOWN = 0x0,
	PROTO_ERROR_COMMAND = 0x1,
	PROTO_ERROR_VALUE_REQUIRED = 0x2,
};

enum proto_reply_code_t {
	REPLY_OK    = 0x0,
	REPLY_ERROR = 0x1,
	REPLY_FATAL = 0x2,
};

enum fgdb_code_t {
	CODE_OK       = 0x0,
	KEY_EXISTS    = 0x1,
	KEY_NOT_FOUND = 0x2,
};

typedef struct proto_reply_t {
	// uint32_t seq;
	enum proto_reply_code_t code;
	union {
		enum proto_reply_error_t fatal;
		enum fgdb_code_t err;
		enum msg_command_t cmd;
	};
	str_t val;
} proto_reply_t;

msgpack_sbuffer* serialize_reply(proto_reply_t * reply);

#endif // PROTO_H
