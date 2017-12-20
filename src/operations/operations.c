#include "common.h"
#include "server/request.h"
#include "server/proto.h"

#include "wal/wal.h"
#include "memory/hashmap.h"
#include "lib/buddy/memory.h"
#include "arena/meta.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

extern wal_logger_t *wal_logger;

proto_reply_t* operation_peek(transaction_t * trans, hashmap_t hashmap) {
	hashmap_error_t err;
	req_t* req = trans->ancestor;
	key_meta_t * key = hashmap_lookup_key(hashmap, &req->msg->key, &err);
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));

	if (!key) {
		req->log->debug(req->log, "Key not found");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_NOT_FOUND;
	} else {
		req->log->debug(req->log, "Key found");
		reply->code = REPLY_OK;
		reply->cmd  = req->msg->cmd;
	}

	return reply;
}

proto_reply_t* operation_select(transaction_t * trans, hashmap_t hashmap) {
	hashmap_error_t err;
	req_t* req = trans->ancestor;
	key_meta_t * key_meta = hashmap_lookup_key(hashmap, &req->msg->key, &err);
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));

	if (!key_meta) {
		req->log->info(req->log, "Key not found");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_NOT_FOUND;
		reply->cmd  = req->msg->cmd;
		return reply;
	}

	page_header_t * found_value_header = page_value_get(key_meta, &reply->val);

	req->log->info(req->log, "Key found #value=%d", reply->val.size);
	req->log->debug(req->log, "Value = `%s`", reply->val.ptr); // here should be xd

	reply->code = REPLY_OK;
	reply->cmd  = req->msg->cmd;

}

proto_reply_t* operation_delete(transaction_t * trans, hashmap_t hashmap) {
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));

	hashmap_error_t err;
	req_t* req = trans->ancestor;
	key_meta_t * deleted = hashmap_delete_key(hashmap, &req->msg->key, &err);

	if (!deleted) {
		req->log->info(req->log, "Key not found");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_NOT_FOUND;
		return reply;
	}
	lsn_t LSN = write_log(wal_logger, trans);

	page_header_t * header = page_value_unset(deleted, &reply->val);

	if (!header) {
		req->log->error(req->log, "Unsetting value failed while delete key");
		reply->code  = REPLY_FATAL;
		reply->cmd   = req->msg->cmd;
		reply->fatal = PROTO_ERROR_UNKNOWN;
		return reply;
	}

	update_lsn(header, LSN);
	reply->code = REPLY_OK;
	reply->cmd = req->msg->cmd;

	req->log->debug(req->log, "Replying value # = %ld {%*.*s}", reply->val.size, reply->val.size, reply->val.size, reply->val.ptr);

	return reply;
}

proto_reply_t* operation_insert(transaction_t * trans, hashmap_t hashmap) {
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));
	req_t* req = trans->ancestor;

	/* Validate incoming data */

	if (!req->msg->val.size) {
		req->log->error(req->log, "Value not found");
		reply->code  = REPLY_FATAL;
		reply->fatal = PROTO_ERROR_VALUE_REQUIRED;

		return reply;
	}

	/* Lookup for existing key */

	hashmap_error_t err;
	key_meta_t * key_found = hashmap_lookup_key(hashmap, &req->msg->key, &err);

	if (key_found) {
		req->log->error(req->log, "Key exists");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_EXISTS;

		return reply;
	}

	lsn_t LSN = write_log(wal_logger, trans);

	/* Insert value into arena */

	key_meta_t * new_key_meta = (key_meta_t *) calloc(1, sizeof(key_meta_t));

	req->log->debug(req->log, "Inserting new key {%*.*s} => value {%*.*s}",
		req->msg->key.size, req->msg->key.size,
		req->msg->key.ptr,
		req->msg->val.size, req->msg->val.size,
		req->msg->val.ptr
	);
	page_header_t * header = page_value_set(&req->msg->val, new_key_meta);

	if (!header) {
		req->log->error(req->log, "Memory not allocated for value");
		reply->code  = REPLY_FATAL;
		reply->fatal = PROTO_ERROR_UNKNOWN;

		return reply;
	}

	update_lsn(header, LSN);

	req->log->debug(req->log, "Value has been put into arena");

	/* Insert key into hashmap */

	if (-1 == hashmap_insert_key(hashmap, new_key_meta, &req->msg->key, &err)) {
		req->log->error(req->log, "Fatal error on inserting key %s", hashmap_error[err]);

		str_t unset_value;
		if (page_value_unset(new_key_meta, &unset_value)) {
			req->log->warn(req->log, "Value has been unset successfully");
		} else {
			req->log->error(req->log, "Unsetting value failed");
		}

		reply->code  = REPLY_FATAL;
		reply->fatal = PROTO_ERROR_UNKNOWN;
		reply->cmd   = req->msg->cmd;

		return reply;
	}

	req->log->info(req->log, "Inserted sucessfully");

	reply->code = REPLY_OK;
	reply->cmd  = req->msg->cmd;
	return reply;
}

proto_reply_t* operation_update(transaction_t * trans, hashmap_t hashmap) {
	// TODO: do not forget to work with LSN when writing update
	// I think it should not be a problem if we just copy-paste code from delete + insert as long as basicly it is exactly that
	// But there might be some problems still
	destroy_request(trans->ancestor);
	return NULL;
}
