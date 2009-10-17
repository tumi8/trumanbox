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

void disp_run(struct dispatcher_t* disp)
{
	connection_t connection;
	int tries_pars_ct;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	int inconnfd;
	pid_t childpid;

	Signal(SIGCHLD, sig_chld);
	tries_pars_ct = 0;

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
			while ( parse_conntrack(&connection) != 0 ) {
				msg(MSG_DEBUG, "could not parse conntrack table, trying again in 2sec...");
				sleep(2);
				tries_pars_ct++;
				if (tries_pars_ct > 5) {
					Close_conn(inconnfd, "incomming connection, because conntrack table could not be parsed\n");
					goto start;
				}
			}
			if ( (childpid = Fork()) == 0) {        /* child process */
				Close(disp->tcpfd);     /* close listening socket within child process */
				struct tcp_handler_t* t = tcphandler_create(disp->mode, &connection, inconnfd);
				tcphandler_run(t);
				tcphandler_destroy(t);
				Exit(0);
			}

		}
		else if (connection.net_proto == UDP) {
			if ( (childpid = Fork()) == 0) {	/* child process */
				struct udp_handler_t* u = udphandler_create(disp->udpfd);
				udphandler_run(u);
				udphandler_destroy(u);
				Exit(0);
			}
		}
		else {
			msg(MSG_DEBUG, "we got some network protocol which is neither tcp nor udp");
		}
		memset(&connection, 0, sizeof(connection));
	}
}


