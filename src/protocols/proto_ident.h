#ifndef _PROTO_IDENT_H_
#define _PROTO_IDENT_H_

#include <common/definitions.h>
#include <stdint.h>

class Configuration;

/* This class is a base class for all protocol identifier that take
 * payload from client/server communication and try to identify the 
 * application protocol involved in the communication.
 */
class ProtoIdent {
	public:
		ProtoIdent(const Configuration& config) : config(config)  {}
		
		virtual protocols_app identify(connection_t *conn, char* payload, size_t payload_len) = 0;
		virtual protocols_app identify(connection_t *conn) = 0;
	protected:
		const Configuration& config;
};


ProtoIdent* pi_create(const Configuration& config);

#endif
