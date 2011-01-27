#include "tcp_handler.h"
#include <sys/types.h>
#include "ssl_handler.h"
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include "definitions.h"
#include "helper_net.h"
#include "msg.h"
#include "configuration.h"
#include "wrapper.h"
#include "process_manager.h"
#include "protocols/proto_handler.h"



struct tcp_handler_t* tcphandler_create(struct configuration_t* config, connection_t* c, int inconn, struct proto_identifier_t* pi, struct proto_handler_t** ph)
{
	const char* sslactive = conf_get(config, "ssl", "mitm_active");
	const char* nepenthesactive = conf_get(config, "nepenthes", "active");
	struct tcp_handler_t* ret = (struct tcp_handler_t*)malloc(sizeof(struct tcp_handler_t));
	if (nepenthesactive) {
		ret->nepenthesActive = atoi(nepenthesactive);
	} else {
		ret->nepenthesActive = 0;
	}
	ret->config = config;
	if (sslactive) {
		ret->sslMitmActive = atoi(sslactive);
	} else {
		ret->sslMitmActive = 0;
	}
	ret->mode = conf_get_mode(config);
	ret->connection = c;
	ret->inConnFd = inconn;
	ret->targetServiceFd = 0;
	ret->pi = pi;
	ret->ph = ph;
	ret->connectedToFinal = 0;
	return ret;
}


void tcphandler_destroy(struct tcp_handler_t* t)
{
	ph_destroy(t->ph);
	pi_destroy(t->pi);
	free(t);
}

void tcphandler_determine_target(struct tcp_handler_t* tcph, protocols_app app_proto, struct sockaddr_in* targetServAddr)
{
	// Determine target service address. This address depends on the chosen program mode
	switch (tcph->mode) {
	case full_emulation:
		// Final target depends on the protocol, protocol identification
		// can only be performed on the payload from the client side.
		// If payload identification on the client side is not possible, 
		// we can only determine the target based on port numbers. 
		// This mode can therefore not determine applications which contain
		// initial server payload and do not use standard ports
		msg(MSG_DEBUG, "Determine target for full emulation mode...");
		tcph->connection->destOffline = 1;
		if (app_proto == UNKNOWN) {
			bzero(tcph->connection->dest, IPLENGTH);
		} else {
			tcph->ph[app_proto]->determine_target(tcph->ph[app_proto]->handler, targetServAddr);
			Inet_ntop(AF_INET, &targetServAddr->sin_addr, tcph->connection->dest, IPLENGTH);
			tcph->connection->dport = ntohs(targetServAddr->sin_port);
		}
		break;
	case half_proxy:
		// Final target depends on the protocol. Protcol identification
		// can be performed on both the intial client as well as the 
		// initial server string
/*
		msg(MSG_DEBUG, "Determine target for half proxy mode...");
					break;*/
	case full_proxy:
		// Connect to the original target (if this target is available)
		msg(MSG_DEBUG, "Determine target for full proxy mode ...");
		bzero(targetServAddr, sizeof(*targetServAddr));
		targetServAddr->sin_family = AF_INET;
		targetServAddr->sin_port = htons((uint16_t)tcph->connection->dport);
		msg(MSG_DEBUG,"determine target orig_dest:  %s dest: %s",tcph->connection->orig_dest,tcph->connection->dest);
		Inet_pton(AF_INET, tcph->connection->dest, &targetServAddr->sin_addr);
		break;
	default:
		msg(MSG_FATAL, "Unknown mode: This is an internal programming error!!!! Exiting!");
		exit(-1);
	}

}


int tcphandler_handle_ssl(struct tcp_handler_t* tcph)
{
	if (tcph->sslMitmActive == 0 || tcph->mode == full_emulation) {
		msg(MSG_DEBUG,"SSL MITM Mode inactive");
		return 0;
}
// We have found a SSL request from the client
	pid_t childpid;
	struct ssl_handler_t* sh = sslhandler_create(tcph);
	msg(MSG_DEBUG,"port %d",sh->sslServerPort);


	msg(MSG_DEBUG,"dest: %s orig_dest %s dport: %d",tcph->connection->dest,tcph->connection->orig_dest,tcph->connection->dport);

	strcpy(tcph->connection->dest,"127.0.0.1");
	tcph->connection->dport = sh->sslServerPort;

	msg(MSG_DEBUG,"dest: %s orig_dest %s dport: %d",tcph->connection->dest,tcph->connection->orig_dest,tcph->connection->dport);
	if ( (childpid = pm_fork_temporary()) == 0) {        // child process
		msg(MSG_DEBUG, "Forked SSL handler with pid %d", getpid());
		sslhandler_run(sh);
		sslhandler_destroy(sh);
		Exit(0);
	}
	else {	 // parent process
		// wait until the child process is finished
		sleep(2);
	}


	return 0;			
	
}

int tcphandler_handle_unknown(struct tcp_handler_t* tcph, struct sockaddr_in* targetServAddr)
{
	protocols_app app_proto = UNKNOWN;
	if (tcph->nepenthesActive != 0)  {
		
		msg(MSG_DEBUG,"nepenthes is active with port %d",tcph->connection->dport);
		switch (tcph->connection->dport) {
						case 42:
						case 135:
						case 139:
						case 445:
						case 1023:
						case 1025:
						case 1433:
						case 3127:
						case 3128:
							targetServAddr->sin_family = AF_INET;
							Inet_pton(AF_INET, conf_get(tcph->config, "nepenthes", "nepenthes_redirect"), &targetServAddr->sin_addr);
							Inet_ntop(AF_INET, &targetServAddr->sin_addr, tcph->connection->dest, IPLENGTH);

							msg(MSG_DEBUG,"we changed target IP: %s",tcph->connection->dest);
							return -1;
							break;
						default:
							break;
			}

	}
	// check for nepenthes
/*	*/
	// try to save the day
	switch (tcph->mode) {
	case full_emulation:
		app_proto = tcph->pi->byport(tcph->pi, tcph->connection);
		// if portbased failed:
		if (app_proto == UNKNOWN) {
			// set nepenthes goal for ports:
			// 42, 135, 139, 445, 1023, 1025, 1433
			msg(MSG_ERROR, "Cannot identify application protocol in full_emulation mode!");

		}
		tcphandler_determine_target(tcph, app_proto, targetServAddr);
		break;
	case  half_proxy:

		tcphandler_determine_target(tcph, app_proto, targetServAddr);
		break;
	case full_proxy:
		// do nothing, we know the original target and will connect to it after this switch
		// target is already intitialized by now
		tcphandler_determine_target(tcph, app_proto, targetServAddr);
		break;
	default:
		msg(MSG_FATAL, "Unknown trumanbox mode: This is an internal programming error!!!! Exiting!");
		exit(-1);
	}
	// we have to know the target now!
	if (!tcph->connectedToFinal) {
		if (-1 == Connect(tcph->targetServiceFd, (struct sockaddr*)targetServAddr, sizeof(*targetServAddr))) {
			Close_conn(tcph->inConnFd, "Connection to targetservice could not be established");
			return -1;
		}
		tcph->connectedToFinal = 1;
	} else {
		if (app_proto == UNKNOWN) {
			// we are connected but the protocol is unknown. what to do know?
			msg(MSG_FATAL, "We are already connected");
			//exit(-1);
		}
	}

	return 0;
}

void tcphandler_run(struct tcp_handler_t* tcph)
{
	int maxfd;
	struct sockaddr_in targetServAddr;
	ssize_t r;
	fd_set rset;
	struct timeval tv;
	char payloadRead[MAXLINE];
	char payload[2*MAXLINE];
	protocols_app app_proto = UNKNOWN;
	struct proto_handler_t* proto_handler;

	bzero(payload, MAXLINE);
	tcph->targetServiceFd = Socket(AF_INET, SOCK_STREAM, 0);
/*
 * 	timeout is dangerous - because we cannot operate on this socket during the blocking call of connect -> suspectible of "Operation now in progress" - error
 * 	struct timeval timeoutTarget;
	timeoutTarget.tv_sec = 5; 
	timeoutTarget.tv_usec = 0;
	setsockopt(tcph->targetServiceFd,SOL_SOCKET,SO_SNDTIMEO,&timeoutTarget,sizeof(timeoutTarget));
*/	
	
	tcphandler_determine_target(tcph, UNKNOWN, &targetServAddr);
	msg(MSG_DEBUG,"finished determining target");
	msg(MSG_DEBUG,"count reads: %d",tcph->connection->countReads);
	FD_ZERO(&rset);
	//FD_SET(tcph->targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
	FD_SET(tcph->inConnFd, &rset);
	//maxfd = max(tcph->targetServiceFd, tcph->inConnFd) + 1;
	maxfd = tcph->inConnFd + 1;

	// wait 3 seconds for initial client payload
	// try to receive server payload if there is no 
	// payload from the client.
	tv.tv_sec = 5;
	tv.tv_usec = 0; 
	
	while (-1 != select(maxfd, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(tcph->targetServiceFd, &rset)) {
			// we received data from the internet server
			bzero(payloadRead,MAXLINE); // clean the old payload string, because we want to save new data
			bzero(payload,MAXLINE*2);

			msg(MSG_DEBUG, "Received data from target server!");
			r = read(tcph->targetServiceFd, payloadRead, MAXLINE - 1);
		
			if (!r) {
				msg(MSG_DEBUG, "Target closed the connection...");
				goto out;
			}
		
			//update the number of reads
			tcph->connection->countReads++;
			
			// copy the payload received in a new, larger char array because we maybe need the additional space for manipulating the server response
			memcpy(payload,payloadRead,r);

			if (tcph->connection->app_proto == UNKNOWN) {
				app_proto = tcph->pi->bypayload(tcph->pi, tcph->connection, payload, r);
				if (app_proto == UNKNOWN) {
					msg(MSG_FATAL, "We could not determine protocol after reading from source and target! But proceed anyway...");
				}
			}
			proto_handler = tcph->ph[tcph->connection->app_proto];
			proto_handler->handle_payload_stc(proto_handler->handler, tcph->connection, payload, &r);
			msg(MSG_DEBUG,"sending servermsg to infected machine");
			if (-1 == write(tcph->inConnFd, payload, r)) {
				msg(MSG_FATAL, "Could not write to target (infected machine)!");
				goto out;
			}
		} else if (FD_ISSET(tcph->inConnFd, &rset)) {
			bzero(payloadRead,MAXLINE); // clean the old payload string, because we want to save new data
			bzero(payload,MAXLINE*2);
			msg(MSG_DEBUG, "Received data from infected machine!");
			r = Read(tcph->inConnFd, payloadRead, MAXLINE - 1);
			msg(MSG_DEBUG,"read the data");
			if (r <= 0) {
				msg(MSG_DEBUG, "Infected machine closed the connection...");
				goto out;
			}
			else {
				msg(MSG_DEBUG,"r is not null '%d'",r);
			
			}
			//update the number of reads
			tcph->connection->countReads++;
				
			memcpy(payload,payloadRead,r);
	
			if (tcph->connection->app_proto == UNKNOWN) {
				app_proto = tcph->pi->bypayload(tcph->pi, tcph->connection, payload, r);
				if (app_proto == SSL_Proto) {
					tcphandler_handle_ssl(tcph);
				}
				if (app_proto == UNKNOWN) {
					tcphandler_handle_unknown(tcph, &targetServAddr);
				} 

				else if (!tcph->connectedToFinal) {
					msg(MSG_DEBUG, "Identified protocol. Connecting to target");
					tcphandler_determine_target(tcph, tcph->connection->app_proto, &targetServAddr);
					if ( -1 == Connect(tcph->targetServiceFd, (struct sockaddr*)&targetServAddr, sizeof(targetServAddr)) ) {
						if (tcph->mode == half_proxy) {
						msg(MSG_DEBUG,"dest is offline");
						// the connection to the original target / port failed, so we change the target to our local emulation server
						
							if (tcph->connection->app_proto == UNKNOWN) {
								bzero(tcph->connection->dest, IPLENGTH);
							} else {
								tcph->ph[tcph->connection->app_proto]->determine_target(tcph->ph[tcph->connection->app_proto]->handler, &targetServAddr);
								Inet_ntop(AF_INET, &(&targetServAddr)->sin_addr, tcph->connection->dest, IPLENGTH);
							}
							tcph->connection->destOffline = 1;	
							if (-1 == Connect(tcph->targetServiceFd, (struct sockaddr*)&targetServAddr, sizeof(targetServAddr))) {
								msg(MSG_FATAL,"Connection to emulation target not possible, abort...");
								goto out;
							}
						}
						else {
							Close_conn(tcph->inConnFd, "Connection to targetservice could not be established");
							goto out;
						}
					}
					tcph->connectedToFinal = 1;
				}
			} 
			
			proto_handler = tcph->ph[tcph->connection->app_proto];
			msg(MSG_DEBUG, "Sending payload to protocol handler ... ");
			proto_handler->handle_payload_cts(proto_handler->handler, tcph->connection, payload, &r);
			msg(MSG_DEBUG, "Sending payload to target server...%d bytes",r);
			if (-1 == write(tcph->targetServiceFd, payload, r)) {
				msg(MSG_FATAL, "Could not write to target server!");
				goto out;
			}
			msg(MSG_DEBUG, "Finished work on this message...");
		} else {
			// We received a timeout. There are now two possiblities:
			// 1.) We already identified the protocol: There is something wrong, as there should not be any timeout
			// 2.) We did not identify the payload: We need to perform some  actions to enable payload identification
			if (tcph->connection->app_proto != UNKNOWN) {
				msg(MSG_ERROR, "Connection timed out!");
				goto out; // exit function
			}
			msg(MSG_DEBUG,"Received a timeout???");
			tcphandler_handle_unknown(tcph, &targetServAddr);
		}
		FD_ZERO(&rset);
		FD_SET(tcph->targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
		FD_SET(tcph->inConnFd, &rset);
		maxfd = max(tcph->targetServiceFd, tcph->inConnFd) + 1;
		tv.tv_sec = 300;
		tv.tv_usec = 0; 
	}

out:
	Close_conn(tcph->inConnFd, "incoming connection, because we are done with this connection");
	Close_conn(tcph->targetServiceFd, "connection to targetservice, because we are done with this connection");
}

