#ifndef _LOGGER_POSTGRES_H_
#define _LOGGER_POSTGRES_H_

#include "logger.h"

struct log_postgres;

int lpg_init(struct logger_t*); // will be called after logger object is being created
int lpg_deinit(struct logger_t*); // will be called after logger object is being destroyed
int lpg_create_log(struct logger_t*);
int lpg_finish_log(struct logger_t*);
int lpg_log_text(struct logger_t*, connection_t* conn, const char* tag, const char* message);
int lpg_log_struct(struct logger_t*, connection_t* conn, void* data);

#endif

