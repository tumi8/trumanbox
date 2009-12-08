#include "irc.h"

#include <stdlib.h>

struct ph_irc {
	int i; // platzhalter
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
	return 0;
}

int ph_irc_deinit(void* handler)
{
	return 0;
}

int ph_irc_handle_payload_stc(void* handler, const char* payload)
{
	return 0;
}

int ph_irc_handle_payload_cts(void* handler, const char* payload)
{
	return 0;
}

int ph_irc_handle_packet(void* handler, const char* packet)
{
	return 0;
}

