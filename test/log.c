#include "log.h"

int main(int argc, char const *argv[]) {

	log_t *log = init_log();
	log->debug(log, "Hello, World!: %s", "MAZAFAKA");

	return 0;
}