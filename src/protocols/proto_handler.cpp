#include "proto_handler.h"
#include "irc.h"
#include "smtp.h"
#include "http.h"
#include "ftp.h"
#include "ssl.h"
#include "ftp_data.h"
#include "unknown.h"
#include "unknown_udp.h"
#include <stdlib.h>

#include <common/definitions.h>
#include <common/msg.h>
#include <common/configuration.h>

ProtoHandler* create_handler(const Configuration& config, protocols_app app)
{
	switch (app) {
	case FTP:
	case FTP_anonym:
		return new FTPHandler(config);
	case FTP_data:
		return new FTPDataHandler(config);
	case SMTP:
		return new SMTPHandler(config);
	case HTTP:
		return new HTTPHandler(config);
	case IRC:
		return new IRCHandler(config);
	case UNKNOWN_UDP:
		return new UnknownUdpHandler(config);
	case SSL_Proto:
		return new SSLHandler(config);
	case DNS:
		// DNS is handled by dns_resolver and not by dispatcher,
		// thre is no need for a handler here	
	default:
		return new UnknownHandler(config);
	}
}

std::map<protocols_app, ProtoHandler*> ph_create(const Configuration& config)
{
	std::map<protocols_app, ProtoHandler*> ret;
	protocols_app i = (protocols_app)0;
	while (i != UNKNOWN) {
		msg(MSG_DEBUG, "Creating protocol handler for %d", i);
		ret[i] = create_handler(config, i);
		i = (protocols_app)(((int)i) +  1);
	}
	msg(MSG_DEBUG, "Created every handler!");

	return ret;
}


int ph_destroy(ProtoHandler** p)
{
	int i = 0;
	for (i = 0; i != UNKNOWN; i++) {
		delete p[i];
	}
	delete p;
	return 0;
}

