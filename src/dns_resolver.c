#include "dns_resolver.h"
#include "helper_net.h"
#include "msg.h"

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

// XXX: check with RFC
#define MAX_REQUEST_LEN 1000
#define MAX_DOMAIN_LEN 1000

struct dns_resolver_t {
	char listen_ip[13];
	uint16_t port;
	uint8_t return_orig;
	pthread_t thread;
	int running;
	uint32_t response_addr;
};

struct dns_resolver_t* dns_create_resolver(const char* listen_address, uint16_t listen_port, const char* answer_address, uint8_t return_orig)
{
	struct dns_resolver_t* ret = (struct dns_resolver_t*)malloc(sizeof(struct dns_resolver_t));
	strncpy(ret->listen_ip, listen_address, 13);
	ret->port = listen_port;
	ret->return_orig = return_orig;
	ret->running = 0;
	inet_pton(AF_INET, answer_address, &ret->response_addr);

	return ret;
}

static void* dns_resolver_thread(void* data)
{
	int socket;
	struct sockaddr_in saddr, cliaddr;
	struct dns_resolver_t* resolver;
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

	resolver = (struct dns_resolver_t*)data;

	socket = Socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port        = htons(resolver->port);

	Bind(socket, (SA *) &saddr, sizeof(saddr));

	while (resolver->running) {
		FD_ZERO(&rset);
		FD_SET(socket, &rset);
		tv.tv_sec = 1; 
		tv.tv_usec = 0;

		maxfd = socket + 1; 
		clilen = sizeof(cliaddr);

		while (select(maxfd, &rset, NULL, NULL, &tv)) {
			if (FD_ISSET(socket, &rset)) {
				r = Recvfrom(socket, request, MAX_REQUEST_LEN, 0, (SA *)&cliaddr, &clilen);
				if (r > 17) {
					// only handle A requests
					opcode = (request[2] >> 3) & 15;
					if (opcode != 0) { 
						msg(MSG_ERROR, "Unknown DNS request %d", opcode);
					} else {
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
						*(domainname + tmp -1) = 0;
						hent = gethostbyname(domainname);
						if (hent == NULL) {
							real_addr = 0;
						} else {
							real_addr = *(uint32_t*)hent->h_addr;
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
						} else {
							memcpy(response + tmp, &resolver->response_addr, 4); tmp += 4;                  // ip address
						}
						Sendto(socket, response, tmp, 0, (struct sockaddr *)&cliaddr, clilen);
					}
				} else {
					// definately malformed DNS request
					msg(MSG_ERROR, "dns_resolver: received malformed request");
				}
			}
			FD_ZERO(&rset);
			FD_SET(socket, &rset);
			tv.tv_sec = 1; 
			tv.tv_usec = 0;

		}
	}
	return NULL;
}

void dns_start_resolver(struct dns_resolver_t* r)
{
	int err;
	r->running = 1;
	if ((err = pthread_create(&r->thread, NULL, dns_resolver_thread, (void*)r))) {
		msg(MSG_ERROR, "Could not create resolver thread: %s", strerror(errno));
		exit(-1);
	}
}

void dns_stop_resolver(struct dns_resolver_t* r)
{
	r->running = 0;
	pthread_join(r->thread, NULL);
}

void dns_destroy_resolver(struct dns_resolver_t* r)
{
	free(r);
}
