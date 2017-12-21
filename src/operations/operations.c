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

proto_reply_t* operation_peek(transaction_t * trans) {
	hashmap_error_t err;
	key_meta_t * key = hashmap_lookup_key(hashmap, &trans->msg->key, &err);
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));

	if (!key) {
		trans->log->debug(trans->log, "Key not found");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_NOT_FOUND;
	} else {
		trans->log->debug(trans->log, "Key found");
		reply->code = REPLY_OK;
		reply->cmd  = trans->msg->cmd;
	}

	return reply;
}

proto_reply_t* operation_select(transaction_t * trans) {
	hashmap_error_t err;
	key_meta_t * key_meta = hashmap_lookup_key(hashmap, &trans->msg->key, &err);
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));

	if (!key_meta) {
		trans->log->info(trans->log, "Key not found");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_NOT_FOUND;
		reply->cmd  = trans->msg->cmd;
		return reply;
	}

	page_header_t * found_value_header = page_value_get(key_meta, &reply->val);

	trans->log->info(trans->log, "Key found #value=%d", reply->val.size);
	trans->log->debug(trans->log, "Value = `%s`", reply->val.ptr); // here should be xd

	reply->code = REPLY_OK;
	reply->cmd  = trans->msg->cmd;

}

proto_reply_t* operation_delete(transaction_t * trans) {
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));

	hashmap_error_t err;
	key_meta_t * deleted = hashmap_delete_key(hashmap, &trans->msg->key, &err);

	if (!deleted) {
		trans->log->info(trans->log, "Key not found");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_NOT_FOUND;
		return reply;
	}
	lsn_t LSN = write_log(wal_logger, trans);

	page_header_t * header = page_value_unset(deleted, &reply->val);

	if (!header) {
		trans->log->error(trans->log, "Unsetting value failed while delete key");
		reply->code  = REPLY_FATAL;
		reply->cmd   = trans->msg->cmd;
		reply->fatal = PROTO_ERROR_UNKNOWN;
		return reply;
	}

	update_lsn(header, LSN);
	reply->code = REPLY_OK;
	reply->cmd = trans->msg->cmd;

	trans->log->debug(trans->log, "Replying value # = %ld {%*.*s}", reply->val.size, reply->val.size, reply->val.size, reply->val.ptr);

	return reply;
}

proto_reply_t* operation_insert(transaction_t * trans) {
	proto_reply_t* reply = malloc(sizeof(proto_reply_t));

	/* Validate incoming data */

	if (!trans->msg->val.size) {
		trans->log->error(trans->log, "Value not found");
		reply->code  = REPLY_FATAL;
		reply->fatal = PROTO_ERROR_VALUE_REQUIRED;

		return reply;
	}

	/* Lookup for existing key */

	hashmap_error_t err;
	key_meta_t * key_found = hashmap_lookup_key(hashmap, &trans->msg->key, &err);

	if (key_found) {
		trans->log->error(trans->log, "Key exists");
		reply->code = REPLY_ERROR;
		reply->err  = KEY_EXISTS;

		return reply;
	}

	lsn_t LSN = write_log(wal_logger, trans);

	/* Insert value into arena */

	key_meta_t * new_key_meta = (key_meta_t *) calloc(1, sizeof(key_meta_t));

	trans->log->debug(trans->log, "Inserting new key {%*.*s} => value {%*.*s}",
		trans->msg->key.size, trans->msg->key.size,
		trans->msg->key.ptr,
		trans->msg->val.size, trans->msg->val.size,
		trans->msg->val.ptr
	);
	page_header_t * header = page_value_set(&trans->msg->val, new_key_meta);

	if (!header) {
		trans->log->error(trans->log, "Memory not allocated for value");
		reply->code  = REPLY_FATAL;
		reply->fatal = PROTO_ERROR_UNKNOWN;

		return reply;
	}

	update_lsn(header, LSN);

	trans->log->debug(trans->log, "Value has been put into arena");

	/* Insert key into hashmap */

	str_t * copykey = string_copy(&trans->msg->key);
	new_key_meta->weak_key = copykey;
	if (-1 == hashmap_insert_key(hashmap, new_key_meta, copykey, &err)) {
		trans->log->error(trans->log, "Fatal error on inserting key %s", hashmap_error[err]);

		destroy_string(copykey);

		str_t unset_value;
		if (page_value_unset(new_key_meta, &unset_value)) {
			trans->log->warn(trans->log, "Value has been unset successfully");
		} else {
			trans->log->error(trans->log, "Unsetting value failed");
		}

		reply->code  = REPLY_FATAL;
		reply->fatal = PROTO_ERROR_UNKNOWN;
		reply->cmd   = trans->msg->cmd;

		return reply;
	}

	trans->log->info(trans->log, "Inserted sucessfully");

	reply->code = REPLY_OK;
	reply->cmd  = trans->msg->cmd;
	return reply;
}

proto_reply_t* operation_update(transaction_t * trans) {
	// TODO: do not forget to work with LSN when writing update
	// I think it should not be a problem if we just copy-paste code from delete + insert as long as basicly it is exactly that
	// But there might be some problems still
	// destroy_request(trans->ancestor);
	return NULL;
}
