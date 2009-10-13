#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "definitions.h"
#include "signals.h"
#include "configuration.h"

#include "dispatching.h"
#include "wrapper.h"
#include "helper_file.h"
#include "helper_net.h"
#include "payload_ident.h"
#include "payload_alter_log.h"
#include "msg.h"
#include "udp_handler.h"
#include "tcp_handler.h"

struct dispatcher_t {
	const char* dump_dir;
	int tcpfd;
	int udpfd;
	operation_mode_t mode;
	int running;
};

struct dispatcher_t* disp_create(struct configuration_t* c, operation_mode_t mode)
{
	struct dispatcher_t* ret = (struct dispatcher_t*)malloc(sizeof(struct dispatcher_t));
	ret->dump_dir = conf_get(c, "main", "dump_dir");	
	ret->mode = mode;
	

	int val=1; // will enable SO_REUSEADDR

	struct sockaddr_in saddr;

	// create tcp socket...
	ret->tcpfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&saddr, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port        = htons(TB_LISTEN_PORT);

	setsockopt(ret->tcpfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	Bind(ret->tcpfd, (SA *) &saddr, sizeof(saddr));
	Listen(ret->tcpfd, LISTENQ);	

	// create udp socket...
	ret->udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&saddr, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port        = htons(TB_LISTEN_PORT);

	Bind(ret->udpfd, (SA *) &saddr, sizeof(saddr));

	return ret;
}

int disp_destroy(struct dispatcher_t* d)
{
	free(d);
	return 0;
}

/*
wait for incomming connection and return the protocol (tcp, udp, unknown)
*/
protocols_net wait_for_incomming_connection(int tcpfd, int udpfd) {
	fd_set	read_set;
	int	maxfdp1, notready;

	FD_ZERO(&read_set);
	maxfdp1 = max(tcpfd, udpfd) + 1;

	FD_SET(tcpfd, &read_set);
	FD_SET(udpfd, &read_set);

	// FIXME: is it possible to retrieve tcp and udp connection simultanously???
	if ( (notready = select(maxfdp1, &read_set, NULL, NULL, NULL)) < 0) {
		if (errno == EINTR)
			return ERROR;
		else {
			msg(MSG_ERROR, "select failed: %s", strerror(errno));
			return ERROR;
		}
	}

	if (FD_ISSET(tcpfd, &read_set))
		return TCP;		
	else if (FD_ISSET(udpfd, &read_set))
		return UDP;
	else
		return UNKNOWN;
}

//void dispatching(int mode) {
void disp_run(struct dispatcher_t* disp)
{
	int			inconnfd,
				targetservicefd,
				maxfdp,
				tries_pars_ct;
	pid_t			childpid;
	struct sockaddr_in	targetservaddr,
				cliaddr;
	socklen_t		clilen;
	char			payload[MAXLINE],
				to_drop[MAXLINE],
				*ptr,
				*protocol_dir;
	ssize_t			r, w, d;
	fd_set 			rset;
	struct timeval 		tv;
	connection_t 		connection;


	Signal(SIGCHLD, sig_chld);

	for ( ; ; ) {
	start:
		connection.net_proto = wait_for_incomming_connection(disp->tcpfd, disp->udpfd);

		if (connection.net_proto == ERROR)
			continue;

		if (connection.net_proto == TCP) {
			clilen = sizeof(cliaddr);
			inconnfd = Accept(disp->tcpfd, (SA *) &cliaddr, &clilen);
			
			Inet_ntop(AF_INET, &cliaddr.sin_addr, connection.source, 15);
			connection.sport = ntohs(cliaddr.sin_port);
			tries_pars_ct = 0;

			while ( parse_conntrack(&connection) != 0 ) {
				msg(MSG_DEBUG, "could not parse conntrack table, trying again in 2sec...");
				sleep(2);
				tries_pars_ct++;
				if (tries_pars_ct > 5) {
					Close_conn(inconnfd, "incomming connection, because conntrack table could not be parsed\n");
					goto start;
				}
			}
				
			// on connect create a child that handles the connection
			if ( (childpid = Fork()) == 0) {	/* child process */
				Close(disp->tcpfd);	/* close listening socket within child process */
	
				targetservicefd = Socket(AF_INET, SOCK_STREAM, 0);
				
				bzero(&targetservaddr, sizeof(targetservaddr));
				targetservaddr.sin_family = AF_INET;
				targetservaddr.sin_port = htons((uint16_t)connection.dport);
				Inet_pton(AF_INET, connection.dest, &targetservaddr.sin_addr);
	
				msg(MSG_DEBUG, "we start doing protocol identification by payload...");

				protocol_identified_by_payload(disp->mode, &connection, inconnfd, payload);
	
				if (connection.app_proto == UNKNOWN) {
					msg(MSG_DEBUG, "...failed!\nso we try doing (weak) protocol identification by port...");
					protocol_identified_by_port(disp->mode, &connection, payload);
				}
	
				if (connection.app_proto == UNKNOWN) {
					msg(MSG_ERROR, "failed!\nthe protocol could not be identified, so we stop handling this connection.\n "
							"the dumped payload can be found in %s/%s:%d", DUMP_FOLDER, connection.dest, connection.dport);
					append_to_file(payload, &connection, DUMP_FOLDER);
					Close_conn(inconnfd, "incomming connection, because of unknown protocol");
					Exit(1);
				}
	
				// now we know the protocol
	
				if (disp->mode < 3) {
	
					bzero(&targetservaddr, sizeof(targetservaddr));
					targetservaddr.sin_family = AF_INET;
	
					Inet_pton(AF_INET, "127.0.0.1", &targetservaddr.sin_addr);
					switch(connection.app_proto) {
						case FTP:
							targetservaddr.sin_port = htons((uint16_t)21);
							break;
						case FTP_anonym:
							targetservaddr.sin_port = htons((uint16_t)21);
							break;
						case FTP_data:
							msg(MSG_DEBUG, "so we set port to: %d", connection.dport);
							targetservaddr.sin_port = htons((uint16_t)connection.dport);
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
							Exit(1);
					}
				}
	
				if (Connect(targetservicefd, (SA *) &targetservaddr, sizeof(targetservaddr)) < 0) {
					Close_conn(inconnfd, "connection to targetservice could not be established");
					Exit(1);
				}
				else
					msg(MSG_DEBUG, "the connection to the targetservice is established and we can now start forwarding\n");
	
				// now we are definitely connected to the targetservice ...
	
				switch(connection.app_proto) {
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
						msg(MSG_ERROR, "didnt set protocol_dir");
						break;
				}
	
				print_timestamp(&connection, protocol_dir);
	
				msg(MSG_DEBUG, "payload is:\n%s", payload);
	
				r = strlen(payload);
	
				if (r) {
					ptr = payload;
					if (connection.app_proto < FTP_data) {
						d = read(targetservicefd, to_drop, MAXLINE-1);
						msg(MSG_DEBUG, "the following %d characters are dropped:\n%s", d, to_drop);
	
						content_substitution_and_logging_stc(&connection, payload, &r);
	
						while(r > 0 && (w = write(inconnfd, ptr, r)) > 0) {
							ptr += w;
							r -= w;
						}
					}
					else {
						if (disp->mode < 3) {
							content_substitution_and_logging_cts(&connection, payload, &r);
							build_tree(&connection, payload);
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
				FD_SET(inconnfd, &rset);
				FD_SET(targetservicefd, &rset);
			
				tv.tv_sec = 300;
				tv.tv_usec = 0;
	
				maxfdp = max(inconnfd, targetservicefd) + 1;
	
				while (select(maxfdp, &rset, NULL, NULL, &tv)) {
					if (FD_ISSET(inconnfd, &rset)) {
						// forwarding from the client to the server
						msg(MSG_DEBUG, "inconnfd is ready\n");
						if ((r = read(inconnfd, payload, MAXLINE-1)) == 0) {
							msg(MSG_DEBUG, "client has closed the connection");
							Close_conn(targetservicefd, "connection to targetservice, because the client has closed the connection");
							Close_conn(inconnfd, "incomming connection, because the client has closed the connection");
							Exit(0);
						} 
						else if (r > 0) {
				
							msg(MSG_DEBUG, "(pid: %d) payload from client:\n%s", getpid(), payload);  // for debugging
							if (disp->mode < 3) {
								content_substitution_and_logging_cts(&connection, payload, &r);
								build_tree(&connection, payload);
							}
							if (disp->mode == 3) // FIXME is this really stable???
								delete_row_starting_with_pattern(payload, "Accept-Encoding:");

							msg(MSG_DEBUG, "(pid: %d) changed payload from client:\n%s", getpid(), payload);  // for debugging
	
							ptr = payload;
							while (r > 0 && (w = write(targetservicefd, ptr, r)) > 0) {
								ptr += w;
								r -= w;
							}

							msg(MSG_DEBUG, "...has been sent to server");
						}
						else {
							msg(MSG_DEBUG, "read error: reading from inconnfd");
							Exit(1);
						}
					}
					memset(payload, 0, sizeof(payload));
	
					if (FD_ISSET(targetservicefd, &rset)) { 
						// forwarding from the server to the client
						msg(MSG_DEBUG, "targetservicefd is ready");
						if ((r = read(targetservicefd, payload, MAXLINE-1)) == 0) {
							msg(MSG_DEBUG, "server has closed the connection\n");
							Close_conn(inconnfd, "incoming connection, because the server has closed the connection");
							Close_conn(targetservicefd, "connection to targetservice, because the server has closed the connection");
							Exit(0);
						}
						else if (r > 0) {
							msg(MSG_DEBUG, "(pid: %d) payload from server:\n%s", (int)getpid(), payload);
	
							content_substitution_and_logging_stc(&connection, payload, &r);
	
							msg(MSG_DEBUG, "(pid: %d) changed payload from server:\n%s", getpid(), payload);
	
							ptr = payload;
							while (r > 0 && (w = write(inconnfd, ptr, r)) > 0) {
								ptr += w;
								r -= w;
							}
							msg(MSG_DEBUG, "has been sent to client...");
	
							memset(payload, 0, sizeof(payload));
						}
						else {
							msg(MSG_ERROR, "(pid: %d)read error: reading from targetservicefd", getpid());
						}
					}
					FD_ZERO(&rset);
					FD_SET(inconnfd, &rset);
					FD_SET(targetservicefd, &rset);
				}
				Close_conn(inconnfd, "incomming connection, because we are done with this connection");
				Close_conn(targetservicefd, "connection to targetservice, because we are done with this connection");
	
				Exit(0);
			}
		}
		else if (connection.net_proto == UDP) {
			if ( (childpid = Fork()) == 0) {	/* child process */
				struct udp_handler_t* u = udphandler_create(disp->udpfd);
				udphandler_run(u);
				udphandler_destroy(u);
			}
		}
		else {
			msg(MSG_DEBUG, "we got some network protocol which is neither tcp nor udp");
		}
		memset(&connection, 0, sizeof(connection));
	}
}


