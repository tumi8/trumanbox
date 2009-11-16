#ifndef _TCP_HANDLER_H_
#define _TCP_HANDLER_H_

#include "definitions.h"
#include "proto_ident.h"

struct tcp_handler_t;

struct tcp_handler_t* tcphandler_create(operation_mode_t mode, connection_t* c, int inconn, struct proto_identifier_t* pi);
void tcphandler_destroy(struct tcp_handler_t* t);

void tcphandler_run(struct tcp_handler_t* t);

#endif
