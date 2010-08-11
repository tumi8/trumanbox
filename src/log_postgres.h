#ifndef _LOGGER_POSTGRES_H_
#define _LOGGER_POSTGRES_H_

#include "logger.h"

struct log_postgres;

//int connect_to_db(); // connect to postgre sql server
//int execute_statement(char* stmt); // execute sql command
int lpg_init(struct logger_t*); // will be called after logger object is being created
int lpg_deinit(struct logger_t*); // will be called after logger object is being destroyed
int lpg_create_log(struct logger_t*);
int lpg_finish_log(struct logger_t*);
int lpg_log_text(struct logger_t*, connection_t* conn, const char* tag, const char* message);
int lpg_log_struct(struct logger_t*, connection_t* conn, const char* tag, void* data);

#endif

