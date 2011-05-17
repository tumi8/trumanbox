#ifndef _PROTO_IDENT_TRUMAN_H_
#define _PROTO_IDENT_TRUMAN_H_

class Configuration;

#include "proto_ident.h"

#include <common/definitions.h>

class ProtoIdentTruman : public ProtoIdent {
	public:
		ProtoIdentTruman(const Configuration& config);
		
		virtual protocols_app identify(connection_t *conn, char* payload, size_t payload_len);
		virtual protocols_app identify(connection_t *conn);
};


#endif
