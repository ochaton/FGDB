#ifndef LOG_H
#define LOG_H

typedef struct log log_t;
struct log {
	char reqid[8];
	void (*debug) (const log_t * self, const char * fmt, ...);
	void (*info)  (const log_t * self, const char * fmt, ...);
	void (*error) (const log_t * self, const char * fmt, ...);
	void (*crit)  (const log_t * self, const char * fmt, ...);
};

log_t * init_log(void);
void destroy_log(log_t * log);

#endif // LOG_H
