
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
	msgpack_sbuffer* buffer = msgpack_sbuffer_new();
	msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);

	/* serializes ["PUT", {Key1 => "Value"}]. */
	msgpack_pack_array(pk, 2);
	msgpack_pack_bin(pk, 3);
	msgpack_pack_bin_body(pk, "PUT", 3);
	msgpack_pack_map(pk, 1);
	msgpack_pack_bin(pk, 4);
	msgpack_pack_bin_body(pk, "Key1", 4);
	msgpack_pack_bin(pk, 5);
	msgpack_pack_bin_body(pk, "Value", 5);

	// fprintf(stderr, "wrote %d bytes\n", write(1, buffer->data, buffer->size));

	/* deserializes it. */
	msgpack_unpacked msg;
	msgpack_unpacked_init(&msg);
	msgpack_unpack_return ret = msgpack_unpack_next(&msg, buffer->data, buffer->size, NULL);

	msgpack_object obj = msg.data;
	printf("%d\n", obj.type == MSGPACK_OBJECT_ARRAY);
	msgpack_object_array arr = obj.via.array;
	printf("%d\n", arr.size);
	printf("%d\n", (&arr)->ptr[1].type == MSGPACK_OBJECT_BIN);
	/* prints the deserialized object. */
	// msgpack_object_print(stdout, obj);  /*=> ["Hello", "MessagePack"] */

	/* cleaning */
	msgpack_sbuffer_free(buffer);
	msgpack_packer_free(pk);
	return 0;
}
