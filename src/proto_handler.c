#include "proto_handler.h"
#include "definitions.h"
#include "msg.h"

#include "protocols/irc.h"
#include "protocols/smtp.h"
#include "protocols/http.h"
#include "protocols/ftp.h"

#include <stdlib.h>

static struct protohandler_t* create_handler(protocols_app app)
{
	struct protohandler_t* ret = (struct protohandler_t*)malloc(sizeof(struct protohandler_t));
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
		break;
	case SMTP:
		ret->handler = ph_smtp_create();
		ret->init = ph_smtp_init;
		ret->deinit = ph_smtp_deinit;
		ret->handle_payload_stc = ph_smtp_handle_payload_stc;
		ret->handle_payload_cts = ph_smtp_handle_payload_cts;
		ret->handle_packet = ph_smtp_handle_packet;
		break;
	case HTTP:
		ret->handler = ph_http_create();
		ret->init = ph_http_init;
		ret->deinit = ph_http_deinit;
		ret->handle_payload_stc = ph_http_handle_payload_stc;
		ret->handle_payload_cts = ph_http_handle_payload_cts;
		ret->handle_packet = ph_http_handle_packet;
		break;
	case IRC:
		ret->handler = ph_irc_create();
		ret->init = ph_irc_init;
		ret->deinit = ph_irc_deinit;
		ret->handle_payload_stc = ph_irc_handle_payload_stc;
		ret->handle_payload_cts = ph_irc_handle_payload_cts;
		ret->handle_packet = ph_irc_handle_packet;
		break;
	default:
		msg(MSG_FATAL, "No handler for protocol %d defined! This is a bug (and will result in segmentation faults! Aborting!", app);
		exit(-1);
	}
	return ret;
}

struct protohandler_t** ph_create(struct configuration_t* c)
{
	struct protohandler_t** result = (struct protohandler_t**)malloc(sizeof(struct protohandler_t*)*UNKNOWN);
	int i = 0;
	for (i = 0; i != UNKNOWN; i++) {
		result[i] = create_handler(i);
		result[i]->init(result[i]->handler, c);
	}

	return result;
}


int ph_destroy(struct protohandler_t** p)
{
	int i = 0;
	for (i = 0; i != UNKNOWN; i++) {
		free(p[i]);
	}
	free(p);
	return 0;
}

