#include "udp_handler.h"
#include "dispatching.h"
#include "helper_net.h"
#include "wrapper.h"

#include <common/definitions.h>
#include <common/msg.h>
#include <common/configuration.h>

#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>


UdpHandler::UdpHandler(int udpfd, const Configuration& config, connection_t* c, ProtoIdent* ident, std::map<protocols_app, ProtoHandler*> protoHandlers)
	:config(config)
{
	this->udpfd = udpfd;
        this->mode = config.getMode();
        this->connection = c;
        this->protoIdent = protoIdent;
        this->protoHandlers = protoHandlers;
        this->connectedToFinal = 0;
}


UdpHandler::~UdpHandler()
{
	ph_destroy(this->protoHandlers);
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
	ProtoHandler* protoHandler;
	
	struct timeval tv;
	int maxfdp;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	ssize_t r;

	FD_ZERO(&rset);
	FD_SET(this->udpfd, &rset);
	
	tv.tv_sec = 300;
	tv.tv_usec = 0;
	
	bzero(payload,MAXLINE);
	maxfdp = this->udpfd + 1;
	clilen = sizeof(cliaddr);

	while (select(maxfdp, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(this->udpfd, &rset)) {
			r = Recvfrom(this->udpfd, payload, MAXLINE, 0, (SA *)  &cliaddr, &clilen);
			Inet_ntop(AF_INET, &cliaddr.sin_addr, this->connection->source, sizeof(this->connection->source));
			this->connection->sport = ntohs(cliaddr.sin_port);
			msg(MSG_DEBUG,"conn source dispatching : %s port %d",this->connection->source,this->connection->sport);
			int tries_pars_ct = 0;
			int found_dest = 1;
			
			// parse_conntrack fills in the remaining variables of connection
			while ( parse_conntrack(this->connection) != 0 ) {
				msg(MSG_DEBUG, "could not parse conntrack table, trying again in 2sec...");
				sleep(2);
				tries_pars_ct++;
				if (tries_pars_ct > 5) {
					found_dest = 0;
					break;
				}
			}
			
			if (found_dest) {
				msg(MSG_DEBUG,"src: '%s' dest '%s'",this->connection->source,this->connection->dest);
				determineTarget(UNKNOWN_UDP, &targetServAddr);
				msg(MSG_DEBUG,"src: '%s' dest '%s'",this->connection->source,this->connection->dest);
				// TOOD: fix this crappy IP!
				if(strstr(this->connection->dest,"192.168.27.255") != 0) {
					goto out;	
				}
			}		
			else {
				msg(MSG_DEBUG,"we have found no destination, therefore the sender gets an echo message");
				bzero(&targetServAddr, sizeof(targetServAddr));
				targetServAddr.sin_family = AF_INET;
				targetServAddr.sin_port = htons((uint16_t)this->connection->sport);
				bzero(this->connection->dest,IPLENGTH);
				bzero(this->connection->orig_dest,IPLENGTH);
				strcpy(this->connection->dest, this->connection->source);
				strcpy(this->connection->orig_dest,this->connection->source);
				//memcpy(this->connection->orig_dest, this->connection->source, strlen(this->connection->source));
				Inet_pton(AF_INET, this->connection->dest, &targetServAddr.sin_addr);
				Inet_pton(AF_INET, this->connection->orig_dest, &targetServAddr.sin_addr);
			}
			

			msg(MSG_DEBUG,"num bytes rcvd: %d",r);
			protoHandler = this->protoHandlers[UNKNOWN_UDP];
			msg(MSG_DEBUG, "Sending payload to protocol handler ...");
			Sendto(this->udpfd, payload, r, 0, (SA *) &targetServAddr, sizeof(targetServAddr));
			if (found_dest) {
				protoHandler->payloadClientToServer(this->connection, payload, &r);
			}
			else {
				protoHandler->payloadServerToClient(this->connection, payload, &r);
			}
			memset(payload, 0, sizeof(payload));
		}
		out:
		FD_ZERO(&rset);
		FD_SET(this->udpfd, &rset);
		maxfdp = this->udpfd + 1;
	}
}

