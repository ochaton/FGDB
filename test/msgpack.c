
/* Message Protocol:
	size - 4 bytes;
	msg  - msg-pack encoded data;
*/

#include <msgpack.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	/* creates buffer and serializer instance. */
	// msgpack_sbuffer* buffer = msgpack_sbuffer_new();
	// msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);

	// /* serializes ["PUT", {Key1 => "Value"}]. */
	// msgpack_pack_array(pk, 2);
	// msgpack_pack_bin(pk, 5);
	// msgpack_pack_bin_body(pk, "Hello", 5);
	// msgpack_pack_bin(pk, 11);
	// msgpack_pack_bin_body(pk, "MessagePack", 11);

	char buf[] = "\x92\xc4\x05\x48\x65\x6c\x6c\x6f\xc4\x0b\x4d\x65\x73\x73\x61\x67\x65\x50\x61\x63\x6b";

	write(1, buf, sizeof(buf)-1);
	/* deserializes it. */
	size_t off = 0;

	msgpack_unpacked msg;
	msgpack_unpacked_init(&msg);

	msg.zone = msgpack_zone_new(8192);
	msgpack_unpack_return ret = msgpack_unpack_next(&msg, buf, sizeof(buf) - 1, &off);

	msgpack_object obj = msg.data;
	printf("%d\n", obj.type == MSGPACK_OBJECT_ARRAY);
	msgpack_object_array arr = obj.via.array;
	printf("%d\n", arr.size);
	printf("%d\n", (&arr)->ptr[1].type == MSGPACK_OBJECT_BIN);
	/* prints the deserialized object. */
	// msgpack_object_print(stdout, obj);  /*=> ["Hello", "MessagePack"] */

	/* cleaning */
	// msgpack_sbuffer_free(buffer);
	// msgpack_packer_free(pk);
	return 0;
}
