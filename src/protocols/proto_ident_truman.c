#include "proto_ident_truman.h"
#include "helper_net.h"
#include "msg.h"
#include "string.h"

#include <stdio.h>

int pi_buildin_init(struct proto_identifier_t* p) { return 0; }
int pi_buildin_deinit(struct proto_identifier_t* p) { return 0; }

char *strcasestr(const char *haystack, const char *needle);

protocols_app pi_buildin_port(struct proto_identifier_t* pi, connection_t *conn)
{
	// here we will still implement the check, if we already know the answer by checking if we have a reponse file with corresponding ip:port name and feed payload with it
	
	// FIXME: unused variables
	// int 			fd, n, 
	//int			anonym_ftp;

	switch (conn->dport) {
		case 21:
			conn->app_proto = FTP;
			break;
		case 25:
			conn->app_proto = SMTP;
			break;
		case 80:
			conn->app_proto = HTTP;
			break;
		case 6667:
			conn->app_proto = IRC;
			break;
		default:
			conn->app_proto = UNKNOWN;
			break;
	}
	//if (conn->app_proto == FTP || conn->app_proto == SMTP)
		//*payload_len = fetch_banner(pi->mode, conn, payload, &anonym_ftp);

	//if (conn->app_proto == FTP && anonym_ftp)
	//conn->app_proto = FTP_anonym;

	msg(MSG_DEBUG, "protocol identified by port is: %d", conn->app_proto);
	return conn->app_proto;
}

protocols_app pi_buildin_payload(struct proto_identifier_t* pi, connection_t *conn, char *payload, size_t payload_len) {
	// here we need to implement logging of responses to file
	//int			r, anonym_ftp;
	char			filename[30];

	conn->app_proto = UNKNOWN;

//	if (!r) {
//		//r = fetch_banner(pi->mode, conn, payload, &anonym_ftp);
//		msg(MSG_DEBUG, "the payload we fetched is:\n%s", payload);
//	}

	//msg(MSG_DEBUG, "Received payload: \"%s\"\n", payload);
	if (payload_len > 0) {
		if (strncmp(payload, "GET /", 5) == 0)
			conn->app_proto = HTTP;
		else if (strncmp(payload, "NICK ", 5) == 0)
			conn->app_proto = IRC;
		else if (strncmp(payload, "FTP_data", 8) == 0) {
			conn->app_proto = FTP_data;
			sprintf(filename, "%s:%d", conn->dest, conn->dport);
			if (0 > remove((char *)filename))
				msg(MSG_ERROR, "could not remove the file: %s", (char *) filename);
			else
				msg(MSG_DEBUG, "and now we removed the file: %s", (char *) filename);
		}
		else if (strncmp(payload, "220 ", 4) == 0) {
			if ( strcasestr(payload, "ftp") != 0 ) {
				conn->app_proto = FTP;
				//if (anonym_ftp)
				//	conn->app_proto = FTP_anonym;
			}
			else if ( (strcasestr(payload, "mail") != 0) || (strcasestr(payload, "smtp") != 0) ) {
				conn->app_proto = SMTP;
			}
			else {
				conn->app_proto = UNKNOWN;
			}
		}
		else {
			conn->app_proto = UNKNOWN;
		}
	}

	msg(MSG_DEBUG, "protocol identified by payload is: %d", conn->app_proto);
	return conn->app_proto;
}

