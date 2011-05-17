#ifndef _PROTO_SSLPROTO_H_
#define _PROTO_SSLPROTO_H_

#include <common/configuration.h>
#include <netinet/in.h>

#include "proto_handler.h"

class SSLHandler : public ProtoHandler
{
	public:
		SSLHandler(const Configuration& config);	
		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len);
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len);
		virtual int determineTarget(struct sockaddr_in* addr);
};

#endif
