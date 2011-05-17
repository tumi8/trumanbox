#ifndef _PROTO_SMTP_H_
#define _PROTO_SMTP_H_

#include <netinet/in.h>

#include "proto_handler.h"

class SMTPHandler : public ProtoHandler
{
	public:
		SMTPHandler(const Configuration& config);	
		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len);
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len);
		virtual int determineTarget(struct sockaddr_in* addr);
};

#endif
