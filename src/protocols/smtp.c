#include "smtp.h"

#include <stdlib.h>

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
	return 0;
}

