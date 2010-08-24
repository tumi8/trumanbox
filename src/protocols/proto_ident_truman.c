#include "proto_ident_truman.h"
#include <stdlib.h>
#include "helper_net.h"
#include "msg.h"
#include "string.h"
#include "log_postgres.h"
#include <stdio.h>
#include "helper_file.h"

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

	msg(MSG_DEBUG, "Received payload: \"%s\"\n", payload);
	if (payload_len > 0) {
		if (strstr(payload,"HTTP/")!=0) {
			conn->app_proto = HTTP;
		}
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
		else if (strncmp(payload, "220", 3) == 0) {
			if ( strcasestr(payload, "ftp") != 0 || strcasestr(payload,"welcome") != 0 ) {
				conn->app_proto = FTP;
				//if (anonym_ftp)
				//	conn->app_proto = FTP_anonym;
			}
			else if ( (strcasestr(payload, "mail") != 0) || (strcasestr(payload, "smtp") != 0) ) {
				conn->app_proto = SMTP;
			}
			else {
				conn->app_proto = SMTP;
				//conn->app_proto = UNKNOWN;
			}
		}
		else {

		
                if (payload_len > 0 ) {
			
			bzero(conn->sslVersion,100);	
			
			if (payload[0] == '\x16') {
				msg(MSG_DEBUG,"Handshake");
                        	if (payload[1] == '\x3' && payload[2] == '\x0') strcpy(conn->sslVersion,"SSL v3 ");
                        	else if (payload[1] == '\x3' && payload[2] == '\x1') strcpy(conn->sslVersion,"TLS 1.0 ");
                        	else if (payload[1] == '\x3' && payload[2] == '\x2') strcpy(conn->sslVersion,"TLS 1.1 ");
                        	else if (payload[1] == '\x3' && payload[2] == '\x3') strcpy(conn->sslVersion,"TLS 1.2 ");
				else strcpy(conn->sslVersion,"Unknown (new/old) SSL version ");
                        	char MessageType[100];
				bzero(MessageType,100);
                       		switch (payload[5]) {
                                case '\x0':  strcpy(MessageType,"HelloRequest"); break;
                                case '\x1':  strcpy(MessageType,"Client Hello"); break;
                                case '\x2':  strcpy(MessageType,"Server Hello"); break;
                                case '\xb':  strcpy(MessageType,"Certificate"); break;
                                case '\xc':  strcpy(MessageType,"ServerKeyExchange"); break;
                                case '\xd':  strcpy(MessageType,"CertificateRequest"); break;
                                case '\xe':  strcpy(MessageType,"ServerHelloDone"); break;
                                case '\xf':  strcpy(MessageType,"Certificate Verify"); break;
                                case '\xf0':  strcpy(MessageType,"ClientKeyExchange"); break;
                                case '\xf4':  strcpy(MessageType,"Finished"); break;
                        }
			if (MessageType != NULL)  {
			
                        	strcat(conn->sslVersion,MessageType);
				conn->app_proto = SSL_Proto;
				msg(MSG_DEBUG, "protocol identified by payload is: %d [%s]", conn->app_proto,conn->sslVersion);
				return conn->app_proto;

			}


                	}
                	else if (payload[0] == '\x17')
               			msg(MSG_DEBUG,"application");
		
			else if (payload[2] == '\x1') {
				if (payload[0] && 0x80 == 1) msg(MSG_DEBUG,"length field uses only 1 byte");
				
				if (payload[3] == '\x3' && payload[4] == '\x0') strcpy(conn->sslVersion,"SSL v3");
               			else if (payload[3] == '\x3' && payload[4] == '\x1') strcpy(conn->sslVersion,"TLS 1.0");
                        	else if (payload[3] == '\x3' && payload[4] == '\x2') strcpy(conn->sslVersion,"TLS 1.1");
                       		else if (payload[3] == '\x3' && payload[4] == '\x3') strcpy(conn->sslVersion,"TLS 1.2");
	
				if (conn->sslVersion!= NULL) {
					strcat(conn->sslVersion," and SSL v2-Client Hello");
					msg(MSG_DEBUG,"we got: [%s]",conn->sslVersion);
					conn->app_proto= SSL_Proto;
					return conn->app_proto;
				}
			}
		
		}
			conn->app_proto = UNKNOWN;
			
			// check if we can classfiy this tcp traffic with the data from the database

			char statement[1000] = "";
			snprintf(statement, 1000, "select count(*) as anz from awaited_pasv where serverIP = inet('%s') and serverPort = %d",
			conn->orig_dest,conn->dport
			);
			msg(MSG_DEBUG,"[to execute: %s]",statement);
			char value[100];
			execute_query_statement_singlevalue(value,statement);
			if (value != NULL) {
				msg(MSG_DEBUG,"try to extract %s",value);
				int valueInt = atoi(value);
				if (valueInt > 0) {
					conn->app_proto = FTP_data;
				}
			}
	
			msg(MSG_DEBUG,"we have the following information: IPServ: [%s] IPServ2: [%s] PortServ: [%d]",conn->orig_dest,conn->dest,conn->dport);
		}
	}
	msg(MSG_DEBUG, "protocol identified by payload is: %d", conn->app_proto);
	return conn->app_proto;
}

