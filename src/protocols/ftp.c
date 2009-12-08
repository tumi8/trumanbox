#include "ftp.h"

#include <stdlib.h>

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

int ph_ftp_handle_payload_stc(void* handler, const char* payload)
{
	return 0;
}

int ph_ftp_handle_payload_cts(void* handler, const char* payload)
{
	return 0;
}

int ph_ftp_handle_packet(void* handler, const char* packet)
{
	return 0;
}

int ph_ftp_determine_target(void* handler, struct sockaddr_in* addr)
{
	return 0;
}

