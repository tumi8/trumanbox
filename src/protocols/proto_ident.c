#include "proto_ident.h"
#include "proto_ident_truman.h"
#include "proto_ident_opendpi.h"
#include "helper_file.h"
#include "helper_net.h"
#include "msg.h"
#include "configuration.h"

#include <stdlib.h>

//protocols_app pi_identify(struct proto_identifier_t* pi, connection_t* conn, int connfd, char* payload, ssize_t* payload_len)
//{
//	pi->bypayload(pi, conn, connfd, payload, payload_len);
//	
//	if (conn->app_proto == UNKNOWN) {
//		msg(MSG_DEBUG, "...failed!\nso we try doing (weak) protocol identification by port...");
//		pi->byport(pi, conn, payload);
//	}
//	
//	if (conn->app_proto == UNKNOWN) {
//		msg(MSG_ERROR, "failed!\nthe protocol could not be identified, so we stop handling this connection.\n "
//				"the dumped payload can be found in %s/%s:%d", DUMP_FOLDER, conn->dest, conn->dport);
//		append_to_file(payload, conn, DUMP_FOLDER);
//		Close_conn(connfd, "incomming connection, because of unknown protocol");
//		return UNKNOWN;
//	}
//
//	return conn->app_proto;
//}

struct proto_identifier_t* pi_create(struct configuration_t* config, pi_type type)
{
	struct proto_identifier_t* result = (struct proto_identifier_t*)malloc(sizeof(struct proto_identifier_t));
	result->config = config;
	result->mode = conf_get_mode(config);
	result->type = type;
	//result->identify = pi_identify;
	switch (type) {
	case inbuild:
		result->init = pi_buildin_init;
		result->deinit = pi_buildin_deinit;
		result->byport = pi_buildin_port;
		result->bypayload = pi_buildin_payload;
		break;
	case opendpi:
#ifdef WITH_OPENDPI
		result->init = pi_opendpi_init;
		result->deinit = pi_opendpi_deinit;
		result->byport = pi_opendpi_port;
		result->bypayload = pi_opendpi_payload;
#else
		msg(MSG_FATAL, "TrumanBox was compiled without OpenDPI support."
				"Please recompile TrumanBox with OpenDPI support or disable opendpi in the config file!");
		exit(-1);
#endif
	}
	return result;
}

void pi_destroy(struct proto_identifier_t* p)
{
	free(p);
}

