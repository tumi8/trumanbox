#ifndef _LOGGER_TRUMAN_H_
#define _LOGGER_TRUMAN_H_

#include "logger.h"

struct log_truman;

int lt_init(struct logger_t*);
int lt_deinit(struct logger_t*);
int lt_create_log(struct logger_t*);
int lt_finish_log(struct logger_t*);
int lt_log_text(struct logger_t*, connection_t* conn, const char* tag, const char* message);
int lt_log_struct(struct logger_t*, connection_t* conn, void* data);

#endif
