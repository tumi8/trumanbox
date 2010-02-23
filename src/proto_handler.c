#include "proto_handler.h"
#include "definitions.h"
#include "msg.h"

#include "protocols/irc.h"
#include "protocols/smtp.h"
#include "protocols/http.h"
#include "protocols/ftp.h"
#include "protocols//unknown.h"

#include <stdlib.h>

static struct proto_handler_t* create_handler(protocols_app app)
{
	struct proto_handler_t* ret = (struct proto_handler_t*)malloc(sizeof(struct proto_handler_t));
	switch (app) {
	case FTP:
	case FTP_data:
	case FTP_anonym:
		ret->handler = ph_ftp_create();
		ret->init = ph_ftp_init;
		ret->deinit = ph_ftp_deinit;
		ret->handle_payload_stc = ph_ftp_handle_payload_stc;
		ret->handle_payload_cts = ph_ftp_handle_payload_cts;
		ret->handle_packet = ph_ftp_handle_packet;
		ret->determine_target = ph_ftp_determine_target;
		break;
	case SMTP:
		ret->handler = ph_smtp_create();
		ret->init = ph_smtp_init;
		ret->deinit = ph_smtp_deinit;
		ret->handle_payload_stc = ph_smtp_handle_payload_stc;
		ret->handle_payload_cts = ph_smtp_handle_payload_cts;
		ret->handle_packet = ph_smtp_handle_packet;
		ret->determine_target = ph_smtp_determine_target;
		break;
	case HTTP:
		ret->handler = ph_http_create();
		ret->init = ph_http_init;
		ret->deinit = ph_http_deinit;
		ret->handle_payload_stc = ph_http_handle_payload_stc;
		ret->handle_payload_cts = ph_http_handle_payload_cts;
		ret->handle_packet = ph_http_handle_packet;
		ret->determine_target = ph_http_determine_target;
		break;
	case IRC:
		ret->handler = ph_irc_create();
		ret->init = ph_irc_init;
		ret->deinit = ph_irc_deinit;
		ret->handle_payload_stc = ph_irc_handle_payload_stc;
		ret->handle_payload_cts = ph_irc_handle_payload_cts;
		ret->handle_packet = ph_irc_handle_packet;
		ret->determine_target = ph_irc_determine_target;
		break;
	case DNS:
		// DNS is handled by dns_resolver and not by dispatcher,
		// thre is no need for a handler here
	default:
		ret->handler = ph_unknown_create();
		ret->init = ph_unknown_init;
		ret->deinit = ph_unknown_deinit;
		ret->handle_payload_stc = ph_unknown_handle_payload_stc;
		ret->handle_payload_cts = ph_unknown_handle_payload_cts;
		ret->handle_packet = ph_unknown_handle_packet;
		ret->determine_target = ph_unknown_determine_target;
	}
	return ret;
}

struct proto_handler_t** ph_create(struct configuration_t* c)
{
	struct proto_handler_t** result = (struct proto_handler_t**)malloc(sizeof(struct proto_handler_t*)*UNKNOWN);
	int i = 0;
	for (i = 0; i <= UNKNOWN; i++) {
		msg(MSG_DEBUG, "Creating protocol handler for %d", i);
		result[i] = create_handler(i);
		msg(MSG_DEBUG, "Created protocol handler for %d ... Initializing handler ...", i);
		result[i]->init(result[i]->handler, c);
		msg(MSG_DEBUG, "Finished handler initialization");
	}
	msg(MSG_DEBUG, "Created every handler!");

	return result;
}


int ph_destroy(struct proto_handler_t** p)
{
	int i = 0;
	for (i = 0; i != UNKNOWN; i++) {
		free(p[i]);
	}
	free(p);
	return 0;
}

