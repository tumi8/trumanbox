#ifndef _TCP_HANDLER_H_
#define _TCP_HANDLER_H_

#include "definitions.h"
#include "proto_ident.h"
#include "proto_handler.h"

struct tcp_handler_t;

// TODO: make this somewhat smoother ...
struct tcp_handler_t* tcphandler_create(operation_mode_t mode, connection_t* c, int inconn, struct proto_identifier_t* pi, struct protohandler_t** ph);
void tcphandler_destroy(struct tcp_handler_t* t);

void tcphandler_run(struct tcp_handler_t* t);

#endif
