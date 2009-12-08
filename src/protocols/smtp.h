#ifndef _PROTO_SMTP_H_
#define _PROTO_SMTP_H_

#include "configuration.h"

void* ph_smtp_create();
int ph_smtp_destroy(void*);

int ph_smtp_init(void* handler, struct configuration_t* c);
int ph_smtp_deinit(void* handler);
int ph_smtp_handle_payload_stc(void* handler, const char* payload);
int ph_smtp_handle_payload_cts(void* handler, const char* payload);
int ph_smtp_handle_packet(void* handler, const char* packet);


#endif
