#ifndef _PROTO_IDENT_OPENDPI_H_
#define _PROTO_IDENT_OPENDPI_H_
#ifdef WITH_OPENDPI

#include "definitions.h"
#include "proto_ident.h"

int pi_opendpi_init(struct proto_identifier_t* p);
int pi_opendpi_deinit(struct proto_identifier_t* p);
protocols_app pi_opendpi_port(struct proto_identifier_t* pi, connection_t *conn, char *payload);
protocols_app pi_opendpi_payload(struct proto_identifier_t* pi, connection_t *conn, int inconnfd, char *payload);


#endif // WITH_OPENDPI
#endif // _PROTO_IDENT_OPENDPI_H_
