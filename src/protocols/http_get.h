#ifndef _PROTO_HTTP_GET_H_
#define _PROTO_HTTP_GET_H_

#include "configuration.h"
#include <netinet/in.h>

void* ph_http_get_create();
int ph_http_get_destroy(void*);

int ph_http_get_init(void* handler, struct configuration_t* c);
int ph_http_get_deinit(void* handler);
int ph_http_get_handle_payload_stc(void* handler, connection_t* conn, const char* payload, size_t* len);
int ph_http_get_handle_payload_cts(void* handler, connection_t* conn, const char* payload, size_t* len);
int ph_http_get_handle_packet(void* handler, const char* packet, size_t len);
int ph_http_get_determine_target(void* handler, struct sockaddr_in*);


#endif
