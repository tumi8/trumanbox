#ifndef _PROTO_IRC_H_
#define _PROTO_IRC_H_

#include "configuration.h"
#include <netinet/in.h>

void* ph_irc_create();
int ph_irc_destroy(void*);

int ph_irc_init(void* handler, struct configuration_t* c);
int ph_irc_deinit(void* handler);
int ph_irc_handle_payload_stc(void* handler, connection_t* conn, const char* payload, ssize_t* len);
int ph_irc_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len);
int ph_irc_handle_packet(void* handler, const char* packet, ssize_t len);
int ph_irc_determine_target(void* handler, struct sockaddr_in*);

#endif
