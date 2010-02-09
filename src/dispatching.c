#include "dispatching.h"
#include "signals.h"
#include "configuration.h"
#include "helper_file.h"
#include "helper_net.h"
#include "payload_alter_log.h"
#include "msg.h"
#include "udp_handler.h"
#include "tcp_handler.h"
#include "process_manager.h"
#include "proto_ident.h"
#include "proto_handler.h"
#include "wrapper.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

struct dispatcher_t {
	int controlfd;
	int tcpfd;
	int udpfd;
	struct proto_identifier_t* pi;
	struct proto_handler_t** ph;
	int running;
	struct configuration_t* config;
};

enum e_command { restart_analysis };

enum e_command read_command(int fd);

struct dispatcher_t* disp_create(struct configuration_t* c)
{
	struct dispatcher_t* ret = (struct dispatcher_t*)malloc(sizeof(struct dispatcher_t));

	ret->pi = pi_create(c, conf_getint(c, "main", "protocol_identifier", 0));
	ret->pi->init(ret->pi);
	ret->ph = ph_create(c);
	ret->config = c;

	int val=1; // will enable SO_REUSEADDR

	struct sockaddr_in saddr;

	pm_init();

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

	// create signaling socket
	ret->controlfd = Socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(TB_CONTROL_PORT);

	Bind(ret->controlfd, (SA*)&saddr, sizeof(saddr));

	return ret;
}

int disp_destroy(struct dispatcher_t* d)
{
	pm_destroy();
	d->pi->deinit(d->pi);
	free(d->pi);
	d->pi = NULL;
	ph_destroy(d->ph);
	d->ph = NULL;
	free(d);
	return 0;
}

/*
wait for incomming connection and return the protocol (tcp, udp, unknown)
*/
protocols_net wait_for_incomming_connection(int tcpfd, int udpfd, int controlfd) {
	fd_set	read_set;
	int	maxfdp1, notready;

	FD_ZERO(&read_set);
	maxfdp1 = max(tcpfd, udpfd) + 1;

	FD_SET(tcpfd, &read_set);
	FD_SET(udpfd, &read_set);
	FD_SET(controlfd, &read_set);

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
	else if (FD_ISSET(controlfd, &read_set))
		return CONTROL;
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
		connection.net_proto = wait_for_incomming_connection(disp->tcpfd, disp->udpfd, disp->controlfd);
		connection.app_proto = UNKNOWN;

		if (connection.net_proto == ERROR)
			continue;

		if (connection.net_proto == TCP) {
			clilen = sizeof(cliaddr);
			inconnfd = Accept(disp->tcpfd, (SA *) &cliaddr, &clilen);
			
			Inet_ntop(AF_INET, &cliaddr.sin_addr, connection.source, 15);
			connection.sport = ntohs(cliaddr.sin_port);
			// parse_conntrack fills in the remaining variables of connection
			while ( parse_conntrack(&connection) != 0 ) {
				msg(MSG_DEBUG, "could not parse conntrack table, trying again in 2sec...");
				sleep(2);
				tries_pars_ct++;
				if (tries_pars_ct > 5) {
					Close_conn(inconnfd, "incomming connection, because conntrack table could not be parsed\n");
					goto start;
				}
			}
			if ( (childpid = pm_fork_temporary()) == 0) {        /* child process */
				Close(disp->tcpfd);     /* close listening socket within child process */
				struct tcp_handler_t* t = tcphandler_create(disp->config, &connection, inconnfd, disp->pi, disp->ph);
				tcphandler_run(t);
				tcphandler_destroy(t);
				Exit(0);
			}

		}
		else if (connection.net_proto == UDP) {
			if ( (childpid = pm_fork_temporary()) == 0) {	/* child process */
				struct udp_handler_t* u = udphandler_create(disp->udpfd);
				udphandler_run(u);
				udphandler_destroy(u);
				Exit(0);
			}
		}
		else if (connection.net_proto == CONTROL) {
			enum e_command res = read_command(disp->controlfd);
			if (res == restart_analysis) {
				pm_kill_temporary();
			}
		} else {
			msg(MSG_DEBUG, "we got some network protocol which is neither tcp nor udp");
		}
		memset(&connection, 0, sizeof(connection));
	}
}

enum e_command read_command(int fd)
{
	// TODO: extend dummy interface
	char payload[MAXLINE];
	ssize_t r;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	r = Recvfrom(fd, payload, MAXLINE, 0, (SA *)  &cliaddr, &clilen);
	Sendto(fd, payload, r, 0, (SA *) &cliaddr, clilen);
	return restart_analysis;
}

