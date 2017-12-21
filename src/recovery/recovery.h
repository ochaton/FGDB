#ifndef RECOVERY_H
#define RECOVERY_H

#include <dirent.h>

#include "common.h"
#include "lru/lruq.h"
#include "operations/operations.h"
#include "wal/wal.h"

void wal_recovery(lsn_t startLSN);

#endif
