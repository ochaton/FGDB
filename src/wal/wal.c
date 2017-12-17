#include  "wal/wal.h"

wal_logger_t* new_wal_logger(lsn_t LSN, lsn_t fLSN, uint32_t log_id) {
	wal_logger_t* w = malloc(sizeof(wal_logger_t));

	//creating new file for logs
	char* path = malloc(31*sizeof(char));
	sprintf(path, "log/%022i.log", log_id);

	if (mkdir("log", 0777) == -1 && errno != EEXIST) {
		fprintf(stderr, "Error in creating path: %s\n", strerror(errno));
		exit(errno);
	};
	w->file = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
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

lsn_t write_log(wal_logger_t* w, transaction_t* t) {
	wal_log_record_t* wr = to_wal_record(w, t);
	binary_record_t*  br = to_binary(wr);
	if ((uint64_t) write(w->file, br->ptr, br->size) != br->size) {
		fprintf(stderr, "Problems while writing binary logs: %s\n", strerror(errno));
		exit(errno);
	}

	// adding \n after every log line for easier parsing in vim binary mode
	if (write(w->file, "\n", 1) != 1) {
		fprintf(stderr, "Problems while writing binary logs: %s\n", strerror(errno));
		exit(errno);
	}
	destroy_binary_record(br);
	lsn_t LSN = wr->LSN;
	destroy_wal_record(wr);
	return LSN;
}

static void continous_copy (void* dest, void* src, uint16_t size, uint64_t* shift) {
	memcpy(dest, src, size);
	*shift += size;
	return;
}

// binary log has following structure
// LSN       8 bytes
// operation 1 byte
// key
//   size    2 bytes
//   val     size bytes
// val
//   size    2 bytes
//   val     size bytes
// ******** *  **   *..* **   *..*
// LSN      op size val  size val
binary_record_t* to_binary(wal_log_record_t* r) {
	binary_record_t* br = malloc(sizeof(binary_record_t));
	br->size = 8+1+2+(r->key.size)+2+(r->val.size);
	br->ptr  = malloc(br->size * sizeof(char));

	char* p = br->ptr;
	//make sure it IS EXACTLY 1 byte
	char op = (char) r->operation;
	uint64_t shift = 0;
	continous_copy(&p[shift], &r->LSN,      sizeof(r->LSN),             &shift);
	// continous_copy(&p[shift], &r->pid,      sizeof(r->pid),             &shift);
	continous_copy(&p[shift], &op,          sizeof(op),                 &shift);
	continous_copy(&p[shift], &r->key.size, sizeof(r->key.size),        &shift);
	continous_copy(&p[shift], r->key.ptr,   sizeof(char) * r->key.size, &shift);
	continous_copy(&p[shift], &r->val.size, sizeof(r->val.size),        &shift);
	continous_copy(&p[shift], r->val.ptr,   sizeof(char) * r->val.size, &shift);
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
	msg_t* m = t->msg;
	r->key.size = m->key.size;
	r->key.ptr  = malloc(r->key.size * sizeof(char));
	memcpy(r->key.ptr, m->key.ptr, m->key.size);

	r->val.size = m->val.size;
	r->val.ptr  = malloc(r->val.size * sizeof(char));
	memcpy(r->val.ptr, m->val.ptr, m->val.size);

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

static void read_to_buf(int file, char* buf, uint64_t size) {
	ssize_t size_read;
	size_read = read(file, buf, size);
	if ((uint64_t) size_read != size) {
		fprintf(stderr, "Error reading binary log: %s\n", strerror(errno));
		exit(errno);
	}
}

transaction_t* recover_transaction(wal_unlogger_t* u) {
	transaction_t* t = malloc(sizeof(transaction_t));

	char* buf8 = malloc(8*sizeof(char));
	char* buf2 = malloc(2*sizeof(char));
	char* buf1 = malloc(1*sizeof(char));

	read_to_buf(u->file, buf8, 8);
	uint64_t LSN   = *((uint64_t*) buf8);

	read_to_buf(u->file, buf1, 1);
	uint16_t op    = *((char*)     buf1);

	// read key from log
	read_to_buf(u->file, buf2, 2);
	uint16_t ksize = *((uint16_t*) buf2);
	char* key_buf = malloc(sizeof(char) * ksize);
	read_to_buf(u->file, key_buf, ksize);

	// read val from log
	read_to_buf(u->file, buf2, 2);
	uint16_t vsize = *((uint16_t*) buf2);
	char* val_buf = malloc(sizeof(char) * vsize);
	read_to_buf(u->file, val_buf, vsize);

	// read following \n
	read_to_buf(u->file, buf1, 1);

	msg_t* m = malloc(sizeof(msg_t));
	m->key.size = ksize;
	m->key.ptr  = key_buf;
	m->val.size = vsize;
	m->val.ptr  = val_buf;
	m->cmd      = (enum msg_command_t) op;

	// NULL ancestor as long as we don't really have any
	t->ancestor = NULL;
	t->msg      = m;

	free(buf8);
	free(buf2);
	free(buf1);
	return t;
}

void destroy_wal_unlogger(wal_unlogger_t* u) {
	close(u->file);
	free(u);
	return;
}
