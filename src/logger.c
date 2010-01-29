#include "logger.h"
#include "configuration.h"

#include <stdlib.h>

struct logger_t* logger_create(struct configuration_t* config)
{
	struct logger_t* log = (struct logger_t*)malloc(sizeof(struct logger_t));

	log->config = config;
	

	return log;
}

void logger_destroy(struct logger_t* logger)
{
	free(logger);
}

