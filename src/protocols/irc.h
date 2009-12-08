#ifndef _PROTO_IRC_H_
#define _PROTO_IRC_H_

#include "configuration.h"

void* ph_irc_create();
int ph_irc_destroy(void*);

int ph_irc_init(void* handler, struct configuration_t* c);
int ph_irc_deinit(void* handler);
int ph_irc_handle_payload_stc(void* handler, const char* payload);
int ph_irc_handle_payload_cts(void* handler, const char* payload);
int ph_irc_handle_packet(void* handler, const char* packet);


#endif
