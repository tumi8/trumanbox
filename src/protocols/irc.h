#ifndef _PROTO_IRC_H_
#define _PROTO_IRC_H_

#include <common/configuration.h>
#include "proto_handler.h"
#include <netinet/in.h>

class IRCHandler : public ProtoHandler
{
	public:
		IRCHandler(const Configuration& config);	
		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len);
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len);
		virtual int determineTarget(struct sockaddr_in* addr);


};

#endif
