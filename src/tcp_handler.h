#ifndef _TCP_HANDLER_H_
#define _TCP_HANDLER_H_

struct tcp_handler_t;

struct tcp_handler_t* tcphandler_create();
void tcphandler_destroy(struct tcp_handler_t* t);

void tcphandlere_run();

#endif
