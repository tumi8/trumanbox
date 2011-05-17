#ifndef _LOGGER_POSTGRES_H_
#define _LOGGER_POSTGRES_H_

#include "logbase.h"

class PostgresLogger : public LogBase {
public:
	PostgresLogger(const Configuration& config);
	virtual int logStruct(connection_t* conn, const char* tag, void* data);
	virtual int logText(connection_t* conn, const char* tag, const char* message);
	virtual int createLog();
	virtual int finishLog();
};

#endif

