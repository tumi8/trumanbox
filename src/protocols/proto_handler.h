#ifndef _PROTO_HANDLER_H_
#define _PROTO_HANDLER_H_

#include "definitions.h"

#include <netinet/in.h>

struct configuration_t;

typedef int (ph_init)(void* handler, struct configuration_t* c);
typedef int (ph_deinit)(void* handler);
typedef int (ph_handle_payload)(void* handler, connection_t* conn, const char* payload, size_t* len);
typedef int (ph_handle_packet)(void* handler, const char* packet, size_t len);
typedef int (ph_determine_target)(void* handler, struct sockaddr_in* addr);

struct proto_handler_t {
	void* handler;
	ph_init* init;
	ph_deinit* deinit;
	ph_handle_payload* handle_payload_stc;
	ph_handle_payload* handle_payload_cts;
	ph_handle_packet* handle_packet;
	ph_determine_target* determine_target;
};

/* Returns an array of protocolhandler. The size of the array is 
 * sizof(protocols_app) (where protcols_app is defined in definitions.h).
 * Each element of result[i] is a protocol handler for protocols_app[i]
 */
struct proto_handler_t** ph_create(struct configuration_t* c);


/* Frees all protcol handlers. This function will call protcolhandler_t->deinit for
 * all associated protocol handlers. 
 */
int ph_destroy(struct proto_handler_t**);

#endif
