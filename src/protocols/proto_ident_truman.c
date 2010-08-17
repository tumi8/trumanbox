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
	

			/*PGconn* psql = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'trumanlogs' user = 'trumanbox' password = 'das$)13x!#+23' connect_timeout = '10'");
				PGresult *res = PQexec(psql, statement);
				if (PQresultStatus(res) != PGRES_TUPLES_OK)
					{
					msg(MSG_FATAL,"Status: %s",PQresStatus(PQresultStatus(res)));
					msg(MSG_FATAL,"%s",PQresultErrorMessage(res));
i
					}
				else {	
					char* value =  PQgetvalue(res,0,0);
					msg(MSG_DEBUG,"we have value: %s",value);
						
					int valueInt = atoi(value);
					if (valueInt > 0) {
						conn->app_proto = FTP_data;
					}
					else {
						// not found, do nothing
					}
																														}
			
			PQfinish(psql);
				*/

			msg(MSG_DEBUG,"we have the following information: IPServ: [%s] IPServ2: [%s] PortServ: [%d]",conn->orig_dest,conn->dest,conn->dport);
		}
	}
	msg(MSG_DEBUG, "protocol identified by payload is: %d", conn->app_proto);
	return conn->app_proto;
}

