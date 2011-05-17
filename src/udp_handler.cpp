#include "configuration.h"
#include "dispatching.h"
#include <arpa/inet.h>
#include "udp_handler.h"
#include "definitions.h"
#include "helper_net.h"
#include "wrapper.h"
#include "msg.h"
#include <stdlib.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>


UdpHandler::UdpHandler(int udpfd, const Configuration& config, connection_t* c, ProtoIdent* ident, struct proto_handler_t** ph);
	:config(config)
{
	this->udpfd = udpfd;
        this->mode = config.getMode();
        this->connection = c;
        this->protoIdent = protoIdent;
        this->ph = ph;
        this->connectedToFinal = 0;
}


UdpHandler::~UdpHandler()
{
	ph_destroy(this->ph);
}


void UdpHandler::determineTarget(protocols_app app_proto, struct sockaddr_in* targetServAddr)
{
	// Determine target service address. This address depends on the chosen program mode
	switch (this->mode) {
	case full_emulation:
		// Final target depends on the protocol, protocol identification
		// can only be performed on the payload from the client side.
		// If payload identification on the client side is not possible, 
		// we can only determine the target based on port numbers. 
		// This mode can therefore not determine applications which contain
		// initial server payload and do not use standard ports
		msg(MSG_DEBUG, "Determine target for full emulation mode...");
		// set goal to nepenthes for port 1434 sql slammer protection
		bzero(this->connection->dest,IPLENGTH);
		bzero(targetServAddr, sizeof(targetServAddr));
		targetServAddr->sin_family = AF_INET;
		targetServAddr->sin_port = htons((uint16_t)this->connection->dport);
		memcpy(this->connection->dest, this->connection->source, strlen(this->connection->source));
		Inet_pton(AF_INET, this->connection->dest, &targetServAddr->sin_addr);
		break;
	case half_proxy:

	case full_proxy:
		// Connect to the original target (if this target is available)
		msg(MSG_DEBUG, "Determine target for full proxy mode ...");
		bzero(targetServAddr, sizeof(targetServAddr));
		targetServAddr->sin_family = AF_INET;
		targetServAddr->sin_port = htons((uint16_t)this->connection->dport);
		memcpy(this->connection->dest, this->connection->orig_dest, strlen(this->connection->orig_dest));
		Inet_pton(AF_INET, this->connection->dest, &targetServAddr->sin_addr);
		
		break;
	default:
		msg(MSG_FATAL, "Unknown mode: This is an internal programming error!!!! Exiting!");
		exit(-1);
	}

}

void UdpHandler::run()
{
	fd_set rset;
	char payload[MAXLINE];
	struct sockaddr_in targetServAddr;
	struct proto_handler_t* proto_handler;
	
	struct timeval tv;
	int maxfdp;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	ssize_t r;

	FD_ZERO(&rset);
	FD_SET(udph->udpfd, &rset);
	
	tv.tv_sec = 300;
	tv.tv_usec = 0;
	
	bzero(payload,MAXLINE);
	maxfdp = udph->udpfd + 1;
	clilen = sizeof(cliaddr);

	while (select(maxfdp, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(udph->udpfd, &rset)) {
			r = Recvfrom(udph->udpfd, payload, MAXLINE, 0, (SA *)  &cliaddr, &clilen);
			Inet_ntop(AF_INET, &cliaddr.sin_addr, udph->connection->source, sizeof(udph->connection->source));
			udph->connection->sport = ntohs(cliaddr.sin_port);
			msg(MSG_DEBUG,"conn source dispatching : %s port %d",udph->connection->source,udph->connection->sport);
			int tries_pars_ct = 0;
			int found_dest = 1;
			
			// parse_conntrack fills in the remaining variables of connection
			while ( parse_conntrack(udph->connection) != 0 ) {
				msg(MSG_DEBUG, "could not parse conntrack table, trying again in 2sec...");
				sleep(2);
				tries_pars_ct++;
				if (tries_pars_ct > 5) {
					found_dest = 0;
					break;
				}
			}
			
			if (found_dest) {
				msg(MSG_DEBUG,"src: '%s' dest '%s'",udph->connection->source,udph->connection->dest);
				udphandler_determine_target(udph, UNKNOWN_UDP, &targetServAddr);
				msg(MSG_DEBUG,"src: '%s' dest '%s'",udph->connection->source,udph->connection->dest);
				if(strstr(udph->connection->dest,"192.168.27.255") != 0) {
					goto out;	
					}
			}		
			else {
				msg(MSG_DEBUG,"we have found no destination, therefore the sender gets an echo message");
				bzero(&targetServAddr, sizeof(targetServAddr));
				targetServAddr.sin_family = AF_INET;
				targetServAddr.sin_port = htons((uint16_t)udph->connection->sport);
				bzero(udph->connection->dest,IPLENGTH);
				bzero(udph->connection->orig_dest,IPLENGTH);
				strcpy(udph->connection->dest, udph->connection->source);
				strcpy(udph->connection->orig_dest,udph->connection->source);
				//memcpy(udph->connection->orig_dest, udph->connection->source, strlen(udph->connection->source));
				Inet_pton(AF_INET, udph->connection->dest, &targetServAddr.sin_addr);
				Inet_pton(AF_INET, udph->connection->orig_dest, &targetServAddr.sin_addr);
			}
			

			msg(MSG_DEBUG,"num bytes rcvd: %d",r);
			proto_handler = udph->ph[UNKNOWN_UDP];
			msg(MSG_DEBUG, "Sending payload to protocol handler ...");
			Sendto(udph->udpfd, payload, r, 0, (SA *) &targetServAddr, sizeof(targetServAddr));
			if (found_dest) {
				proto_handler->handle_payload_cts(proto_handler->handler, udph->connection, payload, &r);
			}
			else {
				proto_handler->handle_payload_stc(proto_handler->handler, udph->connection, payload, &r);
			}
			memset(payload, 0, sizeof(payload));
		}
		out:
		FD_ZERO(&rset);
		FD_SET(udph->udpfd, &rset);
		maxfdp = udph->udpfd + 1;
	}
}

