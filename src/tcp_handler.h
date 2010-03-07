#ifndef _TCP_HANDLER_H_
#define _TCP_HANDLER_H_

#include "definitions.h"
#include "protocols/proto_ident.h"
#include "protocols/proto_handler.h"

struct tcp_handler_t;

struct tcp_handler_t* tcphandler_create(struct configuration_t* config, connection_t* c, int inconn, struct proto_identifier_t* pi, struct proto_handler_t** ph);
void tcphandler_destroy(struct tcp_handler_t* t);

void tcphandler_run(struct tcp_handler_t* t);

#endif
