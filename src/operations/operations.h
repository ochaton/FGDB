#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "server/proto.h"
#include "lru/lruq.h"
#include "memory/hashmap.h"

proto_reply_t* operation_peek(transaction_t   * trans, hashmap_t hashmap);
proto_reply_t* operation_select(transaction_t * trans, hashmap_t hashmap);
proto_reply_t* operation_delete(transaction_t * trans, hashmap_t hashmap);
proto_reply_t* operation_insert(transaction_t * trans, hashmap_t hashmap);
proto_reply_t* operation_update(transaction_t * trans, hashmap_t hashmap);

#endif
