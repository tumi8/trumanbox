#ifndef _PROTO_IDENT_H_
#define _PROTO_IDENT_H_

#include "definitions.h"

struct proto_identifier_t;
struct configuration_t;

/* This function receives a descriptor from an established connection and an
 * connection object and tries to read as many bytes from the connection as are
 * needed to classify the connection or reach the decision that this connection
 * cannot be classified. The read bytes are stored to payload and the
 * identified protocol is returned.
 */
typedef protocols_app (pi_identify_from_conn)(struct proto_identifier_t*, connection_t* conn, int connfd, char* payload, ssize_t* payload_len);
typedef int (pi_init)(struct proto_identifier_t* p);
typedef int (pi_deinit)(struct proto_identifier_t* p);
typedef protocols_app (pi_byport)(struct proto_identifier_t* p, connection_t *conn);
typedef protocols_app (pi_bypayload)(struct proto_identifier_t* p, connection_t *conn, char *payload, size_t payload_len);

enum epi_type { inbuild, opendpi };
typedef enum epi_type pi_type;


struct proto_identifier_t {
	void* identifier;
	struct configuration_t* config;
	operation_mode_t mode;
	pi_type type;
	pi_identify_from_conn* identify;
	pi_init* init;
	pi_deinit* deinit;
	pi_byport* byport;
	pi_bypayload* bypayload;
};

struct proto_identifier_t* pi_create(struct configuration_t* c, pi_type type);
void pi_destroy(struct proto_identifier_t* p);

#endif
