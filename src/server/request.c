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

void parse_request(EV_P_  ev_io *w, int revents);

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
	req->state  = INIT;
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

	req->log->info(req->log, "Request finished");

	if (req->log) destroy_log(req->log);
	free(req);
	return;
}

static int parse_command(req_t * req, msgpack_object * obj);
static int parse_data(req_t * req, msgpack_object * obj);

void parse_request(EV_P_ ev_io *w, int revents) {
	req_t * req = (req_t *) w;

	ssize_t bytes;
	switch (req->state) {
		case INIT:
		{
			req->log->debug(req->log, "Parsing INIT");
			char buffer[1024] = {};
			char * pbuf = &buffer[0];

			if (-1 == (bytes = recv(req->fd, buffer, sizeof(buffer) - 1, 0))) {
				if (errno == EAGAIN || errno == EINTR) {
					return;
				} else {
					req->log->crit(req->log, "Error on read from socket: %s", strerror(errno));
					return destroy_request(req);
				}
			}

			uint32_t msg_len = *(uint32_t *) pbuf;
			// memcpy(&msg_len, pbuf, sizeof(msg_len));
			pbuf += sizeof(msg_len);

			req->log->debug(req->log, "Message len = %ld", msg_len);

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

			size_t bytes2copy = bytes > msg_len ? msg_len : bytes;
			buffer_push(req->buf, pbuf, bytes2copy);

			req->log->debug(req->log, "Buffer push succesfull");
			req->state = READ;
		}
		case READ:
		{
			req->log->debug(req->log, "Parsing READ");
			if (req->buf->free) {
				if (-1 == (bytes = recv(req->fd, &req->buf->start[req->buf->used], req->buf->free, 0))) {
					if (errno == EAGAIN || errno == EINTR) {
						return;
					} else {
						req->log->crit(req->log, "Error on read from socket: %s", strerror(errno));
						return destroy_request(req);
					}
				}

				req->buf->free -= bytes;
				req->buf->used += bytes;

				req->log->debug(req->log, "Available %u bytes", req->buf->free);

				if (!bytes || !req->buf->free) {
					req->state = PARSE;
				} else {
					break;
				}
			} else {
				req->state = PARSE;
			}
		}
		case PARSE:
		{
			req->log->debug(req->log, "Parsing PARSE");
			req->state = SERVICE;

			msgpack_unpacked unpacked;
			msgpack_unpacked_init(&unpacked);

			msgpack_unpack_return ret = msgpack_unpack_next(&unpacked, &req->buf->start[0], req->buf->used, NULL);
			req->log->debug(req->log, "Ret = %d", ret);
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

			ev_io_stop(EV_DEFAULT_ &req->io);
			req->server->on_request(req);

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
	if (obj->type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
		req->log->error(req->log, "Command expected as POSITIVE_INTEGER, got %d", obj->type);
		return -1;
	}

	return message_command(req->msg, obj->via.i64);
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
