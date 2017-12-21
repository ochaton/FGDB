#define _GNU_SOURCE
#include <string.h>

#include  "wal/wal.h"

wal_logger_t* new_wal_logger(lsn_t LSN, lsn_t fLSN, uint32_t log_id) {
	wal_logger_t* w = malloc(sizeof(wal_logger_t));

	//creating new file for logs
	char path[48];
	sprintf(path, "log/%022i.log", log_id);

	if (mkdir("log", 0777) == -1 && errno != EEXIST) {
		fprintf(stderr, "Error in creating path: %s\n", strerror(errno));
		exit(errno);
	};
	w->file = open(path, O_WRONLY | O_CREAT, 0644);
	if (w->file == -1) {
		fprintf(stderr, "Error in opening log-file: %s\n", strerror(errno));
		exit(errno);
	}

	w->LSN        = LSN;
	w->flushedLSN = fLSN;

	return w;
}

void dbg_str (str_t s) {
	for (int i = 0; i<=s.size; i++) {
		fprintf(stderr, "[%d/%d] %c\n", i, s.size, s.ptr[i]);
	}
	return;
}

static void update_heading_lsn(wal_logger_t* w, lsn_t LSN) {
	if (lseek(w->file, 0, SEEK_SET) == -1) {
		fprintf(stderr, "Problems, when seeking in file: %s\n", strerror(errno));
		exit(errno);
	}

	if (write(w->file, &LSN, sizeof(lsn_t)) == -1) {
		fprintf(stderr, "Problems, when updating heading LSN: %s\n", strerror(errno));
		exit(errno);
	}

	// adding \n after every log line for easier parsing in vim binary mode
	// if (write(w->file, "\n", 1) == -1) {
	// 	fprintf(stderr, "Problems while writing binary logs: %s\n", strerror(errno));
	// 	exit(errno);
	// }

	if (lseek(w->file, 0, SEEK_END) == -1) {
		fprintf(stderr, "Problems, when seeking in file: %s\n", strerror(errno));
		exit(errno);
	}
}

// wal_log is going to have following structure
// lastLSN 8 bytes (the last LSN stored in the log)
// binary_record
// ...
// binary_record
// where binary record's structure is defined above to_binary function
lsn_t write_log(wal_logger_t* w, transaction_t* t) {
	wal_log_record_t* wr = to_wal_record(w, t);
	// operation is not logged
	if (!wr) {
		return 0;
	}

	binary_record_t *  br = to_binary(wr);

	update_heading_lsn(w, wr->LSN);

	size_t written = 0;
	while (written < br->size) {
		ssize_t bytes;
		if (-1 == (bytes = write(w->file, br->ptr + written, br->size - written))) {
			if (errno == EAGAIN || errno == EINTR) {
				// Should never happen
			} else {
				fprintf(stderr, "Problems while writing binary logs: %s\n", strerror(errno));
				exit(errno);
			}
		}

		written += bytes;
	}

	// adding \n after every log line for easier parsing in vim binary mode
	// if (write(w->file, "\n", 1) != 1) {
	// 	fprintf(stderr, "Problems while writing binary logs: %s\n", strerror(errno));
	// 	exit(errno);
	// }

	lsn_t LSN = wr->LSN;
	destroy_binary_record(br);
	destroy_wal_record(wr);
	return LSN;
}


// binary log has following structure
// length    8 bytes (keeps own size inside itself)
// LSN       8 bytes
// operation 1 byte
// key
//   size    2 bytes
//   val     size bytes
// val
//   size    2 bytes
//   val     size bytes
// ******** ******** *  **   *..* **   *..*
// length   LSN      op size val  size val
binary_record_t* to_binary(wal_log_record_t* r) {
	binary_record_t* br = malloc(sizeof(binary_record_t));
	br->size = sizeof(br->size)
		+ sizeof(r->LSN)
		+ sizeof(r->operation)
		+ sizeof(r->key.size) + r->key.size
		+ sizeof(r->val.size) + r->val.size;

	char * p = br->ptr  = malloc(br->size);
	p = mempcpy(p, &br->size,    sizeof(br->size));
	p = mempcpy(p, r,            sizeof(r->LSN) + sizeof(r->operation));
	p = mempcpy(p, &r->key.size, sizeof(r->key.size));
	p = mempcpy(p, r->key.ptr,   r->key.size);
	p = mempcpy(p, &r->val.size, sizeof(r->val.size));
	p = mempcpy(p, r->val.ptr,   r->val.size);

	return br;
}

void update_flushed_LSN(wal_logger_t* w, lsn_t fLSN) {
	w->flushedLSN = fLSN;
	return;
}

lsn_t give_new_LSN(wal_logger_t* w) {
	return ++w->LSN;
}

// transforms transaction_t to wal_log_record_t
// returns NULL if the operation does not need to be logged
wal_log_record_t* to_wal_record(wal_logger_t* w, transaction_t* t) {
	if (t->msg->cmd <= SELECT) {
		return NULL;
	}

	wal_log_record_t* r = malloc(sizeof(wal_log_record_t));

	r->operation = t->msg->cmd;
	msg_t* msg = t->msg;
	r->key.size = msg->key.size;
	r->key.ptr  = malloc(r->key.size);
	memcpy(r->key.ptr, msg->key.ptr, msg->key.size);

	r->val.size = msg->val.size;
	r->val.ptr  = malloc(r->val.size);
	memcpy(r->val.ptr, msg->val.ptr, msg->val.size);

	r->LSN = give_new_LSN(w);
	return r;
}

void destroy_wal_record(wal_log_record_t* r) {
	free(r->key.ptr);
	free(r->val.ptr);
	free(r);
	return;
}

void destroy_wal_logger(wal_logger_t* w) {
	close(w->file);
	free(w);
	return;
}

void destroy_binary_record(binary_record_t* r) {
	free(r->ptr);
	free(r);
	return;
}

wal_unlogger_t* new_unlogger(char* path) {
	wal_unlogger_t* u = malloc(sizeof(wal_unlogger_t));
	u->file = open(path, O_RDONLY);
	if (u->file == -1) {
		fprintf(stderr, "Error in opening file: %s\n", strerror(errno));
		exit(errno);
	}
	u->seek = 0;
	return u;
}

// reads lsn_t from the start of the file
// has a side effect of shifting seek in file
// should be called 1 single time at the begining of recovery
lsn_t get_latest_log_LSN(wal_unlogger_t* u) {
	off_t current = lseek(u->file, 0, SEEK_CUR);
	if (current == -1) {
		fprintf(stderr, "Problems, when seeking in file: %s\n", strerror(errno));
		exit(errno);
	}

	lsn_t LSN;
	if (read(u->file, &LSN, sizeof(lsn_t)) == -1) {
		fprintf(stderr, "Problems, while reading from file: %s\n", strerror(errno));
		exit(errno);
	}

	if (lseek(u->file, current, SEEK_CUR) == -1) {
		fprintf(stderr, "Problems, when seeking in file: %s\n", strerror(errno));
		exit(errno);
	}

	return LSN;
}

transaction_t* recover_transaction(wal_unlogger_t* u) {
	binary_record_t br;
	if (-1 == read(u->file, &br.size, sizeof(br.size))) {
		fprintf(stderr, "Error reading binary log (size): %s\n", strerror(errno));
		exit(errno);
	}

	br.size -= sizeof(br.size);
	br.ptr = (char *) malloc(br.size);
	if (-1 == read(u->file, br.ptr, br.size)) {
		fprintf(stderr, "Error reading binary log (record): %s\n", strerror(errno));
		exit(errno);
	}

	wal_log_record_t * wr = malloc(sizeof(wal_log_record_t));
	char *p = br.ptr;

	memcpy(&wr->LSN, p, sizeof(wr->LSN));              p += sizeof(wr->LSN);
	memcpy(&wr->operation, p, sizeof(wr->operation));  p += sizeof(wr->operation);

	// NULL ancestor as long as we don't really have any
	transaction_t* t = malloc(sizeof(transaction_t));
	t->ancestor = NULL;

	t->msg = malloc(sizeof(msg_t));
	t->msg->cmd = wr->operation;

	memcpy(&t->msg->key.size, p, sizeof(t->msg->key.size)); p += sizeof(t->msg->key.size);
	t->msg->key.ptr = (char *) malloc(t->msg->key.size);
	memcpy(t->msg->key.ptr, p, t->msg->key.size);           p += t->msg->key.size;

	memcpy(&t->msg->val.size, p, sizeof(t->msg->val.size)); p += sizeof(t->msg->val.size);
	t->msg->val.ptr = (char *) malloc(t->msg->val.size);
	memcpy(t->msg->val.ptr, p, t->msg->val.size);           p += t->msg->val.size;

	assert((uint64_t) p == (uint64_t) br.ptr + br.size); // WAL-log line was read completely

	free(wr);
	free(br.ptr);

	return t;
}

void destroy_wal_unlogger(wal_unlogger_t* u) {
	close(u->file);
	free(u);
	return;
}
