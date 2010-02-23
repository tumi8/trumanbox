#include "log_truman.h"
#include "msg.h"
#include "configuration.h"
#include "wrapper.h"
#include "semaphore.h"

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#define MAX_FILE_NAME 256
#define DOCUMENT_ENCODING "ISO-8859-1"

struct lt_data { 
	const char* basedir;
	const char* serv_response;
	const char* ftp;
	const char* irc;
	const char* smtp;
	const char* http;
	const char* dump;
	const char* dns;
	const char* xml;
};

static int dump_to_xml_file(xmlTextWriterPtr xml, const char* directory)
{
	DIR* dir = opendir(directory);
	FILE* f;
	char file[MAX_FILE_NAME];
	char text[MAXLINE];
	struct dirent* ent;
	if (!dir) {
		msg(MSG_ERROR, "Cannot open logdir %s: %s", directory, strerror(errno));
		return -1;
	}
	while (NULL != (ent = readdir(dir))) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
			continue;
		}
		snprintf(file, MAX_FILE_NAME - 1, "%s/%s", directory, ent->d_name);

		f = fopen(file, "r");
		if (!f) {
			msg(MSG_ERROR, "Cannot open logfile %s for reading: %s", file, strerror(errno));
			continue;
		}

		while (fgets(text, MAXLINE, f)) {
			xmlTextWriterWriteElement(xml, BAD_CAST "Line", BAD_CAST text);
		}

		if (!feof(f)) {
			msg(MSG_ERROR, "Error reading from file %s: %s", file, strerror(errno));
		}
		
		fclose(f);
		
	}
	closedir(dir);
	return 0;
}

static int create_or_clean_dir(const char* dirname, mode_t mode)
{
	if (!dirname) {
		return -1;
	}
	if (mkdir(dirname, mode) < 0) {
		if (errno == EEXIST) {
			// clean it if there exist any log files in it
			DIR* dir = opendir(dirname);
			char file[MAX_FILE_NAME];
			struct dirent* ent; 
			if (!dir) {
				msg(MSG_FATAL, "Cannot open logdir %s: %s", dirname, strerror(errno));
				return -1;
			}
			while (NULL != (ent = readdir(dir))) {
				if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
					continue;
				snprintf(file, MAX_FILE_NAME -1, "%s/%s", dirname, ent->d_name);
				if (-1 == unlink(file)) {
					msg(MSG_DEBUG, "Cannot clean file %s: %s", file, strerror(errno));
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

static int init_directories(struct lt_data* data)
{
	int ret = 0;

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

	if (!data->dns)
		msg(MSG_FATAL, "No DNS logging directory given!");
	ret += create_or_clean_dir(data->dns, PERMS);

	return ret;
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
	data->http = conf_get(logger->config, "logging", "http");
	data->dump = conf_get(logger->config, "logging", "dump");
	data->dns = conf_get(logger->config, "logging", "dns");
	data->xml = conf_get(logger->config, "logging", "xml");

	ret = init_directories(data);

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
	return init_directories(logger->data);
}

int lt_finish_log(struct logger_t* logger)
{
	struct lt_data* data = (struct lt_data*)logger->data;
	xmlTextWriterPtr writer;
	int rc;
	//xmlChar* tmp;
	writer = xmlNewTextWriterFilename(data->xml, 0);
	if (!writer) {
		msg(MSG_ERROR, "Cannot create output xml file: %s", data->xml);
		return -1;
	}
	rc = xmlTextWriterStartDocument(writer, NULL, DOCUMENT_ENCODING, NULL);
	if (rc < 0) {
		msg(MSG_ERROR, "Could not start document %s", data->xml);
		return -1;
	}

	xmlTextWriterStartElement(writer, BAD_CAST "FTP");
	dump_to_xml_file(writer, data->ftp);
	xmlTextWriterEndElement(writer);

	xmlTextWriterStartElement(writer, BAD_CAST "IRC");
	dump_to_xml_file(writer, data->irc);
	xmlTextWriterEndElement(writer);

	xmlTextWriterStartElement(writer, BAD_CAST "SMTP");
	dump_to_xml_file(writer, data->smtp);
	xmlTextWriterEndElement(writer);

	xmlTextWriterStartElement(writer, BAD_CAST "HTTP");
	dump_to_xml_file(writer, data->http);
	xmlTextWriterEndElement(writer);

	xmlTextWriterStartElement(writer, BAD_CAST "DUMP");
	dump_to_xml_file(writer, data->dump);
	xmlTextWriterEndElement(writer);
	
	xmlTextWriterStartElement(writer, BAD_CAST "DNS");
	dump_to_xml_file(writer, data->dns);
	xmlTextWriterEndElement(writer);

	rc = xmlTextWriterEndDocument(writer);
	if (rc < 0) {
		msg(MSG_ERROR, "Error at testXmlWriterFilename");
		return -1;
	}
	xmlFreeTextWriter(writer);

	return 0;
}

int lt_log_text(struct logger_t* logger, connection_t* conn, protocols_app app, const char* message)
{
	// returns 1 on success, 0 if file exists, and -1 if something goes totally wrong ;-)
	char full_path[MAX_FILE_NAME];
	const char *ptr;
	int fd, w, r;
	const char* base_dir;
	struct lt_data* data = logger->data;

	switch (app) {
	case SMTP:
		base_dir = data->smtp;
		break;
	case FTP:
	case FTP_anonym:
	case FTP_data:
		base_dir = data->ftp;
		break;
	case HTTP:
		base_dir = data->http;
		break;
	case IRC:
		base_dir = data->irc;
		break;
	case DNS:
		base_dir = data->dns;
		break;
	case UNKNOWN:
	default:
		base_dir = data->dump;
		break;
	}

	sprintf(full_path, "%s/%s:%d", base_dir, conn->dest, conn->dport);

	msg(MSG_DEBUG, "now we open %s for appending the string: %s", full_path, message);

	semaph_alloc();

	if ( (fd = open(full_path, O_WRONLY | O_CREAT | O_EXCL | O_SYNC, S_IRUSR | S_IWUSR)) == -1) {
		if (errno == EEXIST) {
			if (-1 == (fd = open(full_path, O_WRONLY | O_APPEND | O_EXCL | O_SYNC))) {
				msg(MSG_ERROR, "cant open file %s: %s", full_path, strerror(errno));
				return -1;
			}
		}  else {
			msg(MSG_ERROR, "cant open file %s: %s", full_path, strerror(errno));
			return -1;
		}
	}

	r = strlen(message);
	ptr = message;

	while (r > 0 && (w = write(fd, ptr, r)) > 0) {
		ptr += w;
		r -= w;
	}

	Close(fd);	

	semaph_free();

	return 0;
}
