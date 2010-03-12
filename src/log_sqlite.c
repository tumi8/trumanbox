#include "log_sqlite.h"
#include "configuration.h"
#include "msg.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sqlite3.h>

struct lsq_data {
	const char* db_file;
	sqlite3* db;
};

int create_db(sqlite3* db)
{
	return 0;
}

int lsq_init(struct logger_t* logger)
{
	struct lsq_data* data = (struct lsq_data*)malloc(sizeof(struct lsq_data));
	struct stat buf;
	int new_db = 0;

	data->db_file = conf_get(logger->config, "logging", "db_file");
	if (!data->db_file) {
		msg(MSG_FATAL, "No db_file given in configuration file!");
		goto out1;
	}
	if (-1 == stat(data->db_file, &buf)) {
		if (errno == ENOENT) {
			msg(MSG_DEBUG, "Database %s does not exist. Will be created during startup!", data->db_file);
			new_db = 1;
		} else {
			msg(MSG_FATAL, "Error stating database %s: %s", data->db_file, strerror(errno));
			goto out1;
		}
	}

	if ( sqlite3_open(data->db_file, &data->db)) {
		msg(MSG_FATAL, "Cannot open sqlite database file %s: %s", data->db_file, sqlite3_errmsg(data->db));
		goto out2;
	}

	logger->data = (void*) data;
	return 0;

out2:
	sqlite3_close(data->db);
out1:
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

