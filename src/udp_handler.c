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

struct udp_handler_t {
	int udpfd;
        operation_mode_t mode; // get from config
        struct configuration_t* config;
        int sock;
        connection_t* connection;
        int connectedToFinal;
        struct proto_identifier_t* pi;
        struct proto_handler_t** ph;
};


struct udp_handler_t* udphandler_create(int udpfd, struct configuration_t* config, connection_t* c, struct proto_identifier_t* pi, struct proto_handler_t** ph)
{
	struct udp_handler_t* ret = (struct udp_handler_t*)malloc(sizeof(struct udp_handler_t));
	ret->udpfd = udpfd;
        ret->config = config;
        ret->mode = conf_get_mode(config);
        ret->connection = c;
        ret->pi = pi;
        ret->ph = ph;
        ret->connectedToFinal = 0;
	return ret;
}


void udphandler_destroy(struct udp_handler_t* u)
{
	ph_destroy(u->ph);
	pi_destroy(u->pi);
	free(u);
}


void udphandler_determine_target(struct udp_handler_t* udph, protocols_app app_proto, struct sockaddr_in* targetServAddr)
{
	// Determine target service address. This address depends on the chosen program mode
	switch (udph->mode) {
	case full_emulation:
		// Final target depends on the protocol, protocol identification
		// can only be performed on the payload from the client side.
		// If payload identification on the client side is not possible, 
		// we can only determine the target based on port numbers. 
		// This mode can therefore not determine applications which contain
		// initial server payload and do not use standard ports
		msg(MSG_DEBUG, "Determine target for full emulation mode...");
		if (app_proto == UNKNOWN_UDP) {
			bzero(udph->connection->dest, IPLENGTH);
		} else {
			udph->ph[app_proto]->determine_target(udph->ph[app_proto]->handler, targetServAddr);
			Inet_ntop(AF_INET, &targetServAddr->sin_addr, udph->connection->dest, IPLENGTH);
		}
		break;
	case half_proxy:
		// Final target depends on the protocol. Protcol identification
		// can be performed on both the intial client as well as the 
		// initial server string
		msg(MSG_DEBUG, "Determine target for half proxy mode...");
		if (app_proto == UNKNOWN_UDP) {
			msg(MSG_DEBUG,"app proto is unknown");
			bzero(udph->connection->dest, IPLENGTH);
		} else {
			msg(MSG_DEBUG,"app proto is not unknown %d",udph);
			udph->ph[app_proto]->determine_target(udph->ph[app_proto]->handler, targetServAddr);
			Inet_ntop(AF_INET, &targetServAddr->sin_addr, udph->connection->dest, IPLENGTH);
		}
		break;
	case full_proxy:
		// Connect to the original target (if this target is available)
		msg(MSG_DEBUG, "Determine target for full proxy mode ...%s",targetServAddr);
		bzero(&targetServAddr, sizeof(targetServAddr));
		msg(MSG_FATAL,"we have not implemented the parse_conntrack functions for UDP!!!");
		//targetServAddr->sin_family = AF_INET;
		/*msg(MSG_DEBUG,"dport: %d",udph->connection->dport);
		targetServAddr->sin_port = htons((uint16_t)udph->connection->dport);
		memcpy(udph->connection->dest, udph->connection->orig_dest, strlen(udph->connection->dest));
		Inet_pton(AF_INET, udph->connection->dest, &targetServAddr->sin_addr);
		*/
		break;
	default:
		msg(MSG_FATAL, "Unknown mode: This is an internal programming error!!!! Exiting!");
		exit(-1);
	}

}

void udphandler_run(struct udp_handler_t* udph)
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
	//udphandler_determine_target(udph, UNKNOWN_UDP, &targetServAddr);

	while (select(maxfdp, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(udph->udpfd, &rset)) {
			r = Recvfrom(udph->udpfd, payload, MAXLINE, 0, (SA *)  &cliaddr, &clilen);
			Inet_ntop(AF_INET, &cliaddr.sin_addr, udph->connection->source, sizeof(udph->connection->source));
			udph->connection->sport = ntohs(cliaddr.sin_port);
			msg(MSG_DEBUG,"conn source dispatching : %s port %d",udph->connection->source,udph->connection->sport);
			int tries_pars_ct = 0;

			
			// parse_conntrack fills in the remaining variables of connection
			while ( parse_conntrack(udph->connection) != 0 ) {
				msg(MSG_DEBUG, "could not parse conntrack table, trying again in 2sec...");
				sleep(2);
				tries_pars_ct++;
				if (tries_pars_ct > 5) {
					break;
				}
			}
			
			// all right we finished parsing the conntrack table and we extracted all necessary information (source ip/port, dest ip/port)
			bzero(&targetServAddr, sizeof(targetServAddr));
			msg(MSG_FATAL,"we have not implemented the parse_conntrack functions for UDP!!!");
			targetServAddr.sin_family = AF_INET;
			msg(MSG_DEBUG,"dport: %d",udph->connection->dport);
			targetServAddr.sin_port = htons((uint16_t)udph->connection->dport);
			//memcpy(udph->connection->dest, udph->connection->orig_dest, strlen(udph->connection->dest));
			Inet_pton(AF_INET, udph->connection->orig_dest, &targetServAddr.sin_addr);
/*	
			targetServAddr.sin_family = AF_INET;
			targetServAddr.sin_port = htons((uint16_t)udph->connection->dport);
			Inet_pton(AF_INET, udph->connection->orig_dest, &targetServAddr.sin_addr);
			strcpy(udph->connection->dest,udph->connection->orig_dest);
*/			

			msg(MSG_DEBUG,"num bytes rcvd: %d",r);
			proto_handler = udph->ph[UNKNOWN_UDP];
			if (proto_handler != NULL) {
				msg(MSG_DEBUG, "Sending payload to protocol handler ...");
				proto_handler->handle_payload_cts(proto_handler->handler, udph->connection, payload, &r);
				//Sendto(udph->udpfd, payload, r, 0, (SA *) &cliaddr, clilen);
				Sendto(udph->udpfd, payload, r, 0, (SA *) &targetServAddr, sizeof(targetServAddr));
			}
		memset(payload, 0, sizeof(payload));
		}
		FD_ZERO(&rset);
		FD_SET(udph->udpfd, &rset);
		maxfdp = udph->udpfd + 1;
	}
}

