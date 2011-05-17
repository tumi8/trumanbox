#ifndef _PROTO_HTTP_H_
#define _PROTO_HTTP_H_

#include <common/configuration.h>
#include <netinet/in.h>
#include "proto_handler.h"

class HTTPHandler : public ProtoHandler
{
	public:
		HTTPHandler(const Configuration& config);	
		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len);
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len);
		virtual int determineTarget(struct sockaddr_in* addr);
	private:
		void emulate_server_response(struct http_client_struct* data,connection_t* conn); 
		void manipulate_server_payload(const char* payload, connection_t* conn, ssize_t* len);
		void extract_http_header_field(char* destination, char* headername, char* header);
};

#endif
