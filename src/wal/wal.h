#ifndef WAL_H
#define WAL_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t lsn_t;

#include "common.h"
#include "server/message.h"
#include "transactions/queue.h"

// TODO: raise flushedLSN when dumping page to disk
// TODO: make single file with recoveryLSNs for each page
// TODO: dump pageLSN with the page

typedef struct {
	lsn_t              LSN;
	str_t              key;
	str_t              val;
	enum msg_command_t operation;
} wal_log_record_t;

typedef struct {
	lsn_t flushedLSN;
	lsn_t LSN;
	int   file;
} wal_logger_t;

typedef struct {
	size_t size;
	char*  ptr;
} binary_record_t;

typedef struct {
	uint64_t seek;
	int      file;
} wal_unlogger_t;

wal_logger_t* new_wal_logger(lsn_t LSN, lsn_t fLSN, uint32_t log_id);

void dbg_str (str_t s);

lsn_t write_log(wal_logger_t* w, transaction_t* t);

binary_record_t* to_binary(wal_log_record_t* r);

void update_flushed_LSN(wal_logger_t* w, lsn_t fLSN);

lsn_t give_new_LSN(wal_logger_t* w);

wal_log_record_t* to_wal_record(wal_logger_t* w, transaction_t* t);

void destroy_wal_record(wal_log_record_t* r);

void destroy_wal_logger(wal_logger_t* w);

void destroy_binary_record(binary_record_t* r);

// RECOVERY STARTS HERE

wal_unlogger_t* new_unlogger(char* path);

transaction_t* recover_transaction(wal_unlogger_t* u);

void destroy_wal_unlogger(wal_unlogger_t* u);

#endif
