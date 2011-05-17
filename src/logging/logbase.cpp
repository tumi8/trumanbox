#include "logbase.h"
#include "log_postgres.h"

#include <stdlib.h>
#include <string.h>

#include <common/msg.h>
#include <common/configuration.h>

LogBase* global_logger = NULL;

LogBase::LogBase(const Configuration& config)
	: config(config)
{

}

int logger_create(const Configuration& config)
{
	if (global_logger) {
		THROWEXCEPTION("Logging module has already been created!");
	}

	// check with logger to instantiate
	std::string logger_type = config.get("logging", "type");
	if (logger_type == "") {
		THROWEXCEPTION("No logger type given in configuration file!");
	}
	
	if (logger_type == "truman") {
		// TODO: Create logging object
		THROWEXCEPTION("The old trumanbox logger is currently not avaialbe ...");
	} else if (logger_type == "sqlite") {
		// TODO : create logging object
		THROWEXCEPTION("The old sqlite logger is currently not available ...");
	} 
	 else if (logger_type == "postgresql") {
		global_logger = new PostgresLogger(config);
	} 
	else {
		THROWEXCEPTION("Unknown subsystem defined in configuration. Maybe even undefined!");
	}

	return 0;
}

LogBase* logger_get()
{
	return global_logger;
}

int logger_destroy()
{
	if (!global_logger) {
		THROWEXCEPTION("Cannot destroy logging module as it has not been created");
	}
	delete global_logger;
	
	return 0;
}

