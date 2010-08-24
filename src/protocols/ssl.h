#ifndef _PROTO_SSLPROTO_H_
#define _PROTO_SSLPROTO_H_

#include "configuration.h"
#include <netinet/in.h>

void* ph_ssl_create();
int ph_ssl_destroy(void*);

int ph_ssl_init(void* handler, struct configuration_t* c);
int ph_ssl_deinit(void* handler);
int ph_ssl_handle_payload_stc(void* handler, connection_t* conn, const char* payload, ssize_t* len);
int ph_ssl_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len);
int ph_ssl_handle_packet(void* handler, const char* packet, ssize_t len);
int ph_ssl_determine_target(void* handler, struct sockaddr_in*);


#endif
