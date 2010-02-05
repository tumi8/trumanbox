#include "logger.h"
#include "configuration.h"
#include "msg.h"

#include "log_truman.h"

#include <stdlib.h>
#include <string.h>


struct logger_t* logger_create(struct configuration_t* config)
{
	struct logger_t* log = (struct logger_t*)malloc(sizeof(struct logger_t));

	log->config = config;

	// check with logger to instantiate
	const char* logger = conf_get(config, "logger", "type");
	
	if (!strcmp(logger, "truman")) {
		log->init = lt_init;
		log->deinit = lt_deinit;
		log->create_log = lt_create_log;
		log->finish_log = lt_finish_log;
		log->log = lt_log_text;
	} else {
		msg(MSG_FATAL, "Unknown or not logging subsystem defined in configuration");
		goto out;
	}

	log->init(log);

	return log;

out:
	free(log);
	return NULL;
}

void logger_destroy(struct logger_t* logger)
{
	logger->deinit(logger);
	free(logger);
}

