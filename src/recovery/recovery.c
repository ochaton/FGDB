#include "recovery/recovery.h"

static void perform_transaction (transaction_t* t) {
	switch(t->msg->cmd) {
		case PEEK:
		{
			operation_peek(t);
			break;
		}
		case SELECT:
		{
			operation_select(t);
			break;
		}
		case INSERT:
		{
			operation_insert(t);
			break;
		}
		case DELETE:
		{
			operation_delete(t);
			break;
		}
		case UPDATE:
		{
			operation_update(t);
			break;
		}
		default:
		{
			fprintf(stderr, "Unknown operation %d\n", t->msg->cmd);
			break;
		}
	}
}

void wal_recovery(lsn_t startLSN) {
	DIR           *d;
	struct dirent *dir;
	d = opendir("log");
	if (d) {
		dir = readdir(d);
		closedir(d);
	} else {
		// no logs -> no recovery
		return;
	}

	wal_unlogger_t* u = new_unlogger(dir->d_name);
	lsn_t lastLSN = get_latest_log_LSN(u);
	recovery_t* r = recover_transaction(u);
	while (r) {
		if (r->LSN >= startLSN) {
			perform_transaction(r->transaction);
		}
		r = recover_transaction(u);
	}

}
