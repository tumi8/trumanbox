#include "smtp.h"
#include "wrapper.h"

#include <stdlib.h>
#include <string.h>

struct ph_smtp {
	struct configuration_t* config;
};

void* ph_smtp_create()
{
	void* ret = malloc(sizeof(struct ph_smtp));
	return ret;
}

int ph_smtp_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_smtp_init(void* handler, struct configuration_t* c)
{
	struct ph_smtp* smtp = (struct ph_smtp*)handler;
	smtp->config = c;
	return 0;
}

int ph_smtp_deinit(void* handler)
{
	return 0;
}

int ph_smtp_handle_payload_stc(void* handler, const char* payload)
{
	return 0;
}

int ph_smtp_handle_payload_cts(void* handler, const char* payload)
{
	return 0;
}

int ph_smtp_handle_packet(void* handler, const char* packet)
{
	return 0;
}

int ph_smtp_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_smtp* smtp = (struct ph_smtp*)handler;
	if (conf_get_mode(smtp->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(smtp->config, "main", "smtp_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)21);
	}
	return 0;
}

