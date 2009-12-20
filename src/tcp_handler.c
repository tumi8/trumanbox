#include "tcp_handler.h"
#include "definitions.h"
#include "helper_net.h"
#include "msg.h"
#include "proto_handler.h"
#include "configuration.h"

#include <stdlib.h>

struct tcp_handler_t {
	operation_mode_t mode; // get from config
	struct configuration_t* config;
	int sock;
	connection_t* connection;
	int inconnfd;
	struct proto_identifier_t* pi;
	struct proto_handler_t** ph;
};

struct tcp_handler_t* tcphandler_create(struct configuration_t* config, connection_t* c, int inconn, struct proto_identifier_t* pi, struct proto_handler_t** ph)
{
	struct tcp_handler_t* ret = (struct tcp_handler_t*)malloc(sizeof(struct tcp_handler_t*));
	ret->config = config;
	ret->mode = conf_get_mode(config);
	ret->connection = c;
	ret->inconnfd = inconn;
	ret->pi = pi;
	ret->ph = ph_create(config);

	return ret;
}


void tcphandler_destroy(struct tcp_handler_t* t)
{
	ph_destroy(t->ph);
	pi_destroy(t->pi);
	free(t);
}

void tcphandler_run(struct tcp_handler_t* tcph)
{
	int targetServiceFd, maxfd;
	struct sockaddr_in targetServAddr;
	int connectedToFinalTarget = 0;
	int r;
	fd_set rset;
	struct timeval tv;
	char payload[MAXLINE];
	protocols_app app_proto;
	struct proto_handler_t* proto_handler;

	bzero(payload, MAXLINE);
	targetServiceFd = Socket(AF_INET, SOCK_STREAM, 0);

	// Determine target service address. This address depends on the chosen program mode
	switch (tcph->mode) {
	case full_emulation:
		// Final target depends on the protocol, protocol identification
		// can only be performed on the payload from the client side.
		// If payload identification on the client side is not possible, 
		// we can only determine the target based on port numbers. 
		// This mode can therefore not determine applications which contain
		// initial server payload and do not use standard ports
		bzero(tcph->connection->dest, IPLENGTH);
		break;
	case half_proxy:
		// Final target depends on the protocol. Protcol identification
		// can be performed on both the intial client as well as the 
		// initial server string
		bzero(tcph->connection->dest, IPLENGTH);
		break;
	case full_proxy:
		// Connect to the original target (if this target is available)
		bzero(&targetServAddr, sizeof(targetServAddr));
		targetServAddr.sin_family = AF_INET;
		targetServAddr.sin_port = htons((uint16_t)tcph->connection->dport);
		memcpy(tcph->connection->dest, tcph->connection->orig_dest, IPLENGTH);
		Inet_pton(AF_INET, tcph->connection->dest, &targetServAddr.sin_addr);
	default:
		msg(MSG_FATAL, "Unknown mode: This is an internal programming error!!!! Exiting!");
		exit(-1);
	}

	FD_ZERO(&rset);
	FD_SET(targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
	FD_SET(tcph->inconnfd, &rset);
	maxfd = max(targetServiceFd, tcph->inconnfd);

	// wait 3 seconds for initial client payload
	// try to receive server payload if there is no 
	// payload from the client.
	tv.tv_sec = 3;
	tv.tv_usec = 0; 

	while (-1 != select(maxfd, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(targetServiceFd, &rset)) {
			// we received data from the internet server
			r = read(targetServiceFd, payload, MAXLINE - 1);
			if (!r) {
				msg(MSG_DEBUG, "Target closed the connection...");
				goto out;
			}
			if (tcph->connection->app_proto == UNKNOWN) {
				app_proto = tcph->pi->bypayload(tcph->pi, tcph->connection, payload, r);
				if (app_proto == UNKNOWN) {
					// TODO: handle this one! can we manage this?
				}
			}
			proto_handler = tcph->ph[tcph->connection->app_proto];
			proto_handler->handle_payload_stc(proto_handler->handler, payload, r);
		} else if FD_ISSET(tcph->inconnfd, &rset) {
			// we received data from the infrected machine
			r = read(tcph->inconnfd, payload, MAXLINE - 1);
			if (!r) {
				msg(MSG_DEBUG, "Source closed the connection...");
				goto out;
			}
			if (tcph->connection->app_proto == UNKNOWN) {
				app_proto = tcph->pi->bypayload(tcph->pi, tcph->connection, payload, r);
				if (app_proto == UNKNOWN) {
					// TODO: handle this one! can we manage this?
				}
			}
			proto_handler = tcph->ph[tcph->connection->app_proto];
			proto_handler->handle_payload_cts(proto_handler->handler, payload, r);
		} else {
			// We received a timeout. There are know to possiblities:
			// 1.) We already identified the protocol: There is something wrong, as there should not be any timeout
			// 2.) We did not identify the payload: We need to perform some 
			//     actions to enable payload identification
			if (tcph->connection->app_proto != UNKNOWN) {
				goto out; // exit function
			}
			// try to save the day
			switch (tcph->mode) {
			case full_emulation:
				app_proto = tcph->pi->byport(tcph->pi, tcph->connection);
				// if portbased failed:
				if (app_proto == UNKNOWN) {
					msg(MSG_ERROR, "Cannot identify application protocol in full_emulation mode!");
					goto out;
				}
				tcph->ph[app_proto]->determine_target(tcph->ph[app_proto]->handler, &targetServAddr);
			case  half_proxy:
				// TODO: fetch banner
				// try to identify using fetched banner
				break;
			case full_proxy:
				// do nothing, we know the original target and will connect to it after this switch
				// target is already intitialized by now
				break;
			default:
				msg(MSG_FATAL, "Unknown mode: This is an internal programming error!!!! Exiting!");
				exit(-1);
			}
			// we have to know the target now!
			if (!connectedToFinalTarget) {
				if (-1 == Connect(targetServiceFd, (struct sockaddr*)&targetServAddr, sizeof(targetServAddr))) {
					Close_conn(tcph->inconnfd, "Connection to targetservice could not be established");
					return;
				}
			}
				
		}
		FD_ZERO(&rset);
		FD_SET(targetServiceFd, &rset); // as this socket is not connected to anyone, it should to be responsible for select to fail
		FD_SET(tcph->inconnfd, &rset);
		maxfd = max(targetServiceFd, tcph->inconnfd);
		tv.tv_sec = 300;
		tv.tv_usec = 0; 
	}
//	msg(MSG_DEBUG, "we start doing protocol identification by payload...");
//	
//	proto = tcph->pi->identify(tcph->pi, tcph->connection, tcph->inconnfd, payload, &r);
//
//	// redirect traffic if we are in emulation mode
//
//	tcph->ph[proto]->determine_target(tcph->ph[proto]->handler, &targetservaddr);
//		
//	if (Connect(targetservicefd, (SA *) &targetservaddr, sizeof(targetservaddr)) < 0) {
//		Close_conn(tcph->inconnfd, "connection to targetservice could not be established");
//		return;
//	} else
//		msg(MSG_DEBUG, "the connection to the targetservice is established and we can now start forwarding\n");
//	
//	// now we are definitely connected to the targetservice ...
//	switch(tcph->connection->app_proto) {
//		case FTP:
//			protocol_dir = FTP_COLLECTING_DIR;
//			break;
//		case FTP_anonym:
//			protocol_dir = FTP_COLLECTING_DIR;
//			break;
//		case FTP_data:
//			protocol_dir = FTP_COLLECTING_DIR;
//			break;
//		case SMTP:
//			protocol_dir = SMTP_COLLECTING_DIR;
//			break;
//		case HTTP:
//			protocol_dir = HTTP_COLLECTING_DIR;
//			break;
//		case IRC:
//			protocol_dir = IRC_COLLECTING_DIR;
//			break;
//		default:
//			msg(MSG_ERROR, "Could not set protocol_dir");
//			break;
//	}
//	
//	print_timestamp(tcph->connection, protocol_dir);
//	
//	msg(MSG_DEBUG, "payload is:\n%s", payload);
out:
	Close_conn(tcph->inconnfd, "incomming connection, because we are done with this connection");
	Close_conn(targetServiceFd, "connection to targetservice, because we are done with this connection");
}

