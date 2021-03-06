#include "logger.h"
#include "configuration.h"
#include "msg.h"

#include "log_truman.h"
#include "log_sqlite.h"
#include "log_postgres.h"
#include <stdlib.h>
#include <string.h>

struct logger_t* global_logger = NULL;

int logger_create(struct configuration_t* config)
{
	if (global_logger) {
		msg(MSG_ERROR, "Logging module has already been created!");
		return -1;
	}
	global_logger = (struct logger_t*)malloc(sizeof(struct logger_t));

	global_logger->config = config;

	// check with logger to instantiate
	const char* logger = conf_get(config, "logging", "type");
	if (!logger) {
		msg(MSG_FATAL, "No logger type given in configuration file!");
		return -1;
	}
	
	if (!strcmp(logger, "truman")) {
		global_logger->init = lt_init;
		global_logger->deinit = lt_deinit;
		global_logger->create_log = lt_create_log;
		global_logger->finish_log = lt_finish_log;
		global_logger->log = lt_log_text;
		global_logger->log_struct = lt_log_struct;
	} else if (!strcmp(logger, "sqlite")) {
		global_logger->init = lsq_init;
		global_logger->deinit = lsq_deinit;
		global_logger->create_log = lsq_create_log;
		global_logger->finish_log = lsq_finish_log;
		global_logger->log = lsq_log_text;
		global_logger->log_struct = lsq_log_struct;
	} 
	 else if (!strcmp(logger,"postgresql")) {
		global_logger->init = lpg_init;
		global_logger->deinit = lpg_deinit;
		global_logger->create_log = lpg_create_log;
		global_logger->finish_log = lpg_finish_log;
		global_logger->log = lpg_log_text;
		global_logger->log_struct = lpg_log_struct;
	} 
	else {
		msg(MSG_FATAL, "Unknown subsystem defined in configuration. Maybe even undefined!");
		free(global_logger);
		global_logger= NULL;
		return -1;
	}

	return global_logger->init(global_logger);

}

inline struct logger_t* logger_get()
{
	return global_logger;
}

int logger_destroy()
{
	if (!global_logger) {
		msg(MSG_ERROR, "Cannot destroy logging module as it has not been created");
		return -1;
	}
	global_logger->deinit(global_logger);
	free(global_logger);
	global_logger = NULL;
	
	return 0;
}

