#include "irc.h"

#include "wrapper.h"
#include "logger.h"
#include "msg.h"

#include <stdlib.h>
#include <string.h>

struct ph_irc {
	struct configuration_t* config;
};

void* ph_irc_create()
{
	void* ret = malloc(sizeof(struct ph_irc));
	return ret;
}

int ph_irc_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_irc_init(void* handler, struct configuration_t* c)
{
	struct ph_irc* irc = (struct ph_irc*)handler;
	irc->config = c;
	return 0;
}

int ph_irc_deinit(void* handler)
{
	return 0;
}

int ph_irc_handle_payload_stc(void* handler, connection_t* conn,  const char* payload, size_t* len)
{
	return logger_get()->log(logger_get(), conn, "server", payload);
}

int ph_irc_handle_payload_cts(void* handler, connection_t* conn,  const char* payload, size_t* len)
{
	/*char	username[50],
		password[50],
		caught_data[100];
	if (strncmp(payload, "USER ", 5) == 0) {
		msg(MSG_DEBUG, "we caught a USER token");
		strncpy(username, payload, sizeof(username)-1);
		msg(MSG_DEBUG, "and username is: %s", username);
		msg(MSG_DEBUG, "now we append the username: %s to our accountfile", username);
			//append_to_file(username, conn, IRC_COLLECTING_DIR);
	} else if (strncmp(payload, "PASS ", 5) == 0) {
		msg(MSG_DEBUG, "we caught a PASS token");
		strncpy(password, payload, sizeof(password)-1);
		msg(MSG_DEBUG, "now we append the pwd: %s to our accountfile", password);
		//append_to_file(password, conn, IRC_COLLECTING_DIR);
	} else if (strncmp(payload, "JOIN ", 5) == 0) {
		msg(MSG_DEBUG, "we caught a JOIN token");
		strncpy(caught_data, payload, sizeof(caught_data)-1);
		msg(MSG_DEBUG, "now we append: %s to our accountfile", caught_data);
		//append_to_file(caught_data, conn, IRC_COLLECTING_DIR);
	} else if (strncmp(payload, "WHO ", 4) == 0) {
		msg(MSG_DEBUG, "we caught a WHO token");
		strncpy(caught_data, payload, sizeof(caught_data)-1);
		msg(MSG_DEBUG, "now we append: %s to our accountfile", caught_data);
		//append_to_file(caught_data, conn, IRC_COLLECTING_DIR);
	} else if (strncmp(payload, "NICK ", 5) == 0) {
		msg(MSG_DEBUG, "we caught a NICK token");
		strncpy(caught_data, payload, sizeof(caught_data)-1);
		msg(MSG_DEBUG, "now we append: %s to our accountfile", caught_data);
		//append_to_file(caught_data, conn, IRC_COLLECTING_DIR);
	} else if (strncmp(payload, "PROTOCTL ", 9) == 0) {
		msg(MSG_DEBUG, "we caught a PROTOCTL token");
		strncpy(caught_data, payload, sizeof(caught_data)-1);
		msg(MSG_DEBUG, "now we append: %s to our accountfile\n", caught_data);
		//append_to_file(caught_data, conn, IRC_COLLECTING_DIR);
	} else if (strncmp(payload, "PING ", 5) == 0) {
		msg(MSG_DEBUG, "we caught a PING token\n");
		strncpy(caught_data, payload, sizeof(caught_data)-1);
		msg(MSG_DEBUG, "now we append: %s to our accountfile\n", caught_data);
		//append_to_file(caught_data, conn, IRC_COLLECTING_DIR);
	} else if (strncmp(payload, "MODE ", 5) == 0) {
		msg(MSG_DEBUG, "we caught a MODE token\n");
		strncpy(caught_data, payload, sizeof(caught_data)-1);
		msg(MSG_DEBUG, "now we append: %s to our accountfile\n", caught_data);
		//append_to_file(caught_data, conn, IRC_COLLECTING_DIR);
	}*/
	return logger_get()->log(logger_get(), conn, "client", payload);
}

int ph_irc_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_irc_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_irc* irc = (struct ph_irc*)handler;
	if (conf_get_mode(irc->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(irc->config, "irc", "irc_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)6667);
	}
	return 0;
}

