#include "dispatching.h"
#include "signals.h"
#include "helper_file.h"
#include "helper_net.h"
#include "udp_handler.h"
#include "tcp_handler.h"
#include "process_manager.h"
#include "wrapper.h"
#include "logger.h"

#include "protocols/proto_ident.h"
#include "protocols/proto_handler.h"

#include <common/configuration.h>
#include <common/msg.h>


#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

enum e_command { unknown_command, start_analysis, stop_analysis, restart_analysis };

static enum e_command read_command(const Configuration& config, int fd, char** filename);

Dispatcher::Dispatcher(const Configuration& config)
	: config(config)
{
	this->protoIdent = new ProtoIdent();// o= pi_create(c, conf_getint(c, "main", "protocol_identifier", 0));

	int val=1; // will enable SO_REUSEADDR

	struct sockaddr_in saddr;

	pm_init();

	// create tcp socket...
	this->tcpfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&saddr, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port        = htons(TB_LISTEN_PORT);

	setsockopt(this->tcpfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	Bind(this->tcpfd, (SA *) &saddr, sizeof(saddr));
	Listen(this->tcpfd, LISTENQ);	

	// create udp socket...
	this->udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&saddr, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port        = htons(TB_LISTEN_PORT);

	Bind(this->udpfd, (SA *) &saddr, sizeof(saddr));

	// create signaling socket
	this->controlfd = Socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(TB_CONTROL_PORT);

	Bind(this->controlfd, (SA*)&saddr, sizeof(saddr));
}

Dispatcher::~Dispatcher()
{
	pm_destroy();
	ph_destroy(this->ph);
}

/*
wait for incoming connection and return the protocol (tcp, udp, unknown)
*/
protocols_net wait_for_incoming_connection(int tcpfd, int udpfd, int controlfd) {
	fd_set	read_set;
	int	maxfdp1, notready;

	FD_ZERO(&read_set);
	maxfdp1 = std::max(controlfd, std::max(tcpfd, udpfd)) + 1;

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
		return ERROR;
}

void Dispatcher::run()
{
	connection_t connection;
	int tries_pars_ct;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	int inconnfd;
	pid_t childpid;

	Signal(SIGCHLD, sig_chld);
	tries_pars_ct = 0;

	logger_get()->create_log(logger_get());
	for ( ; ; ) {
	start:
		connection.net_proto = wait_for_incoming_connection(this->tcpfd, this->udpfd, this->controlfd);
		connection.app_proto = UNKNOWN;
		connection.log_server_struct_ptr = NULL;
		connection.log_client_struct_ptr = NULL;
		connection.log_server_struct_initialized = 0;
		connection.log_client_struct_initialized = 0;
		connection.multiple_client_chunks = 0;
		connection.multiple_server_chunks = 0;
		connection.destOffline = 0;
		connection.countReads = 0;
		// generate timestamp for the connection
		create_timestamp(connection.timestamp);


		if (connection.net_proto == ERROR)
			continue;

		if (connection.net_proto == TCP) {
			clilen = sizeof(cliaddr);
			inconnfd = Accept(this->tcpfd, (SA *) &cliaddr, &clilen);
			
			Inet_ntop(AF_INET, &cliaddr.sin_addr, connection.source, sizeof(connection.source));
			connection.sport = ntohs(cliaddr.sin_port);
			// parse_conntrack fills in the remaining variables of connection
			while ( parse_conntrack(&connection) != 0 ) {
				msg(MSG_DEBUG, "could not parse conntrack table, trying again in 2sec...");
				sleep(2);
				tries_pars_ct++;
				if (tries_pars_ct > 5) {
					Close_conn(inconnfd, "incoming connection, because conntrack table could not be parsed\n");
					goto start;
				}
			}
			if ( (childpid = pm_fork_temporary()) == 0) {        /* child process */
				msg(MSG_DEBUG, "Forked TCP handler with pid %d", getpid());
				Close(this->tcpfd);     /* close listening socket within child process */
				TcpHandler t(inconnfd, this->config, &connection, this->protoIdent, this->ph);
				t.run();
				Exit(0);
			}

		}
		else if (connection.net_proto == UDP) {

		if ( (childpid = pm_fork_temporary()) == 0) {
				connection.app_proto = UNKNOWN_UDP;
				msg(MSG_DEBUG, "Forked UDP handler with pid %d", getpid());
				UdpHandler u(this->udpfd, this->config, &connection, this->protoIdent, this->ph);
				u.run();
				Exit(0);
		}
		}
		else if (connection.net_proto == CONTROL) {
			char malwaresample_filename[MAX_PATH_LENGTH];
			bzero(malwaresample_filename,MAX_PATH_LENGTH);
			char* ptrToSamplefilename = malwaresample_filename;
			enum e_command res = read_command(this->config, this->controlfd,&ptrToSamplefilename);
			if (res == restart_analysis) {
				msg(MSG_DEBUG, "Got restart analysis command. Killing Processes.");
				pm_kill_temporary();
				msg(MSG_DEBUG, "Finalizing logfiles!");
				logger_get()->finish_log(logger_get());
				msg(MSG_DEBUG, "Creating new log!");
				logger_get()->create_log(logger_get());
				msg(MSG_DEBUG, "Restarted logging process!");
			}
			else if (res == start_analysis) {
				msg(MSG_DEBUG,"Got start analysis command for sample :'%s'",malwaresample_filename);
				char update_trumanbox_runtime_id[1000] = "update trumanbox_settings set value = value+1 where key = 'CURRENT_SAMPLE'";
				execute_statement(update_trumanbox_runtime_id);
				char new_malware_dataset[1000];
				snprintf(new_malware_dataset,1000,"insert into malwaresamples (id,beginlogging,comments) values ((select t.value from trumanbox_settings t where t.key = 'CURRENT_SAMPLE'), (select current_timestamp), '%s' )",malwaresample_filename);
				execute_statement(new_malware_dataset);
				
//				logger_get()->create_log(logger_get());
				msg(MSG_DEBUG,"Started logging process");
			}
			else if (res == stop_analysis) {
				msg(MSG_DEBUG,"Got stop analysis command");
				pm_kill_temporary();
				msg(MSG_DEBUG, "Finalizing logfiles!");
				logger_get()->finish_log(logger_get());
			}
		} else {
			msg(MSG_DEBUG, "we got some network protocol which is neither tcp nor udp");
		}
		memset(&connection, 0, sizeof(connection));
	}
}

static enum e_command read_command(const Configuration& config, int fd, char** filename)
{
	// TODO: extend dummy interface
	char payload[MAXLINE];
	ssize_t r;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	std::string remote_pw_string = config.get("main", "remotepw");
	std::string restart_string = config.get("main", "logger_restart_string");
	std::string start_string = config.get("main", "logger_start_string");
	std::string stop_string = config.get("main", "logger_stop_string");
	char* ptrToFilename = NULL;
	r = Recvfrom(fd, payload, MAXLINE, 0, (SA *)  &cliaddr, &clilen);
	// TODO: remove -1. i need this because i'm testing with netcat which adds an additional \n
	int pwlen = remote_pw_string.length();
	if (!strncmp(payload, remote_pw_string.c_str(), pwlen)) {
		msg(MSG_DEBUG,"Password successfully received!");
		if (strstr(payload,restart_string.c_str()) != 0) {
			return restart_analysis;
		}
		else if ((ptrToFilename = strstr(payload, start_string.c_str())) != 0) {
			// we now extract the filename of the malware sample, given as third argument
			ptrToFilename = strstr(ptrToFilename," ");
			ptrToFilename ++;
			char keys[] = "\n\r \0";
			int len = strcspn(ptrToFilename,keys);
			memcpy(*filename,ptrToFilename,len);
			return start_analysis;
		}
		else if (strstr(payload,stop_string.c_str()) != 0)  {
			return stop_analysis;
		}
		msg(MSG_DEBUG,"unknown command entered");

	}
	
	return unknown_command;
}

int parse_conntrack(connection_t *conn) {
	FILE *fd;
	char line[MAX_LINE_LENGTH];
	char proto[5];
	char *begin, *end, portnum[10];
	char tmp[100];
	char *r;

	memset(line, 0, MAX_LINE_LENGTH);

	if ((fd = fopen("/proc/net/ip_conntrack", "r")) == NULL) {
		msg(MSG_ERROR, "Can't open ip_conntrack for reading: %s\n", strerror(errno));
		return -1;
	}

	if (conn->net_proto == TCP)
		strcpy(proto, "tcp");
	else if (conn->net_proto == UDP)
		strcpy(proto, "udp");
	else if (conn->net_proto == ICMP)
		strcpy(proto, "icmp");
	else
		proto[0] = 0;

	while ( (NULL != (r = fgets(line, MAX_LINE_LENGTH, fd))) ) {
		//sleep(2);

//		msg(MSG_DEBUG, "We got:\n%s", line);

		if (strncmp(line, proto, 3) == 0) {

			begin = strstr(line, "src=") + 4;
			end = strchr(begin, ' ');
			snprintf(tmp, (end-begin+1), "%s", begin);

			if (strncmp(conn->source, begin, (end - begin)) == 0) {
				begin = strstr(end, "sport=") + 6;
				end = strchr(begin, ' ');
			
				snprintf(portnum, end-begin+1, "%s", begin);
				if (conn->sport == atoi(portnum)) {
					// We have found right conntrack entry
					begin = strstr(line, "dst=") + 4;
					end = strchr(begin, ' ');

					strncpy(conn->orig_dest, begin, end-begin);
					conn->orig_dest[end-begin] = '\0';
					begin = strstr(end, "dport=") + 6;
					end = strchr(begin, ' ');

					snprintf(portnum, end-begin+1, "%s", begin);

					strcpy(conn->dest, conn->orig_dest);
					conn->dport = atoi(portnum);
					conn->orig_dport = conn->dport;
					fclose(fd);
					return 0;
				}
				else {
					//msg(MSG_ERROR, "Source Port does not match\n");
					continue;
				}
			}
			else {
				//msg(MSG_DEBUG, "The source IP: %s\ndoes not match with the corresponding conntrack entry: %s\n", conn->source, tmp);
				continue;
			}
		}
		else {
			//msg(MSG_ERROR, "Protocol does not match\n");
			continue;
		}
	}

	fclose(fd);
	return -1;
}

