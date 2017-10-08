#include "request.h"
#include "staff.h"
#include "log.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <msgpack.h>

#include <ev.h>

enum { MAX_MESSAGE_SIZE = 10*1<<10 };

void parse_request(EV_P_ ev_io *w, int revents);

void init_request(int fd, ev_server * server) {
	req_t * req = (req_t *) malloc(sizeof(req_t));
	req->log = init_log();

	req->log->info(req->log, "Started request on fd=%d", fd);

	if (-1 == setnonblock(req->fd = fd)) {
		req->log->crit(req->log, "Setnonblock failed with %s\n", strerror(errno));
		destroy_request(req);
		return;
	}

	req->log->debug(req->log, "setnonblock successfull");

	req->server = server;
	req->state  = READ;
	req->buf = NULL;
	req->msg = NULL;

	ev_io_init(&req->io, parse_request, req->fd, EV_READ);
	ev_io_start(EV_DEFAULT_ &req->io);

	return;
}

void destroy_request(req_t * req) {
	if (!req) return;
	if (req->buf) destroy_buffer(req->buf);
	req->log->debug(req->log, "Buffer destroyed");

	if (req->msg) destroy_message(req->msg);
	req->log->debug(req->log, "Message destroyed");

	ev_io_stop(EV_DEFAULT_ &req->io);
	close(req->fd);

	if (req->log) destroy_log(req->log);
	free(req);
	return;
}

static int parse_command(req_t * req, msgpack_object * obj);
static int parse_data(req_t * req, msgpack_object * obj);

void parse_request(EV_P_ ev_io *w, int revents) {
	req_t * req = (req_t *) w;

	req->log->debug(req->log, "Parsing request");

	ssize_t bytes;
	char buffer[1024];
	char *pbuf = &buffer[0];
	size_t to_read = sizeof(buffer);

	if (req->buf) {
		pbuf = req->buf->start + req->buf->used;
		to_read = req->buf->free;
	}

	if (-1 == (bytes = read(req->fd, pbuf, to_read))) {
		if (errno == EAGAIN || errno == EINTR) {
			return;
		} else {
			req->log->crit(req->log, "Error on read from socket: %s", strerror(errno));
			return destroy_request(req);
		}
	}

	if (!bytes) {
		req->state = PARSE;
	}

	switch (req->state) {
		case READ:
		{
			ssize_t msg_len;
			sscanf(pbuf, "%d", &msg_len);

			if (msg_len > MAX_MESSAGE_SIZE) {
				req->log->crit(req->log, "Message is too long %d > %d", msg_len, MAX_MESSAGE_SIZE);
				destroy_request(req);
				return;
			}

			if (NULL == (req->buf = init_buffer(msg_len))) {
				req->log->crit(req->log, "Buffer not allocated!");
				destroy_request(req);
				return;
			}

			push2buffer(req->buf, &buffer[0], bytes);
			break;
		}
		case PARSE:
		{
			msgpack_unpacked unpacked;
			msgpack_unpacked_init(&unpacked);

			size_t offset = 0;

			msgpack_unpack_return ret = msgpack_unpack_next(&unpacked, req->buf->start, req->buf->used, &offset);
			if (MSGPACK_UNPACK_PARSE_ERROR == ret) {
				req->log->error(req->log, "Message-Pack error: MSGPACK_UNPACK_PARSE_ERROR");
				return destroy_request(req);
			}

			msgpack_object obj = unpacked.data;
			if (obj.type != MSGPACK_OBJECT_ARRAY) {
				req->log->error(req->log, "Server expected %d but got %d", MSGPACK_OBJECT_ARRAY, obj.type);
				return destroy_request(req);
			}

			msgpack_object_array arr = obj.via.array;
			if (arr.size != 2) {
				req->log->error(req->log, "Array must include 2 items, but got %u", arr.size);
				return destroy_request(req);
			}

			msgpack_object data = (&arr)->ptr[1];
			if (-1 == parse_data(req, &data)) {
				return destroy_request(req);
			}

			msgpack_object command = (&arr)->ptr[0];
			if (-1 == parse_command(req, &command)) {
				req->log->error(req->log, "Unknown command");
				return destroy_request(req);
			}

			req->log->debug(req->log, "Unpack finished successfully");

			msgpack_unpacked_destroy(&unpacked);
			buffer_free(req->buf);

			req->state = SERVICE;

			ev_io_stop(EV_DEFAULT_ &req->io);
			// server->on_request(req);

			return;
		}
		default:
		{
			req->log->error(req->log, "Fatal. Should not be here");
			destroy_request(req);
		}
	}
	return;
}

static int parse_command(req_t * req, msgpack_object * obj) {
	if (obj->type != MSGPACK_OBJECT_STR ||
		obj->type != MSGPACK_OBJECT_BIN
	) {
		req->log->error(req->log, "Command expected as string or bin, got %d", obj->type);
		return -1;
	}

	return message_command(req->msg, obj->via.str.ptr, obj->via.str.size);
}

static int parse_data(req_t * req, msgpack_object * obj) {
	switch (obj->type) {
		case MSGPACK_OBJECT_BIN:
		case MSGPACK_OBJECT_STR:
		{

			// Only here I know real size of message:
			uint32_t len = obj->via.str.size;
			if (NULL == (req->msg = init_message(len))) {
				return -1;
			}

			// In message is only key
			if (-1 == message_set(&req->msg->key, obj->via.str.ptr, len)) return -1;
			if (-1 == message_set(&req->msg->val, NULL, 0)) return -1;

			return 0;
		}
		case MSGPACK_OBJECT_MAP:
		{
			if (!obj->via.map.size) {
				req->log->error(req->log, "Expected non-empty map");
				return -1;
			}

			msgpack_object * key = &(&obj->via.map)->ptr[0].key;
			msgpack_object * val = &(&obj->via.map)->ptr[0].val;

			if (key->type != MSGPACK_OBJECT_BIN ||
				key->type != MSGPACK_OBJECT_STR
			) {
				req->log->error(req->log, "Unexpected type %d in hash-key", key->type);
				return -1;
			}
			if (val->type != MSGPACK_OBJECT_BIN ||
				val->type != MSGPACK_OBJECT_STR
			) {
				req->log->error(req->log, "Unexpected type %d in hash-val", val->type);
				return -1;
			}

			uint32_t len = key->via.str.size + val->via.str.size;
			if (NULL == (req->msg = init_message(len))) {
				return -1;
			}

			if (-1 == message_set(&req->msg->key, key->via.str.ptr, key->via.str.size)) return -1;
			if (-1 == message_set(&req->msg->val, val->via.str.ptr, val->via.str.size)) return -1;
			return 0;
		}
		default:
		{
			req->log->error(req->log, "Unexpected type %s", obj->type);
			return -1;
		}
	}
}
