#ifndef FGDB_CONFIG_H
#define FGDB_CONFIG_H

typedef struct config_t {

	struct {
		char snap_dir[1024];
		char key_file[256];
	} disk;

	struct {
		char wal_dir[1024];
	} wal;

	struct {
		size_t size;
	} arena;
} config_t;

#endif // FGDB_CONFIG_H