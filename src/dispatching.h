#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <common/definitions.h>

class Configuration;
class ProtoIdent;

class Dispatcher
{
	public:
		Dispatcher(const Configuration& c);
		~Dispatcher();
		void run();
	
	private:
		int parse_conntrack(connection_t *conn);
		int controlfd;
		int tcpfd;
		int udpfd;
		struct proto_handler_t** ph;
		int running;
		const Configuration& config;
		ProtoIdent* protoIdent;
		
};

#endif
