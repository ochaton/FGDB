#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "common.h"
#include "server/proto.h"
#include "lru/lruq.h"
#include "memory/hashmap.h"
#include "transactions/queue.h"

proto_reply_t* operation_peek(transaction_t   * trans);
proto_reply_t* operation_select(transaction_t * trans);
proto_reply_t* operation_delete(transaction_t * trans);
proto_reply_t* operation_insert(transaction_t * trans);
proto_reply_t* operation_update(transaction_t * trans);

#endif
