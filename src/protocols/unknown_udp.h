#ifndef _PROTO_UNKNOWN_UDP_H_
#define _PROTO_UNKNOWN_UDP_H_

#include "configuration.h"
#include <netinet/in.h>

class UnknownUdpHandler : public ProtoHandler
{
	public:
		UnknownUdpHandler(const Configuration& config);	
		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len);
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len);
		virtual int determineTarget(struct sockaddr_in* addr);


};


void* ph_unknown_udp_create();
int ph_unknown_udp_destroy(void*);

int ph_unknown_udp_init(void* handler, struct configuration_t* c);
int ph_unknown_udp_deinit(void* handler);
int ph_unknown_udp_handle_payload_stc(void* handler, connection_t* conn, const char* payload, ssize_t* len);
int ph_unknown_udp_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len);
int ph_unknown_udp_handle_packet(void* handler, const char* packet, ssize_t len);
int ph_unknown_udp_determine_target(void* handler, struct sockaddr_in*);


#endif
