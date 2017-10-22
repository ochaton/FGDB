#include "common.h"
#include "request.h"
#include "proto.h"

#include "hashmap.h"
#include "memory.h"
#include "arena.h"

#include <errno.h>
#include <string.h>

void operation_peek(req_t * req, hashmap_t * hashmap);
void operation_select(req_t * req, hashmap_t * hashmap);
void operation_delete(req_t * req, hashmap_t * hashmap);
void operation_insert(req_t * req, hashmap_t * hashmap);
void operation_update(req_t * req, hashmap_t * hashmap);

void operation_peek(req_t * req, hashmap_t * hashmap) {
	hashmap_key_t * key = hashmap_lookup_key(hashmap, &req->msg->key);
	proto_reply_t reply;

	if (!key) {
		req->log->debug(req->log, "Key not found");
		reply.code = REPLY_ERROR;
		reply.err  = KEY_NOT_FOUND;
	} else {
		req->log->debug(req->log, "Key found");
		reply.code = REPLY_OK;
		reply.cmd  = req->msg->cmd;
	}

	request_reply(req, &reply);
	return;
}

void operation_select(req_t * req, hashmap_t * hashmap) {
	destroy_request(req);
}

void operation_delete(req_t * req, hashmap_t * hashmap) {
	destroy_request(req);
}

void operation_insert(req_t * req, hashmap_t * hashmap) {
	hashmap_key_t * key = hashmap_lookup_key(hashmap, &req->msg->key);
	proto_reply_t reply;

	if (!req->msg->val.size) {
		req->log->error(req->log, "Value not found");
		reply.code  = REPLY_FATAL;
		reply.fatal = PROTO_ERROR_VALUE_REQUIRED;

		request_reply(req, &reply);
		return;
	}

	if (key) {
		req->log->error(req->log, "Key exists");
		reply.code = REPLY_ERROR;
		reply.err  = KEY_EXISTS;

		request_reply(req, &reply);
		return;
	}

	req->log->debug(req->log, "Inserting new key=>value");

	// Here we take memory for our value
	arena_node_t * value = (arena_node_t *)
		buddy_alloc(req->msg->val.size + sizeof(arena_node_t));

	if (!value) {
		req->log->error(req->log, "Memory not allocated for value");
		reply.code  = REPLY_FATAL;
		reply.fatal = PROTO_ERROR_UNKNOWN;

		request_reply(req, &reply);
		return;
	} else {
		req->log->debug(req->log, "Arena allocated");
	}

	value->size = req->msg->val.size;

	// TODO: here can be some shit...
	memcpy(&value->ptr[0], req->msg->val.ptr, value->size);

	hashmap_key_t key_to_insert;
	key_to_insert.key.size = req->msg->key.size;
	key_to_insert.key.ptr  = (char *) malloc(key_to_insert.key.size);

	if (!key_to_insert.key.ptr) {
		req->log->error(req->log, "Memory not allocated for key errno=%s", strerror(errno));
		buddy_free(value);

		reply.code  = REPLY_FATAL;
		reply.fatal = PROTO_ERROR_UNKNOWN;

		request_reply(req, &reply);
		return;
	}

	// TODO: here also can be some shit
	memcpy(key_to_insert.key.ptr, req->msg->key.ptr, key_to_insert.key.size);
	key_to_insert.page   = &value->size;
	key_to_insert.offset = 0;
	key_to_insert.location = INMEMORY;
	key_to_insert.fragmentated = CLEAN;

	hashmap_error_t err;
	hashmap_key_t * inserted;
	if (NULL == (inserted = hashmap_insert_key(hashmap, &key_to_insert, &err))) {
		req->log->error(req->log, "Insert to hashmap error: %s", hashmap_error[err]);
		free(key_to_insert.key.ptr);
		buddy_free(value);

		reply.code  = REPLY_FATAL;
		reply.fatal = PROTO_ERROR_UNKNOWN;

		request_reply(req, &reply);
		return;
	}

	value->rev_key = inserted;
	req->log->info(req->log, "Inserted sucessfully");

	reply.code = REPLY_OK;
	reply.cmd  = req->msg->cmd;
	request_reply(req, &reply);
	return;
}

void operation_update(req_t * req, hashmap_t * hashmap) {
	destroy_request(req);
}
