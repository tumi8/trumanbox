#ifndef _TCP_HANDLER_H_
#define _TCP_HANDLER_H_

#include "definitions.h"

struct tcp_handler_t;

struct tcp_handler_t* tcphandler_create(operation_mode_t mode, connection_t* c, int inconn);
void tcphandler_destroy(struct tcp_handler_t* t);

void tcphandler_run(struct tcp_handler_t* t);

#endif
