#include "unknown.h"

#include <stdlib.h>

void* ph_unknown_create()
{
	return NULL;
}

int ph_unknown_destroy(void* handler)
{
	return 0;
}

int ph_unknown_init(void* handler, struct configuration_t* c)
{
	return 0;
}

int ph_unknown_deinit(void* handler)
{
	return 0;
}

int ph_unknown_handle_payload_stc(void* handler, connection_t* conn,  const char* payload, size_t len)
{
	return 0;
}

int ph_unknown_handle_payload_cts(void* handler, connection_t* conn,  const char* payload, size_t len)
{
	return 0;
}

int ph_unknown_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_unknown_determine_target(void* handler, struct sockaddr_in* addr)
{
	return 0;
}

