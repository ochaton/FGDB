#include "message.h"
#include <stdlib.h>
#include <string.h>

msg_t * init_message(uint32_t bytes) {
	msg_t * msg = (msg_t *) malloc(sizeof(msg_t) + bytes);
	return msg;
}

void destroy_message(msg_t * msg) {
	free(msg);
}

int message_set(str_t * mstr, const char * src, uint32_t size) {
	if (!size) {
		mstr->size = 0;
		mstr->ptr  = NULL;
		return 0;
	}
	strncpy(mstr->ptr, src, size);
	mstr->size = size;
	return 0;
}

int message_command(msg_t *msg, const char * src, uint32_t size) {
	char cmd[10];
	strncpy(cmd, src, size);
	if (!strncmp(cmd, "GET", 3)) {
		msg->cmd = GET;
	} else if (!strncmp(cmd, "PUT", 3)) {
		msg->cmd = PUT;
	} else if (!strncmp(cmd, "UPDATE", 6)) {
		msg->cmd = UPDATE;
	} else if (!strncmp(cmd, "DELETE", 6)) {
		msg->cmd = DELETE;
	} else if (!strncmp(cmd, "PEEK", 4)) {
		msg->cmd = PEEK;
	} else {
		return -1;
	}
	return 0;
}
