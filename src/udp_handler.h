#ifndef _UDP_HANDLER_H_
#define _UDP_HANDLER_H_

#include <common/definitions.h>
#include "protocols/proto_ident.h"
#include "protocols/proto_handler.h"

class Configuration;

class UdpHandler {
	public:
		UdpHandler(int udpfd, const Configuration& config, connection_t* conn, ProtoIdent* ident, struct proto_handler_t** ph);
		~UdpHandler();
	
		void run();

	private:
		void determinTtarget(protocols_app app_proto, struct sockaddr_in* targetServAddr);

		int udpfd;
	        operation_mode_t mode; // get from config
		struct configuration_t* config;
		int sock;
		connection_t* connection;
		int connectedToFinal;
		ProtoIdent* protoIdent;
		struct proto_handler_t** ph;
};


#endif
