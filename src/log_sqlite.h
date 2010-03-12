#ifndef _LOGGER_SQLITE_H_
#define _LOGGER_SQLITE_H_

#include "logger.h"

struct log_sqlite;

int lsq_init(struct logger_t*);
int lsq_deinit(struct logger_t*);
int lsq_create_log(struct logger_t*);
int lsq_finish_log(struct logger_t*);
int lsq_log_text(struct logger_t*, connection_t* conn, const char* tag, const char* message);

#endif
