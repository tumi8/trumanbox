#include "http.h"

#include <stdlib.h>

struct ph_http {
	int i; // platzhalter
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

