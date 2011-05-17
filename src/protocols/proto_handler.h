#ifndef _PROTO_HANDLER_H_
#define _PROTO_HANDLER_H_

#include <common/definitions.h>

#include <netinet/in.h>
#include <map>

class Configuration;

class ProtoHandler {
	public:
		ProtoHandler(const Configuration& config);

		virtual int payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len) = 0;
		virtual int payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len) = 0;
		virtual int determineTarget(struct sockaddr_in* addr) = 0;

	protected:
		const Configuration& config;
};

/* Returns an array of protocolhandler. The size of the array is 
 * sizof(protocols_app) (where protcols_app is defined in definitions.h).
 * Each element of result[i] is a protocol handler for protocols_app[i]
 */
std::map<protocols_app, ProtoHandler*> ph_create(const Configuration& config);


/* Frees all protcol handlers. This function will call protcolhandler_t->deinit for
 * all associated protocol handlers. 
 */
int ph_destroy(std::map<protocols_app, ProtoHandler*> handlers);

#endif
