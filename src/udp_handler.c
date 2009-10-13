#include "udp_handler.h"

#include <stdlib.h>

struct udp_handler_t {
	int sock;
};

struct udp_handler_t* udphandler_create()
{
	struct udp_handler_t* ret = (struct udp_handler_t*)malloc(sizeof(struct udp_handler_t*));

	return ret;
}


void udphandler_destroy(struct udp_handler_t* u)
{
	free(u);
}

void udphandlere_run()
{

}

