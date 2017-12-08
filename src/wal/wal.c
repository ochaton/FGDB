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

void update_flushed_LSN(wal_logger_t* w, lsn_t fLSN) {
	w->flushedLSN = fLSN;
	return;
}

lsn_t give_new_LSN(wal_logger_t* w) {
	return ++w->LSN;
}

wal_log_record_t* to_wal_record(wal_logger_t* w, transaction_t* t) {
	wal_log_record_t* r = malloc(sizeof(wal_log_record_t));

	msg_t* m = t->msg;

}
