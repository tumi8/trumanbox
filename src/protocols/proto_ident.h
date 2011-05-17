#ifndef _PROTO_IDENT_H_
#define _PROTO_IDENT_H_

#include <common/definitions.h>
#include <stdint.h>

/* This class is a base class for all protocol identifier that take
 * payload from client/server communication and try to identify the 
 * application protocol involved in the communication.
 */
class ProtoIdent {
	public:
		ProtoIdent();
		
		virtual protocols_app identify(connection_t *conn, char* payload, size_t payload_len);
		virtual protocols_app idenfify(connection_t *conn, uint16_t port);
};

#endif
