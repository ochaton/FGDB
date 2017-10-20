#include "proto.h"
#include <msgpack.h>

msgpack_sbuffer* serialize_reply (proto_reply_t * reply) {
	msgpack_sbuffer* buffer = msgpack_sbuffer_new();
	msgpack_packer* packer = msgpack_packer_new(buffer, msgpack_sbuffer_write);

	msgpack_pack_array(packer, 2);
	msgpack_pack_int(packer, reply->code);

	switch(reply->code) {
		case REPLY_OK:
		{
			switch(reply->cmd) {
				case PEEK: case INSERT: case UPDATE:
				{
					msgpack_pack_int(packer, CODE_OK);
					break;
				}
				case SELECT: case DELETE:
				{
					msgpack_pack_bin(packer, reply->val.size);
					msgpack_pack_bin_body(packer, reply->val.ptr, reply->val.size);
					break;
				}
			}
			break;
		}
		case REPLY_ERROR:
		{
			msgpack_pack_int(packer, reply->err);
			break;
		}
		case REPLY_FATAL:
		{
			msgpack_pack_int(packer, reply->fatal);
			break;
		}
	}

	msgpack_packer_free(packer);
	return buffer;
}
