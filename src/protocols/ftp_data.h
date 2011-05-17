#ifndef _PROTO_FTP_DATA_H_
#define _PROTO_FTP_DATA_H_

#include <netinet/in.h>

#include <common/configuration.h>
#include "proto_handler.h"

class FTPDataHandler : public ProtoHandler
{
	public:
		FTPDataHandler(const Configuration& config);	
		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len);
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len);
		virtual int determineTarget(struct sockaddr_in* addr);
};

#endif
