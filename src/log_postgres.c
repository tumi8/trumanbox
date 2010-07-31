#include "log_postgres.h"
#include "configuration.h"
#include "msg.h"
#include "definitions.h"
#include "logger.h"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <postgresql/libpq-fe.h>

#define MAX_STATEMENT 8000

static char statement[MAX_STATEMENT];
PGconn *psql;

// checks at first whether the connection is already established, before it tries to connect
int connect_to_db() {
	
	
	if (!psql || PQstatus(psql) != CONNECTION_OK) {
		psql = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'trumanlogs' user = 'trumanbox' password = 'das$)13x!#+23' connect_timeout = '10'");
	}
	else {
		// connection is already established...
		return 1;
	}


	if (!psql) {
		msg(MSG_FATAL,"libpq error : PQconnectdb returned NULL.\n\n");
		return 0;
	}

	if (PQstatus(psql) != CONNECTION_OK) {
		msg(MSG_FATAL,"libpq error: PQstatus(psql) != CONNECTION_OK\n\n");
		return 0;
	}
	return 1;
}


int execute_statement(char* stmt) {
	if (connect_to_db()) {
		PGresult *res = PQexec(psql, stmt);
	    	if (PQresultStatus(res) != PGRES_COMMAND_OK)
		{
			msg(MSG_FATAL,"ERROR: %s",PQresultErrorMessage(res));
			msg(MSG_FATAL,"Could not execute \n %s \n",stmt);
			return 0;
		}
		else 
		{ 
			return 1;
		}
	}
	else {
	//connection error
	return 0;
	}
}



// Returns: Whether connection is successful AND if tables are already present
int lpg_init(struct logger_t* logger)
{
	// TODO:
	// Check if tables are already created
	
	
	return connect_to_db();

}


// returns:
int lpg_deinit(struct logger_t* logger)
{
	return 1;
}


// returns: 1 if creation was successful, 0 if not
int lpg_create_log(struct logger_t* logger)
{
	// todo: create tables if they don't exist yet
	return 1; // by default, all tables are already created
}

// returns: 1 if everything was fine, 0 if error
int lpg_finish_log(struct logger_t* logger)
{
	PQfinish(psql);
	return 1;
}

int lpg_log_text(struct logger_t* logger, connection_t* conn, const char* tag, const char* message)
{
	

	return 0;
}

int lpg_log_struct(struct logger_t* log, connection_t* conn, const char* tag, void* data)
{
	
	switch (conn->app_proto) {
	     // log depening on the protoco
	case SMTP:
	{	
		if (strcmp(tag,"client") == 0) {
			struct smtp_client_struct* logdata =  (struct smtp_client_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into SMTP_CLIENT_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,ClientMessage,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->clientMsg,conn->timestamp
			);
			execute_statement(statement);

		}
		else {
			struct smtp_server_struct* logdata =  (struct smtp_server_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into SMTP_SERVER_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,StatusCode,ServerMessage,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s','%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->statusCode,logdata->serverMsg,conn->timestamp
			);
			execute_statement(statement);


		}
	}
	break;

	case HTTP:
	{	
		if (strcmp(tag,"client") == 0) {
			struct http_client_struct* logdata =  (struct http_client_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into HTTP_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,requestedhost,requestedlocation,useragent,method,requestheader,requestbodybinarylocation,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', '%s', '%s',  '%s', '%s','%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->requestedHost,logdata->requestedLocation,logdata->userAgent,logdata->method,logdata->requestHeader,logdata->requestBodyBinaryLocation,conn->timestamp
			);
			execute_statement(statement);
			
		}
		else {
			struct http_server_struct* logdata =  (struct http_server_struct *) data;
			snprintf(statement, MAX_STATEMENT, "update HTTP_LOGS set servertype = '%s', responsecontenttype = '%s', responselastmodified = '%s', responseheader = '%s', responsebodybinarylocation = '%s' where trumantimestamp = '%s'",
			logdata->serverType,logdata->responseContentType,logdata->responseLastModified,logdata->responseHeader,logdata->responseBodyBinaryLocation,conn->timestamp
			);
			execute_statement(statement);				    

		
		}
	 	break;
	}

	break;
	case IRC:
	{	
		if (strcmp(tag,"client") == 0) {
			struct irc_client_struct* logdata =  (struct irc_client_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into IRC_CLIENT_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,Command,Arguments,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', '%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->command,logdata->arguments,conn->timestamp
			);
			execute_statement(statement);

		}
		else {
			struct irc_server_struct* logdata =  (struct irc_server_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into IRC_SERVER_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,ServerName,NumericReply,RecipientNickname,Message,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s','%s','%s','%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->serverName,logdata->numericReply,logdata->recipientNickname,logdata->message,conn->timestamp
			);
			execute_statement(statement);

		
		}
	 break;
	}
	case UNKNOWN:
	{
		if (strcmp(conn->dest,"") == 0) {
			snprintf(conn->dest,IPLENGTH,"0.0.0.0");
		}
		msg(MSG_DEBUG,"Unknown Logging: %s, %d, %s, %s, %d",conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport);
		if (strcmp(tag,"client") == 0) {
			struct unknown_client_struct* logdata =  (struct unknown_client_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into UNKNOWN_CLIENT_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,ClientMessage,ClientMessageBinaryLocation,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', '%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->clientMsg,logdata->clientMsgBinaryLocation,conn->timestamp
			);
			execute_statement(statement);

		}
		else {
			struct unknown_server_struct* logdata =  (struct unknown_server_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into UNKNOWN_SERVER_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,serverMessage,serverMessageBinaryLocation,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s','%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->serverMsg,logdata->serverMsgBinaryLocation,conn->timestamp
			);
			execute_statement(statement);

	
		}
	 	break;	
	}

	case DNS:
	{
			struct dns_struct* logdata =  (struct dns_struct *) data;
			msg(MSG_DEBUG,"logdata: [%s:%s:%s]",logdata->realServerIP,logdata->serverIP,logdata->clientIP);
			snprintf(statement, MAX_STATEMENT, "insert into DNS_LOGS (clientIP,ServerIP,RealServerIP,DomainName,date,trumantimestamp) Values (inet('%s'),inet('%s'),inet('%s'),'%s', (select current_timestamp),'%s')",
			logdata->clientIP,logdata->realServerIP,logdata->serverIP,logdata->domain,conn->timestamp
			);
			execute_statement(statement);

	
	}
	default:
		{
		msg(MSG_DEBUG, "Protocol not yet handled, abort...");
		}


 	} // end of switch
	return 0;
} // end of lpg_log_struct

