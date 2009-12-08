#include "http.h"
#include "wrapper.h"

#include <stdlib.h>
#include <string.h>

struct ph_http {
	struct configuration_t* config;
};

void* ph_http_create()
{
	void* ret = malloc(sizeof(struct ph_http));
	return ret;
}

int ph_http_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_http_init(void* handler, struct configuration_t* c)
{
	struct ph_http* http = (struct ph_http*)handler;
	http->config = c;
	return 0;
}

int ph_http_deinit(void* handler)
{
	return 0;
}

int ph_http_handle_payload_stc(void* handler, const char* payload)
{
	return 0;
}

int ph_http_handle_payload_cts(void* handler, const char* payload)
{
	return 0;
}

int ph_http_handle_packet(void* handler, const char* packet)
{
	return 0;
}

int ph_http_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_http* http = (struct ph_http*)handler;
	if (conf_get_mode(http->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(http->config, "main", "http_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)21);
	}
	return 0;
}

