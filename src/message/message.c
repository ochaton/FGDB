#include "message.h"
#include <stdlib.h>
#include <string.h>

msg_t * init_message(uint32_t bytes) {
	msg_t * msg = (msg_t *) malloc(sizeof(msg_t));
	msg->key.ptr = msg->val.ptr = NULL;
	return msg;
}

void destroy_message(msg_t * msg) {
	if (msg->key.ptr) free(msg->key.ptr);
	if (msg->val.ptr) free(msg->val.ptr);
	free(msg);
}

int message_set(str_t * mstr, const char * src, uint32_t size) {
	if (!size) {
		mstr->size = 0;
		mstr->ptr = NULL;
		return 0;
	}
	if (mstr->ptr) {
		free(mstr->ptr);
	}
	mstr->ptr = (char *) malloc(size + 1);
	strncpy(mstr->ptr, src, size);
	mstr->ptr[size] = 0;
	mstr->size = size;
	return 0;
}

int message_command(msg_t *msg, uint32_t num) {
	msg->cmd = (enum msg_command_t) num;
	return 0;
}

const char * message_cmd_str[] = {
	"UNKNOWN",
	"PEEK",
	"SELECT",
	"INSERT",
	"UPDATE",
	"DELETE",
};
