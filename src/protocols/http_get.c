#include "http_get.h"
#include "wrapper.h"
#include "logger.h"
#include "helper_file.h"
#include "msg.h"

#include <stdlib.h>
#include <string.h>

struct ph_http_get {
	struct configuration_t* config;
};

void* ph_http_get_create()
{
	void* ret = malloc(sizeof(struct ph_http_get));
	return ret;
}

int ph_http_get_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_http_get_init(void* handler, struct configuration_t* c)
{
	struct ph_http_get* http_get = (struct ph_http_get*)handler;
	http_get->config = c;
	return 0;
}

int ph_http_get_deinit(void* handler)
{
	return 0;
}

int ph_http_get_handle_payload_stc(void* handler, connection_t* conn, const char* payload, size_t* len)
{
	return logger_get()->log(logger_get(), conn, "server", payload);
}

int ph_http_get_handle_payload_cts(void* handler, connection_t* conn, const char* payload, size_t* len)
{
	msg(MSG_DEBUG,"Entered http get handle payload");
	build_tree(conn, payload);
	return logger_get()->log(logger_get(), conn, "client", payload);
}

int ph_http_get_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_http_get_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_http_get* http_get = (struct ph_http_get*)handler;
	if (conf_get_mode(http_get->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(http_get->config, "http", "http_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)80);
	}
	return 0;
}

