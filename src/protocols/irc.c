#include "irc.h"

#include <stdlib.h>
#include <string.h>
#include "wrapper.h"

#include "logger.h"

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
	return logger_get()->log(logger_get(), conn, "content-stc", payload);
}

int ph_irc_handle_payload_cts(void* handler, connection_t* conn,  const char* payload, size_t* len)
{
	return logger_get()->log(logger_get(), conn, "content-cts", payload);
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

