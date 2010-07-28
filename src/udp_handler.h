#ifndef _UDP_HANDLER_H_
#define _UDP_HANDLER_H_

#include "definitions.h"
#include "protocols/proto_ident.h"
#include "protocols/proto_handler.h"


struct udp_handler_t;

void udphandler_determine_target(struct udp_handler_t* udph, protocols_app app_proto, struct sockaddr_in* targetServAddr);

struct udp_handler_t* udphandler_create(int udpfd, struct configuration_t* config, connection_t* c,  struct proto_identifier_t* pi, struct proto_handler_t** ph);

void udphandler_destroy(struct udp_handler_t* u);

void udphandler_run(struct udp_handler_t* u);

#endif
