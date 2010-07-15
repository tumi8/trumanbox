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
#include </usr/include/postgresql/libpq-fe.h>

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
	
	snprintf(statement, MAX_STATEMENT, "INSERT into SMTP_LOGS () values vlqa");
	
	// BIG FAT TODO: sql injection ....
	switch (conn->app_proto) {
	// log depening on the protocol	
	case SMTP:
	//	if (execute_stmt(statement)) {
	//		msg(MSG_ERROR, "Could not execute: \n %s", statement);
	//	}
		msg(MSG_DEBUG,"SMTP Logging attempt");
		break;
	case FTP:
		msg(MSG_DEBUG,"FTP Logging attempt");
		break;
	case FTP_anonym:
		msg(MSG_DEBUG,"FTP anonym Logging attempt");
		break;
	case FTP_data:
		msg(MSG_DEBUG,"FTP data Logging attempt");
		break;
	case HTTP:
	
		msg(MSG_DEBUG,"HTTP Logging attempt");
		/*if (strcmp(tag,"client") == 0) {
			msg(MSG_DEBUG,"We have a client request log");
		// we perform a log operation for a client request
		// try to extract host, url, user-agent
		char header[1000];
		char userAgent[1000];
		char requestedHost[1000];
		char requestedLocation[1000];
		char method[20];
		char* ptrToBody = NULL; //this pointer contains the address of the body of the POST-Request
		char* ptrToHeader = header;
		char* ptrToRequestedLocation = NULL;

		ptrToBody  = strstr(message, "\r\n\r\n"); // skip the new lines/ carriage returns
		int bodyLength = strlen(ptrToBody);
		int completeLength = strlen(message);
		int headerLength = completeLength - bodyLength;
		
		
		

		// HEADER extractor
		strncpy(header,message,headerLength);
		*(ptrToHeader+headerLength+1) = '\0';
		msg(MSG_DEBUG,"Header: %s",header);


		// METHOD extractor
		int methodLength = strcspn(header," ");
		strncpy(method,header,methodLength);
		method[methodLength] = '\0';
		msg(MSG_DEBUG,"Method: '%s'",method);	

		// LOCATION extractor 
		
		ptrToRequestedLocation = strstr(header,"/");
		int locationLength = strcspn(ptrToRequestedLocation," ");
		strncpy(requestedLocation,ptrToRequestedLocation,locationLength);
		requestedLocation[locationLength] = '\0';
		msg(MSG_DEBUG,"Location: '%s'",requestedLocation);
		
		extract_header_field(requestedHost,"Host:",header);
		extract_header_field(userAgent,"User-Agent:",header);

		snprintf(statement,MAX_STATEMENT, "INSERT into %s (RequesterIP, DestinationIP, TrueDestinationIP, RequestHeader, Date, RequestedHost, RequestedLocation, UserAgent, Method,timestamp) VALUES ('%s','%s','%s','%s',(select current_timestamp),'%s','%s','%s','%s','%s');",data->tables[HTTP_GET],conn->source,conn->orig_dest,conn->dest,header,requestedHost,requestedLocation,userAgent,method,conn->timestamp);

		
		}
		else {
		msg(MSG_DEBUG,"We have a server response log");
		char header[1000];
		char serverType[100];
		char contentType[20];
		char lastModified[40];
		char* ptrToHeader = header;
		char* ptrToBody = NULL; //this pointer contains the address of the body 
		char* tmpPtr = NULL; // used for miscellaneous purposes
                ptrToBody  = strstr(message, "\r\n\r\n") + 4; // skip the new lines/ carriage returns ; we have to add + 4 because of the 4 characters \r\n\r\n
		int bodyLength = strlen(ptrToBody);
		int completeLength = strlen(message);
		int headerLength = completeLength - bodyLength;

		// HEADER extractor
		strncpy(header,message,headerLength);
		*(ptrToHeader+headerLength+1) = '\0';
		msg(MSG_DEBUG,"Response Header: %s",header);
		
		
		// extract header fields
		extract_header_field(contentType,"Content-Type:",header);
		extract_header_field(serverType,"Server:",header);
		extract_header_field(lastModified,"Last-Modified:",header);


		// now check if the server response has content-type of kind text
		tmpPtr = strstr(contentType,"text");
		char bodyText[100];
		
		if (tmpPtr == NULL) {
			char* dummyMsg = "body contains no plain text data";
			strcpy(bodyText,dummyMsg);

			//  we have to copy the binary data into the response body field 
		}
		else {
			//  we got text data, thus we can save the response as a string into the body field
			char bodyText[bodyLength+1];
			strncpy(bodyText,ptrToBody,bodyLength);
			bodyText[bodyLength] = '\0';
			msg(MSG_DEBUG,"Body: '%s'",bodyText);
		}

		snprintf(statement,MAX_STATEMENT, "update %s set ResponseHeader = '%s' , ResponseBody = '%s' , ResponseLastModified = '%s' , ServerType = '%s' where timestamp = '%s';",data->tables[HTTP_GET],header,bodyText,lastModified,serverType,conn->timestamp);

		
		msg(MSG_DEBUG,"try to execute: \n %s",statement);
		}

 
		snprintf(statement,MAX_STATEMENT, "INSERT into %s (RequesterIP, DestinationIP, TrueDestinationIP, Header, Date) VALUES ('%s','%s','%s','%s',(select current_timestamp));",data->tables[HTTP_GET],conn->source,conn->orig_dest,conn->dest,message);
		msg(MSG_DEBUG,"INSERT into %s (RequesterIP, DestinationIP, TrueDestinationIP, Header, Date) VALUES ('%s','%s','%s','%s',(select current_timestamp));",data->tables[HTTP_GET],conn->source,conn->orig_dest,conn->dest,message);
*/		//snprintf(statement, MAX_STATEMENT, "INSERT into %s (domain, direction, content) values ('%s','%s', '%s');", data->tables[HTTP_GET], conn->orig_dest, tag, message);
	/*	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
		if (rc) {
			msg(MSG_ERROR, "Error performing '%s': %s", statement, err);
		}*/		
		break;
	case IRC:
		break;
	case DNS:
		//snprintf(statement, MAX_STATEMENT, "INSERT into %s (domain, original, returned) values ('%s', '%s', '%s');", data->tables[DNS], message, conn->orig_dest, conn->dest);
		//rc = sqlite3_exec(data->db, statement, callback, 0, &err);
		//if (rc) {
		//	msg(MSG_ERROR, "Error performing '%s': %s", statement, err);
		//	}		
		break;
	case UNKNOWN:
		
		break;
	}

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
			snprintf(statement, MAX_STATEMENT, "insert into HTTP_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,requestedhost,requestedlocation,useragent,method,requestheader,requestbodytext,date,TrumanTimestamp) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', '%s', '%s', '%s', '%s', '%s', (select current_timestamp),'%s')",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->requestedHost,logdata->requestedLocation,logdata->userAgent,logdata->method,logdata->requestHeader,logdata->requestBodyText,conn->timestamp
			);
			execute_statement(statement);

		}
		else {
			struct http_server_struct* logdata =  (struct http_server_struct *) data;
			snprintf(statement, MAX_STATEMENT, "update HTTP_LOGS set servertype = '%s', responsecontenttype = '%s', responselastmodified = '%s', responseheader = '%s', responsebodytext = '%s' where trumantimestamp = '%s'",
			logdata->serverType,logdata->responseContentType,logdata->responseLastModified,logdata->responseHeader,logdata->responseBodyText,conn->timestamp
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
	default:
		{
		msg(MSG_DEBUG, "Protocol not yet handled, abort...");
		}


 	} // end of switch
	return 0;
} // end of lpg_log_struct

