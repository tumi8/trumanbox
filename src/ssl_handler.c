#include "ssl_handler.h"
#include "definitions.h"
#include "helper_net.h"
#include "msg.h"
#include "configuration.h"
#include "wrapper.h"
#include "tcp_handler.h"
#include "protocols/proto_handler.h"
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>



struct ssl_handler_t* sslhandler_create(struct tcp_handler_t* tcph)
{
	struct ssl_handler_t* ret = (struct ssl_handler_t*)malloc(sizeof(struct ssl_handler_t));
	ret->tcphandler = tcph;
	ret->inConnFd = 0; 
	ret->targetServiceFd = 0;
	ret->sslServerPort = 0; // we set up the ssl server port 
	ret->destPort = tcph->connection->dport;
	memcpy(ret->dest,tcph->connection->dest,IPLENGTH);
	struct sockaddr_in sin;
        int val=1;
	    
	if((ret->sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		msg(MSG_FATAL,"Couldn't make socket");
		return NULL;
	}
			
	memset(&sin,0,sizeof(sin));
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(0); // let the kernel choose a port
	setsockopt(ret->sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
					
	if(bind(ret->sock,(struct sockaddr *)&sin,sizeof(sin))<0)
	{
		msg(MSG_FATAL,"Couldn't bind");
		return NULL;
	}

	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(server_addr);
	int rc = getsockname(ret->sock,(struct sockaddr *)&server_addr, &addrlen);
	rc++;
	ret->sslServerPort = ntohs(server_addr.sin_port);
	listen(ret->sock,20);  
	return ret;
}


void sslhandler_destroy(struct ssl_handler_t* t)
{
	free(t);
}


void sslhandler_run(struct ssl_handler_t* sslh)
{
	/*int maxfd;
	struct sockaddr_in targetServAddr;
	ssize_t r;
	fd_set rset;
	struct timeval tv;
		

	// TODO: SSL Server initialization
	
	// TODO: SSL Client initializiation

	
	
	FD_ZERO(&rset);
	//FD_SET(sslh->targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
	FD_SET(sslh->inConnFd, &rset);
	//maxfd = max(sslh->targetServiceFd, sslh->inConnFd) + 1;
	maxfd = sslh->inConnFd + 1;

	// wait 3 seconds for initial client payload
	// try to receive server payload if there is no 
	// payload from the client.
	tv.tv_sec = 5;
	tv.tv_usec = 0; 
	
	while (-1 != select(maxfd, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(sslh->targetServiceFd, &rset)) {
			// we received data from the internet server

			msg(MSG_DEBUG, "Received data from target server!");
			r = read(sslh->targetServiceFd, payloadRead, MAXLINE - 1);
			
			// TODO: Read payload and log to file

			// TODO: write payload to the client socket

			if (!r) {
				msg(MSG_DEBUG, "Target closed the connection...");
				goto out;
			}
			
		} else if (FD_ISSET(sslh->inConnFd, &rset)) {
			msg(MSG_DEBUG, "Received data from infected machine!");
			r = read(sslh->inConnFd, payloadRead, MAXLINE - 1);
			if (!r) {
				msg(MSG_DEBUG, "Infected machine closed the connection...");
				goto out;
			}

			// TODO: read payload and log to file
			//
			// TODO: write payload to the server socket
		} else {

		}
		FD_ZERO(&rset);
		FD_SET(sslh->targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
		FD_SET(sslh->inConnFd, &rset);
		maxfd = max(sslh->targetServiceFd, sslh->inConnFd) + 1;
		tv.tv_sec = 300;
		tv.tv_usec = 0; 
	}

out:
	Close_conn(sslh->inConnFd, "incoming connection, because we are done with this connection");
	Close_conn(sslh->targetServiceFd, "connection to targetservice, because we are done with this connection");
*/
}

