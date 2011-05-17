#ifndef _TCP_HANDLER_H_
#define _TCP_HANDLER_H_

#include <common/definitions.h>
#include "protocols/proto_ident.h"
#include "protocols/proto_handler.h"

class Configuration;
class SSLHandler;

class TcpHandler {
	public:
		TcpHandler(int inconnfd, const Configuration& config, connection_t* conn, ProtoIdent* ident, struct proto_handler_t** ph);
		~TcpHandler();
	
		void run();

	protected:
		void determineTarget(protocols_app app_proto, struct sockaddr_in* targetServAddr);
		int handleSSL();
		int handleUnknown(struct sockaddr_in* targetServAddr);

		operation_mode_t mode; // get from config
		const Configuration& config;
		int sock;
		connection_t* connection;
		int inConnFd;
		int sslMitmActive;
		int nepenthesActive;
		int targetServiceFd;
		int connectedToFinal;
		class ProtoIdent* protoIdent;
		struct proto_handler_t** ph;
	friend class SSLHandler;
};


#endif
