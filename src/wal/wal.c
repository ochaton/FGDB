#include  "wal/wal.h"

wal_logger_t* new_wal_logger(lsn_t LSN, lsn_t fLSN, uint32_t log_id) {
	wal_logger_t* w = malloc(sizeof(wal_logger_t));

	//creating new file for logs
	char* path = malloc(30*sizeof(char));
	sprintf(path, "log/%022i.log", log_id);
	w->file = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
	
	if (w->file == -1) {
		fprintf(stderr, "Error in opening log-file: %s\n", strerror(errno));
		exit(errno);
	}

	w->LSN        = LSN;
	w->flushedLSN = fLSN;

	return w;
}

lsn_t write_log(wal_logger_t* w, transaction_t* t) {
	wal_log_record_t* wr = to_wal_record(w, t);
	binary_record_t*  br = to_binary(wr);
	destroy_binary_record(br);
	lsn_t LSN = wr->LSN;
	destroy_wal_record(wr);
	return LSN;
}

// binary log has following structure
// LSN       8 bytes
// page_id   8 bytes
// operation 1 byte
// key
//   size    2 bytes
//   val     size bytes
// val
//   size    2 bytes
//   val     size bytes
// ******** ******** *  **   *..* **   *..*
// LSN      page_id  op size val  size val
binary_record_t* to_binary(wal_log_record_t* r) {
	binary_record_t* br = malloc(sizeof(binary_record_t));
	br->size = 8+8+1+2+(r->key.size)+2+(r->val.size);
	br->ptr  = malloc(br->size * sizeof(char));
	char* p = br->ptr;
	(lsn_t)     *p[0]   = r->LSN;
	(page_id_t) *p[8]   = r->pid;
	(msg_cmd_t) *p[16]  = r->operation;
	(uint16_t)  *p[17]  = r->key.size;

	strcpy(p[19], r->key.ptr, r->key.size);
	(uint16_t)  *p[19+(r->key.size)] = r->val.size;
	strcpy(*p[21+(r->key.size), r->val.ptr, r->val.size);
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
	if (t->cmd <= SELECT) {
		return NULL;
	}
	w->operation = t->cmd;

	wal_log_record_t* r = malloc(sizeof(wal_log_record_t));

	msg_t* m = t->msg;
	r->key.size = m->key.size;
	r->key.ptr  = malloc(r->key.size * sizeof(char));
	memcpy(r->key.ptr, m->key.ptr, m.key.size);

	r->val.size = m->val.size;
	r->val.ptr  = malloc(r->val.size * sizeof(char));
	memcpy(r->val.ptr, m->val.ptr, m.val.size);

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
}

void destroy_binary_record(binary_record_t* r) {
	free(r->ptr);
	free(r);
}
