#include "log_truman.h"
#include "msg.h"
#include "configuration.h"

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct lt_data { 
	const char* basedir;
	const char* serv_response;
	const char* ftp;
	const char* irc;
	const char* smtp;
	const char* http;
	const char* dump;
};

int create_or_clean_dir(const char* dirname, mode_t mode)
{
	if (!dirname) {
		return -1;
	}
	if (mkdir(dirname, mode) < 0) {
		if (errno == EEXIST) {
			// clean it if there exist any log files in it
			DIR* dir = opendir(dirname);
			char file[256];
			struct dirent* ent; 
			if (!dir) {
				msg(MSG_FATAL, "Cannot open logdir %s: %s", dirname, strerror(errno));
				return -1;
			}
			while (NULL != (ent = readdir(dir))) {
				snprintf(file, 255, "%s/%s", dirname, ent->d_name);
				if (-1 == unlink(file)) {
					msg(MSG_FATAL, "Cannot clean file %s: %s", file, strerror(errno));
					return -1;
				}
			}
			closedir(dir);
		} else {
			msg(MSG_FATAL, "Logdir %s could not be created: %s", dirname, strerror(errno));
			return -1;
		}
	}
	return 0;
}

int lt_init(struct logger_t* logger)
{
	int ret = 0;
	struct lt_data* data = (struct lt_data*)malloc(sizeof(struct lt_data));
	data->basedir = conf_get(logger->config, "logging", "log_base");
	data->serv_response = conf_get(logger->config, "logging", "server_responses");
	data->ftp = conf_get(logger->config, "logging", "ftp");
	data->irc = conf_get(logger->config, "logging", "irc");
	data->smtp = conf_get(logger->config, "logging", "smtp");
	data->dump = conf_get(logger->config, "logging", "dump");

	if (!data->basedir)
		msg(MSG_FATAL, "No base logging directory given!");
	ret += create_or_clean_dir(data->basedir, PERMS);

	if (!data->serv_response)
		msg(MSG_FATAL, "No server response directory given!");
	ret += create_or_clean_dir(data->serv_response, PERMS);

	if (!data->ftp)
		msg(MSG_FATAL, "No FTP logging directory given!");
	ret += create_or_clean_dir(data->ftp, PERMS);
       
	if (!data->irc)
		msg(MSG_FATAL, "No IRC logging directory given!");
	ret += create_or_clean_dir(data->irc, PERMS);
	
	if (!data->smtp)
		msg(MSG_FATAL, "No SMTP logging directory given!");
	ret += create_or_clean_dir(data->smtp, PERMS);

	if (!data->http)
		msg(MSG_FATAL, "No HTTP logging dirctory given!");
	ret += create_or_clean_dir(data->http, PERMS);

	if (!data->dump)
		msg(MSG_FATAL, "No dump logging directory given!");
	ret += create_or_clean_dir(data->dump, PERMS);

	logger->data = (void*)data;

       	return ret;
}

int lt_deinit(struct logger_t* logger)
{
	struct lt_data* data = (struct lt_data*)logger->data;
	free(data);
	return 0;
}

int lt_create_log(struct logger_t* logger)
{
	return 0;
}

int lt_finish_log(struct logger_t* logger)
{
	return 0;
}

int lt_log_text(struct logger_t* logger, char* fmt, ...)
{
	return 0;
}


