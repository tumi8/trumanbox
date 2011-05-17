#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <common/definitions.h>
#include <protocols/proto_handler.h>

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
		std::map<protocols_app, ProtoHandler*> protoHandlers;
		int running;
		const Configuration& config;
		ProtoIdent* protoIdent;
		
};

int parse_conntrack(connection_t *conn);


#endif
