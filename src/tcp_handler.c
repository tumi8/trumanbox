#include "tcp_handler.h"
#include "definitions.h"
#include "helper_net.h"
#include "msg.h"

#include <stdlib.h>

struct tcp_handler_t {
	operation_mode_t mode;
	int sock;
	connection_t* connection;
	int inconnfd;
	struct proto_identifier_t* pi;
};

struct tcp_handler_t* tcphandler_create(operation_mode_t mode, connection_t* c, int inconn, struct proto_identifier_t* pi)
{
	struct tcp_handler_t* ret = (struct tcp_handler_t*)malloc(sizeof(struct tcp_handler_t*));
	ret->mode = mode;
	ret->connection = c;
	ret->inconnfd = inconn;
	ret->pi = pi;

	return ret;
}


void tcphandler_destroy(struct tcp_handler_t* t)
{
	free(t);
}

void tcphandler_run(struct tcp_handler_t* tcph)
{
	int			targetservicefd,
				maxfdp;
	struct sockaddr_in	targetservaddr;
	char			payload[MAXLINE],
				to_drop[MAXLINE],
				*ptr,
				*protocol_dir = NULL;
	ssize_t			r, w, d;
	fd_set 			rset;
	struct timeval 		tv;
	
	targetservicefd = Socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&targetservaddr, sizeof(targetservaddr));
	targetservaddr.sin_family = AF_INET;
	targetservaddr.sin_port = htons((uint16_t)tcph->connection->dport);
	Inet_pton(AF_INET, tcph->connection->dest, &targetservaddr.sin_addr);
	
	msg(MSG_DEBUG, "we start doing protocol identification by payload...");
	
	tcph->pi->identify(tcph->pi, tcph->connection, tcph->inconnfd, payload);
	// now we know the protocol
	
	// redirect traffic if we are in emulation mode
	if (tcph->mode < full_proxy) {
		bzero(&targetservaddr, sizeof(targetservaddr));
		targetservaddr.sin_family = AF_INET;
			
		// TODO: fix this
		Inet_pton(AF_INET, "127.0.0.1", &targetservaddr.sin_addr);
		switch(tcph->connection->app_proto) {
			case FTP:
				targetservaddr.sin_port = htons((uint16_t)21);
				break;
			case FTP_anonym:
				targetservaddr.sin_port = htons((uint16_t)21);
				break;
			case FTP_data:
				msg(MSG_DEBUG, "so we set port to: %d", tcph->connection->dport);
				targetservaddr.sin_port = htons((uint16_t)tcph->connection->dport);
				break;
			case SMTP:
				targetservaddr.sin_port = htons((uint16_t)25);
				break;
			case HTTP:
				targetservaddr.sin_port = htons((uint16_t)80);
				break;
			case IRC:
				targetservaddr.sin_port = htons((uint16_t)6667);
				break;
				default:
					return;
		}
	}
	
	if (Connect(targetservicefd, (SA *) &targetservaddr, sizeof(targetservaddr)) < 0) {
		Close_conn(tcph->inconnfd, "connection to targetservice could not be established");
		return;
	} else
		msg(MSG_DEBUG, "the connection to the targetservice is established and we can now start forwarding\n");
	
	// now we are definitely connected to the targetservice ...
	switch(tcph->connection->app_proto) {
		case FTP:
			protocol_dir = FTP_COLLECTING_DIR;
			break;
		case FTP_anonym:
			protocol_dir = FTP_COLLECTING_DIR;
			break;
		case FTP_data:
			protocol_dir = FTP_COLLECTING_DIR;
			break;
		case SMTP:
			protocol_dir = SMTP_COLLECTING_DIR;
			break;
		case HTTP:
			protocol_dir = HTTP_COLLECTING_DIR;
			break;
		case IRC:
			protocol_dir = IRC_COLLECTING_DIR;
			break;
		default:
			msg(MSG_ERROR, "Could not set protocol_dir");
			break;
	}
	
	print_timestamp(tcph->connection, protocol_dir);
	
	msg(MSG_DEBUG, "payload is:\n%s", payload);
	
	r = strlen(payload);
	
	if (r) {
		ptr = payload;
		if (tcph->connection->app_proto < FTP_data) {
			d = read(targetservicefd, to_drop, MAXLINE-1);
			msg(MSG_DEBUG, "the following %d characters are dropped:\n%s", d, to_drop);
			
			content_substitution_and_logging_stc(tcph->connection, payload, &r);
			
			while(r > 0 && (w = write(tcph->inconnfd, ptr, r)) > 0) {
				ptr += w;
				r -= w;
			}
		} else {
			if (tcph->mode < full_proxy) {
				content_substitution_and_logging_cts(tcph->connection, payload, &r);
				build_tree(tcph->connection, payload);
			}
	
			while(r > 0 && (w = write(targetservicefd, ptr, r)) > 0) {
				ptr += w;
				r -= w;
			}
			msg(MSG_DEBUG, "and has been sent to server...\n");
		}
	}
	
	memset(payload, 0, sizeof(payload));
	
	FD_ZERO(&rset);
	FD_SET(tcph->inconnfd, &rset);
	FD_SET(targetservicefd, &rset);
			
	tv.tv_sec = 300;
	tv.tv_usec = 0;
	
	maxfdp = max(tcph->inconnfd, targetservicefd) + 1;
	
	while (select(maxfdp, &rset, NULL, NULL, &tv)) {
		if (FD_ISSET(tcph->inconnfd, &rset)) {
			// forwarding from the client to the server
			msg(MSG_DEBUG, "inconnfd is ready\n");
			if ((r = read(tcph->inconnfd, payload, MAXLINE-1)) == 0) {
				msg(MSG_DEBUG, "client has closed the connection");
				Close_conn(targetservicefd, "connection to targetservice, because the client has closed the connection");
				Close_conn(tcph->inconnfd, "incomming connection, because the client has closed the connection");
				return;
			} else if (r > 0) {
				msg(MSG_DEBUG, "(pid: %d) payload from client:\n%s", getpid(), payload);  // for debugging
				if (tcph->mode < full_proxy) {
					content_substitution_and_logging_cts(tcph->connection, payload, &r);
					build_tree(tcph->connection, payload);
				}
				if (tcph->mode == full_proxy) // FIXME is this really stable???
					delete_row_starting_with_pattern(payload, "Accept-Encoding:");
					
				msg(MSG_DEBUG, "(pid: %d) changed payload from client:\n%s", getpid(), payload);  // for debugging
	
				ptr = payload;
				while (r > 0 && (w = write(targetservicefd, ptr, r)) > 0) {
					ptr += w;
					r -= w;
				}

				msg(MSG_DEBUG, "...has been sent to server");
			} else {
				msg(MSG_DEBUG, "read error: reading from inconnfd");
				return;
			}
		}
		memset(payload, 0, sizeof(payload));
		
		if (FD_ISSET(targetservicefd, &rset)) { 
			// forwarding from the server to the client
			msg(MSG_DEBUG, "targetservicefd is ready");
			if ((r = read(targetservicefd, payload, MAXLINE-1)) == 0) {
				msg(MSG_DEBUG, "server has closed the connection\n");
				Close_conn(tcph->inconnfd, "incoming connection, because the server has closed the connection");
				Close_conn(targetservicefd, "connection to targetservice, because the server has closed the connection");
				return;
			} else if (r > 0) {
				msg(MSG_DEBUG, "(pid: %d) payload from server:\n%s", (int)getpid(), payload);
				
				content_substitution_and_logging_stc(tcph->connection, payload, &r);
				
				msg(MSG_DEBUG, "(pid: %d) changed payload from server:\n%s", getpid(), payload);
				
				ptr = payload;
				while (r > 0 && (w = write(tcph->inconnfd, ptr, r)) > 0) {
					ptr += w;
					r -= w;
				}
				msg(MSG_DEBUG, "has been sent to client...");
				
				memset(payload, 0, sizeof(payload));
			} else {
				msg(MSG_ERROR, "(pid: %d)read error: reading from targetservicefd", getpid());
			}
		}
		FD_ZERO(&rset);
		FD_SET(tcph->inconnfd, &rset);
		FD_SET(targetservicefd, &rset);
	}
	Close_conn(tcph->inconnfd, "incomming connection, because we are done with this connection");
	Close_conn(targetservicefd, "connection to targetservice, because we are done with this connection");
}

