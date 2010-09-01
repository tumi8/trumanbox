#ifndef _SSL_HANDLER_H_
#define _SSL_HANDLER_H_

#include "definitions.h"
#include "tcp_handler.h"

struct ssl_handler_t {
	struct tcp_handler_t* tcphandler; 
	int serverSocket;
	int serverConnectionSocket;
	int clientSocket;
	char dest[IPLENGTH];
	u_int16_t destPort;
	u_int16_t sslServerPort;
};
struct ssl_handler_t* sslhandler_create(struct tcp_handler_t* tcph);
void sslhandler_destroy(struct ssl_handler_t* t);

void sslhandler_run(struct ssl_handler_t* t);

#endif
