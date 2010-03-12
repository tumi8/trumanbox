#include "log_sqlite.h"
#include "configuration.h"
#include "msg.h"

#include <stdlib.h>

#include <sqlite3.h>

struct lsq_data {
	const char* db_file;
	sqlite3* db;
};

int lsq_init(struct logger_t* logger)
{
	struct lsq_data* data = (struct lsq_data*)malloc(sizeof(struct lsq_data));

	data->db_file = conf_get(logger->config, "logging", "db_file");

	if ( sqlite3_open(data->db_file, &data->db)) {
		msg(MSG_FATAL, "Cannot open sqlite database file %s: %s", data->db_file, sqlite3_errmsg(data->db));
		goto out;
	}

	logger->data = (void*) data;
	return 0;

out:
	sqlite3_close(data->db);
	free(data);
	return -1;
}

int lsq_deinit(struct logger_t* logger)
{
	struct lsq_data* data = (struct lsq_data*)logger->data;
	sqlite3_close(data->db);
	free(data);
	logger->data = NULL;
	return 0;
}

int lsq_create_log(struct logger_t* logger)
{
	//struct lsq_data* data = (struct lsq_data*)logger->data;
	return 0;
}

int lsq_finish_log(struct logger_t* logger)
{
	//struct lsq_data* data = (struct lsq_data*)logger->data;
	return 0;
}

int lsq_log_text(struct logger_t* logger, connection_t* conn, const char* tag, const char* message)
{
	//struct lsq_data* data = (struct lsq_data*)logger->data;
	return 0;
}

