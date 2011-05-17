#ifndef _DNS_RESOLVER_H_
#define _DNS_RESOLVER_H_

#include <common/definitions.h>

#include <stdint.h>
#include <string>

class Configuration;

class DNSResolver {
public:
	DNSResolver(const Configuration& conf);

	void start();
	void stop();

private:
	std::string listen_ip;
	std::string fake_addr;
	uint16_t port;
	uint8_t return_orig;
	uint32_t response_addr;
	pid_t pid;
	connection_t conn;

	static void dns_worker(DNSResolver* resolver);

};


#endif
