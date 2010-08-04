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
#include "helper_file.h"

#define MAX_STATEMENT 8000

static char statement[MAX_STATEMENT];


// Returns: Whether connection is successful AND if tables are already present
int lpg_init(struct logger_t* logger)
{
	// TODO:
	// Check if tables are already created
	return 1;	
	

}


// returns:
int lpg_deinit(struct logger_t* logger)
{
	return 1;
}


// returns: 1 if creation was successful, 0 if not
int lpg_create_log(struct logger_t* logger)
{
	const char* testmode = conf_get(logger->config, "logging", "testmode");	
	if (strcmp(testmode,"0") == 0) {
                char update_trumanbox_runtime_id[1000] = "update trumanbox_settings set value = value+1 where key = 'SAMPLE_COUNTER'";
                execute_statement(update_trumanbox_runtime_id);
                char set_current_id[1000] = "update trumanbox_settings set value = (select t.value from trumanbox_settings t  where t.key = 'SAMPLE_COUNTER') where key = 'CURRENT_SAMPLE'";
                execute_statement(set_current_id);
	}
	else {
                char set_current_id[1000] = "update trumanbox_settings set value = -1 where key = 'CURRENT_SAMPLE'";
                execute_statement(set_current_id);
	}
	return 1; // by default, all tables are already created
}

// returns: 1 if everything was fine, 0 if error
int lpg_finish_log(struct logger_t* logger)
{
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
		struct smtp_struct* logdata =  (struct smtp_struct *) data;
		if (strcmp(tag,"client") == 0) {
			snprintf(statement, MAX_STATEMENT, "insert into SMTP_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,Message,Type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s','client', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->Message,conn->timestamp
			);
			execute_statement(statement);

		}
		else {
			snprintf(statement, MAX_STATEMENT, "insert into SMTP_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,Message,Type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', 'server', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->Message,conn->timestamp
			);
			execute_statement(statement);


		}
	}
	break;

	case HTTP:
	{	
		if (strcmp(tag,"client") == 0) {
			struct http_client_struct* logdata =  (struct http_client_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into HTTP_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,requestedhost,requestedlocation,useragent,method,requestheader,requestbodybinarylocation,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', '%s', '%s',  '%s', '%s','%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
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
			snprintf(statement, MAX_STATEMENT, "insert into IRC_CLIENT_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,Command,Arguments,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', '%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->command,logdata->arguments,conn->timestamp
			);
			execute_statement(statement);

		}
		else {
			struct irc_server_struct* logdata =  (struct irc_server_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into IRC_SERVER_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,ServerName,NumericReply,RecipientNickname,Message,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s','%s','%s','%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
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
		struct unknown_struct* logdata =  (struct unknown_struct *) data;

		if (strcmp(tag,"client") == 0) {
			snprintf(statement, MAX_STATEMENT, "insert into UNKNOWN_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort, binaryLocation, type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, '%s', 'client', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->binaryLocation,conn->timestamp
			);
			execute_statement(statement);

		}
		else {
			snprintf(statement, MAX_STATEMENT, "insert into UNKNOWN_LOGS (ClientIP,ClientPort,ServerIP,RealServerIP,ServerPort,binaryLocation,type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d,'%s', (select current_timestamp),'%s', 'server', (select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->dport,logdata->binaryLocation,conn->timestamp
			);
			execute_statement(statement);

	
		}
	 	break;	
	}

	case DNS:
	{
			struct dns_struct* logdata =  (struct dns_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into DNS_LOGS (clientIP,ServerIP,RealServerIP,DomainName,date,trumantimestamp,sample_id) Values (inet('%s'),inet('%s'),inet('%s'),'%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			logdata->clientIP,logdata->realServerIP,logdata->serverIP,logdata->domain,conn->timestamp
			);
			execute_statement(statement);

	break;
	}
	default:
		{
		msg(MSG_DEBUG, "Protocol not yet handled, abort...");
		}


 	} // end of switch
	return 0;
} // end of lpg_log_struct

