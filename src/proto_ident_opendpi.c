#ifdef WITH_OPENDPI

#include "proto_ident_opendpi.h"

#include <stdlib.h>
#include <ipq_api.h>

static struct odpi {
	struct ipoque_detection_module_struct *ipoque_struct;
};

int pi_opendpi_init(struct proto_identifier_t* p)
{
	struct odpi* o = (struct odpi*)malloc(sizeof(struct odpi));
	//o->ipoque_struct

	p->identifier = (void*)o;
	return 0;
}

int pi_opendpi_deinit(struct proto_identifier_t* p)
{
	return 0;
}

protocols_app pi_opendpi_port(struct proto_identifier_t* pi, connection_t *conn, char *payload)
{
	// OpenDPI cannot classify by only looking at the port
	return UNKNOWN;
}

protocols_app pi_opendpi_payload(struct proto_identifier_t* pi, connection_t *conn, int inconnfd, char *payload)
{
	struct odpi* o = (struct odpi*)pi->identifier;
	return UNKNOWN;
}

#endif
