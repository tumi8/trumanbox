#include "tcp_handler.h"

#include <stdlib.h>

struct tcp_handler_t {
	int sock;
};

struct tcp_handler_t* tcphandler_create()
{
	struct tcp_handler_t* ret = (struct tcp_handler_t*)malloc(sizeof(struct tcp_handler_t*));

	return ret;
}


void tcphandler_destroy(struct tcp_handler_t* t)
{
	free(t);
}

void tcphandlere_run()
{

}

