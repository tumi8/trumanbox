#ifndef _PROTO_HTTP_POST_H_
#define _PROTO_HTTP_POST_H_

#include "configuration.h"
#include <netinet/in.h>
#include "msg.h"


void* ph_http_post_create();
int ph_http_post_destroy(void*);

int ph_http_post_init(void* handler, struct configuration_t* c);
int ph_http_post_deinit(void* handler);
int ph_http_post_handle_payload_stc(void* handler, connection_t* conn, const char* payload, size_t* len);
int ph_http_post_handle_payload_cts(void* handler, connection_t* conn, const char* payload, size_t* len);
int ph_http_post_handle_packet(void* handler, const char* packet, size_t len);
int ph_http_post_determine_target(void* handler, struct sockaddr_in*);
void save_request(const connection_t *conn, const char *cmd_str);


#endif
