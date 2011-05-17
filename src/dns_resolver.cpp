#include "dns_resolver.h"
#include "helper_net.h"
#include "signals.h"
#include "process_manager.h"
#include "wrapper.h"
#include "logger.h"
#include "helper_file.h"

#include <common/msg.h>
#include <common/configuration.h>
#include <common/definitions.h>


#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// XXX: check with RFC
#define MAX_REQUEST_LEN 1000
#define MAX_DOMAIN_LEN 1000

static void dns_worker(struct dns_resolver_t* resolver);
static void sig_int(int signo);

DNSResolver::DNSResolver(const Configuration& config)
{
	
	this->listen_ip = config.get("dns", "listen_address");
	this->fake_addr = config.get("dns", "fake_address");
	this->port = config.getInt("dns", "port", 53);
	this->return_orig = config.getInt("dns", "return_original", 1);
	this->pid = 0;
	inet_pton(AF_INET, config.get("dns", "fake_address").c_str(), &this->response_addr);

	strncpy(this->conn.dest, this->listen_ip.c_str(), IPLENGTH);
	create_timestamp(this->conn.timestamp); 
	this->conn.dport = 53;
	this->conn.app_proto = DNS;
}

void DNSResolver::start()
{
	if (0 == (this->pid = pm_fork_permanent())) {
		Signal(SIGINT, sig_int);
		dns_worker(this);
		THROWEXCEPTION("DNS-Worker has finished! This should never happen!");
	} 
}

void DNSResolver::stop()
{
	msg(MSG_DEBUG, "Stopping dns resolver");
	if (-1 == kill(this->pid, SIGINT))
		msg(MSG_FATAL, "Could not send signal to dns_resolver: %s", strerror(errno));
}


void DNSResolver::dns_worker(DNSResolver* resolver)
{
	int socket;
	struct sockaddr_in saddr, cliaddr;
	fd_set 			rset;
	int maxfd;
	int	r;
	socklen_t clilen;
	char		request[MAX_REQUEST_LEN];
	char		response[MAX_REQUEST_LEN];
	struct timeval 		tv;
	char opcode;
	int len, i, tmp;
	char domainname[MAX_DOMAIN_LEN];
	struct hostent* hent;
	uint32_t real_addr;
	char real_addr_str[INET_ADDRSTRLEN];
	char returned_addr_str[INET_ADDRSTRLEN];


	socket = Socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port        = htons(resolver->port);

	Bind(socket, (SA *) &saddr, sizeof(saddr));

	FD_ZERO(&rset);
	FD_SET(socket, &rset);
	tv.tv_sec = 1; 
	tv.tv_usec = 0;
	
	maxfd = socket + 1; 
	clilen = sizeof(cliaddr);
	
	while (-1 != select(maxfd, &rset, NULL, NULL, &tv)) {
		if (!FD_ISSET(socket, &rset)) {
			FD_ZERO(&rset);
			FD_SET(socket, &rset);
			tv.tv_sec = 1; 
			tv.tv_usec = 0;
			continue;
		}
		r = Recvfrom(socket, request, MAX_REQUEST_LEN, 0, (SA *)&cliaddr, &clilen);
		if (r <= 17) {
			// definately malformed DNS request
			msg(MSG_ERROR, "dns_resolver: received malformed request");
		}
	

		// only handle A requests
		opcode = (request[2] >> 3) & 15;
		if (opcode != 0) { 
			msg(MSG_ERROR, "Unknown DNS request %d", opcode);
			continue;
		} 
		// get domainname from request (only the first...)
		i = 12; // start position of the query
		len = request[i];
		tmp = 0;
		do { 
			memcpy(domainname + tmp, request + i+1, len);
			tmp += len;
			*(domainname + tmp) = '.'; ++tmp;
			i += len + 1;
			len = request[i];
		} while (len != 0);
		*(domainname + tmp - 1) = 0;
		hent = gethostbyname(domainname);

		if (hent == NULL) {
			real_addr = 0;
			memcpy(real_addr_str, "0.0.0.0", INET_ADDRSTRLEN);
		} else {
			real_addr = *(uint32_t*) hent->h_addr;
			Inet_ntop(AF_INET, hent->h_addr, real_addr_str, INET_ADDRSTRLEN);
		}
		msg(MSG_DEBUG, "dns_resolver: received request for domain %s", domainname);

		// build response
		memcpy(response, request, 2); tmp = 2;   // transaction id
		memcpy(response + tmp, "\x81\x80",2); tmp += 2;                                  // message type (response)
		memcpy(response + tmp, request + 4, 2); tmp += 2;                               // number of querys
		memcpy(response + tmp, request + 4, 2); tmp += 2;                               // number of answers
		memcpy(response + tmp, "\x00\x00\x00\x00", 4); tmp += 4;                          // Authority RRs, Additional RRs
		memcpy(response + tmp, request + 12, r - 12); tmp += r - 12;                    // original request
		memcpy(response + tmp, "\xc0\x0c", 2); tmp += 2;                                  // pointer to domain name
		memcpy(response + tmp, "\x00\x01\x00\x01\x00\x00\x00\x3c\x00\x04", 10); tmp += 10; //Response type, ttl and resource data length
		if (real_addr) {
			memcpy(response + tmp, &real_addr, 4); tmp += 4;
			memcpy(returned_addr_str, real_addr_str, INET_ADDRSTRLEN);
		} else {
			memcpy(response + tmp, &resolver->response_addr, 4); tmp += 4;             // ip address
			memcpy(returned_addr_str, resolver->fake_addr.c_str(), INET_ADDRSTRLEN);
		}
		Sendto(socket, response, tmp, 0, (struct sockaddr *)&cliaddr, clilen);


		// OK , DNS name resolved - now enter the logging function
		struct dns_struct* data = (struct dns_struct*) malloc(sizeof(struct dns_struct));

		Inet_ntop(AF_INET, &cliaddr.sin_addr, data->clientIP, INET_ADDRSTRLEN);
		strcpy(data->serverIP,returned_addr_str);
		strcpy(data->realServerIP,real_addr_str);
		strcpy(data->domain,domainname);

		logger_get()->log_struct(logger_get(), &resolver->conn, "", data);
		}
}

static void sig_int(int signo)
{
	Exit(0);
}

