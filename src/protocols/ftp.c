#include "ftp.h"

#include <stdlib.h>
#include <string.h>

#include "wrapper.h"
#include "logger.h"
#include "helper_file.h"
#include "msg.h"

struct ph_ftp {
	struct configuration_t* config;
};

void* ph_ftp_create()
{
	void* ret = malloc(sizeof(struct ph_ftp));
	return ret;
}

int ph_ftp_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_ftp_init(void* handler, struct configuration_t* c)
{
	struct ph_ftp* ftp = (struct ph_ftp*)handler;
	ftp->config = c;
	return 0;
}

int ph_ftp_deinit(void* handler)
{
	return 0;
}

int ph_ftp_handle_payload_stc(void* handler, connection_t* conn, const char* payload, size_t len)
{
	return logger_get()->log(logger_get(), conn, "content-cts", payload);
}

int ph_ftp_handle_payload_cts(void* handler, connection_t* conn, const char* payload, size_t len)
{
	char 	*ptr,
		username[50],
		password[50];
	build_tree(conn, payload);
	switch(conn->app_proto) {
		case FTP:
			if (strncmp(payload, "USER ", 5) == 0) {
				msg(MSG_DEBUG, "we catched a USER token");
				strncpy(username, payload, sizeof(username)-1);
				msg(MSG_DEBUG, "and username is: %s", username);
				msg(MSG_DEBUG, "now we append the username: %s to our accountfile", username);
				//append_to_file(username, conn, FTP_COLLECTING_DIR);
		
				if (strncmp(payload, "USER anonymous", 14) == 0)
					return 0;
		
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_USER);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				len = strlen(payload);
			} else if (strncmp(payload, "PASS ", 5) == 0) {
				msg(MSG_DEBUG, "we catched a PASS token");
				strncpy(password, payload, sizeof(password)-1);
				msg(MSG_DEBUG, "now we append the pwd: %s to our accountfile", password);
				//append_to_file(password, conn, FTP_COLLECTING_DIR);
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_PASS);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				len = strlen(payload);
			}
			break;
		case FTP_anonym:
			if (strncmp(payload, "USER ", 5) == 0) {
				msg(MSG_DEBUG, "we catched a USER token");
				strncpy(username, payload, sizeof(username)-1);
				msg(MSG_DEBUG, "and username is: %s", username);
				msg(MSG_DEBUG, "now we append the username: %s to our accountfile", username);
				//append_to_file(username, conn, FTP_COLLECTING_DIR);		
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_USER);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				len = strlen(payload);
			} else if (strncmp(payload, "PASS ", 5) == 0) {
				msg(MSG_DEBUG, "we catched a PASS token");
				strncpy(password, payload, sizeof(password)-1);
				msg(MSG_DEBUG, "now we append the pwd: %s to our accountfile", password);
				//append_to_file(password, conn, FTP_COLLECTING_DIR);
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_PASS);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				len = strlen(payload);
			}
			break;	
		default:
			break;
	}
	return logger_get()->log(logger_get(), conn, "content-cts", payload);
}

int ph_ftp_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_ftp_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_ftp* ftp = (struct ph_ftp*)handler;
	if (conf_get_mode(ftp->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(ftp->config, "ftp", "ftp_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)21);
	}
	return 0;
}

