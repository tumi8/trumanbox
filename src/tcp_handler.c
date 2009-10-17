#include "tcp_handler.h"
#include "definitions.h"
#include "helper_net.h"

#include <stdlib.h>

struct tcp_handler_t {
	int sock;
};

struct tcp_handler_t* tcphandler_create()
{
	struct tcp_handler_t* ret = (struct tcp_handler_t*)malloc(sizeof(struct tcp_handler_t*));

	return ret;
}


void tcphandler_destroy(struct tcp_handler_t* t)
{
	free(t);
}

void tcphandler_run(connection_t* connection, int innconnfd)
{
	int			inconnfd,
				targetservicefd,
				maxfdp,
	struct sockaddr_in	targetservaddr,
	char			payload[MAXLINE],
				to_drop[MAXLINE],
				*ptr,
				*protocol_dir;
	ssize_t			r, w, d;
	fd_set 			rset;
	struct timeval 		tv;
	
	
				
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

