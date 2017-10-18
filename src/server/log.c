#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <stdarg.h>
#include <string.h>

#include "staff.h"

#define LOG_MESSAGE(LEVEL, REQID, FMT, args) do { \
	char time_str[20]; \
	get_strftime(&time_str[0], sizeof(time_str)); \
	char msg[4096]; \
	size_t writed = snprintf(msg, sizeof(msg), "[%s]\t[%d]\t[%s]\t%s\t", (LEVEL), getpid(), (REQID), time_str); \
	strcpy(msg + writed, (FMT)); \
	size_t msg_len = strlen(msg); \
	msg[msg_len] = '\n'; \
	msg[msg_len+1] = 0; \
	vprintf(msg, args); \
} while (0)

static char * rand_string(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t n = 0; n < size; n++) {
        int key = staff_random() % (int) (sizeof charset - 1);
        str[n] = charset[key];
    }
    str[size - 1] = '\0';
    return str;
}

void debug (const log_t *, const char * fmt, ...);
void info  (const log_t *, const char * fmt, ...);
void error (const log_t *, const char * fmt, ...);
void crit  (const log_t *, const char * fmt, ...);
void warn  (const log_t *, const char * fmt, ...);

log_t * init_log(void) {
	log_t * log = (log_t *) malloc(sizeof(log_t));
	rand_string(&log->reqid[0], sizeof(log->reqid));

	log->debug = debug;
	log->info  = info;
	log->error = error;
	log->crit  = crit;
	log->warn  = warn;
	return log;
}

void destroy_log(log_t * log) {
	if (log) free(log);
	return;
}

static char * get_strftime(char *str, size_t size) {
	time_t rawtime;
	time(&rawtime);
	// YYYY-MM-DDTHH:MM:SS
	strftime(str, size, "%FT%T", localtime(&rawtime));
	return str;
}

void debug (const log_t * self, const char * fmt, ...) {
	va_list args;
	va_start(args,fmt);

	LOG_MESSAGE("DEBUG", self->reqid, fmt, args);
	va_end(args);
	return;
}

void info (const log_t * self, const char * fmt, ...) {
	va_list args;
	va_start(args,fmt);

	LOG_MESSAGE("INFO", self->reqid, fmt, args);
	va_end(args);
	return;
}

void error (const log_t * self, const char * fmt, ...) {
	va_list args;
	va_start(args,fmt);

	LOG_MESSAGE("ERROR", self->reqid, fmt, args);
	va_end(args);
	return;
}

void crit (const log_t * self, const char * fmt, ...) {
	va_list args;
	va_start(args,fmt);

	LOG_MESSAGE("CRIT", self->reqid, fmt, args);
	va_end(args);
	return;
}

void warn (const log_t * self, const char * fmt, ...) {
	va_list args;
	va_start(args,fmt);

	LOG_MESSAGE("WARN", self->reqid, fmt, args);
	va_end(args);
	return;
}
