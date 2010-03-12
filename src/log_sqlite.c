#include "log_sqlite.h"
#include "configuration.h"
#include "msg.h"
#include "definitions.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sqlite3.h>

#define TABLE_NAME_LENGTH 100
#define MAX_STATEMENT 1000

static char statement[MAX_STATEMENT];

struct lsq_data {
	const char* db_file;
	sqlite3* db;
	char tables[UNKNOWN + 1][TABLE_NAME_LENGTH];
};


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  NotUsed=0;

  for(i=0; i<argc; i++){
    msg(MSG_DEBUG, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  return 0;
}


int create_db(struct lsq_data* data)
{
	char* err = 0;
	int rc;
	int i;

	for (i = 0; i <= UNKNOWN; ++i) {
		snprintf(statement, MAX_STATEMENT, "DROP TABLE %s;", data->tables[i]);
		rc = sqlite3_exec(data->db, statement, callback, 0, &err);
		//if (rc != SQLITE_OK) {
		//	msg(MSG_ERROR, "Cannot drop table %s: %s", data->tables[i], err);
		//	return -1;
		//}
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (domain TEXT, original INTEGER, returned INTEGER);", data->tables[DNS]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
        if (rc != SQLITE_OK) {
                msg(MSG_ERROR, "Error createing table currentdns: %s", err);
                return -1;
        }

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (domain TEXT, query STRING);", data->tables[HTTP]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currenthttp: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (domain TEXT);", data->tables[FTP]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentftp: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (server TEXT, rctp TEXT);", data->tables[SMTP]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentsmtp: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (server TEXT, keyword TEXT, value TEXT);", data->tables[IRC]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentirc: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (content TEXT);", data->tables[UNKNOWN]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentunknown: %s", err);
		return -1;
	}
	return 0;
}

int lsq_init(struct logger_t* logger)
{
	struct lsq_data* data = (struct lsq_data*)malloc(sizeof(struct lsq_data));
	struct stat buf;
	int new_db = 0;

	strncpy(data->tables[SMTP], "currentsmtp", TABLE_NAME_LENGTH);
	strncpy(data->tables[FTP], "currentftp", TABLE_NAME_LENGTH);
	strncpy(data->tables[FTP_anonym], "currentftp", TABLE_NAME_LENGTH);
	strncpy(data->tables[FTP_data], "currentftp", TABLE_NAME_LENGTH);
	strncpy(data->tables[HTTP], "currenthttp", TABLE_NAME_LENGTH);
	strncpy(data->tables[IRC], "currentirc", TABLE_NAME_LENGTH);
	strncpy(data->tables[DNS], "currentdns", TABLE_NAME_LENGTH);
	strncpy(data->tables[UNKNOWN], "currentunknown", TABLE_NAME_LENGTH);

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

	// TODO: create permanent tables
	//if (new_db) {
	//	create_db(data->db);
	//}

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
	struct lsq_data* data = (struct lsq_data*)logger->data;
	return create_db(data);
}

int lsq_finish_log(struct logger_t* logger)
{
	//struct lsq_data* data = (struct lsq_data*)logger->data;
	return 0;
}

int lsq_log_text(struct logger_t* logger, connection_t* conn, const char* tag, const char* message)
{
	struct lsq_data* data = (struct lsq_data*)logger->data;
	char* err;

	switch (conn->app_proto) {
	case SMTP:
		snprintf(statement, MAX_STATEMENT, "INSERT into %s (%s %s);", data->tables[SMTP], conn->orig_dest, message);
		sqlite3_exec(data->db, statement, callback, 0, &err);
		break;
	case FTP:
	case FTP_anonym:
	case FTP_data:

		break;
	case HTTP:
		
		break;
	case IRC:

		break;
	case DNS:

		break;

	case UNKNOWN:
		
		break;
	};
	return 0;
}

