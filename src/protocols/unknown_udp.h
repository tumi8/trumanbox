#ifndef _PROTO_UNKNOWN_UDP_H_
#define _PROTO_UNKNOWN_UDP_H_

#include <common/configuration.h>
#include "proto_handler.h"
#include <netinet/in.h>

class UnknownUdpHandler : public ProtoHandler
{
	public:
		UnknownUdpHandler(const Configuration& config);	
		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len);
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len);
		virtual int determineTarget(struct sockaddr_in* addr);
};

#endif
