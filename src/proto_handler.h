#ifndef _PROTO_HANDLER_H_
#define _PROTO_HANDLER_H_

struct configuration_t;

typedef int (ph_init)(void* handler, struct configuration_t* c);
typedef int (ph_deinit)(void* handler);
typedef int (ph_handle_payload)(void* handler, const char* payload);
typedef int (ph_handle_packet)(void* handler, const char* packet);

struct protohandler_t {
	ph_init* init;
	ph_deinit* deinit;
	ph_handle_payload* handle_payload;
	ph_handle_packet* handle_packet;
};

/* Returns an array of protocolhandler. The size of the array is 
 * sizof(protocols_app) (where protcols_app is defined in definitions.h).
 * Each element of result[i] is a protocol handler for protocols_app[i]
 */
struct protohandler_t** ph_create(struct configuration_t* c);


/* Frees all protcol handlers. This function will NOT call protcolhandler_t->deinit. 
 * This is left to the user!
 */
int ph_destroy(struct protohandler_t**);

#endif
