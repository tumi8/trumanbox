#ifndef _PROTO_IDENT_TRUMAN_H_
#define _PROTO_IDENT_TRUMAN_H_

#include "definitions.h"
#include "proto_ident.h"

int pi_buildin_init(struct proto_identifier_t* p);
int pi_buildin_deinit(struct proto_identifier_t* p);
protocols_app pi_buildin_port(struct proto_identifier_t* pi, connection_t *conn);
protocols_app pi_buildin_payload(struct proto_identifier_t* pi, connection_t *conn, char *payload, size_t payload_len);

#endif
