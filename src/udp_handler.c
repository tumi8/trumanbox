#include "udp_handler.h"
#include "definitions.h"
#include "helper_net.h"
#include "wrapper.h"

#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

struct udp_handler_t {
	int udpfd;
};

struct udp_handler_t* udphandler_create(int udpfd)
{
	struct udp_handler_t* ret = (struct udp_handler_t*)malloc(sizeof(struct udp_handler_t*));
	ret->udpfd = udpfd;
	return ret;
}


void udphandler_destroy(struct udp_handler_t* u)
{
	free(u);
}

void udphandler_run(struct udp_handler_t* u)
{
	fd_set rset;
	char payload[MAXLINE];
	struct timeval tv;
	int maxfdp;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	ssize_t r;

	FD_ZERO(&rset);
	FD_SET(u->udpfd, &rset);
	
	tv.tv_sec = 300;
	tv.tv_usec = 0;
	
	maxfdp = u->udpfd + 1;
	clilen = sizeof(cliaddr);
	
	while (select(maxfdp, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(u->udpfd, &rset)) {
			r = Recvfrom(u->udpfd, payload, MAXLINE, 0, (SA *)  &cliaddr, &clilen);
			Sendto(u->udpfd, payload, r, 0, (SA *) &cliaddr, clilen);
			memset(payload, 0, sizeof(payload));
		}
		FD_ZERO(&rset);
		FD_SET(u->udpfd, &rset);
	}
}

