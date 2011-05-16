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
#include <postgresql/libpq-fe.h>


#define MAX_STATEMENT 8000

static char statement[MAX_STATEMENT];


int execute_nonquery_statement(char* stmt) {
 
	int result = 0;
	PGconn* psql  = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'trumanlogs' user = 'trumanbox' password = 'das$)13x!#+23' connect_timeout = '10'");
	if (!psql) {
                msg(MSG_FATAL,"libpq error : PQconnectdb returned NULL.\n\n");
                return result;
        }

        if (PQstatus(psql) != CONNECTION_OK) {
                msg(MSG_FATAL,"libpq error: PQstatus(psql) != CONNECTION_OK\n\n");
                return result;
        }
	
        PGresult *res = PQexec(psql, stmt);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
                {
                msg(MSG_FATAL,"ERROR: %s",PQresultErrorMessage(res));
                msg(MSG_FATAL,"while executing '%s'",stmt);
		result = 0;
                }
                else
                {
                result = 1; // success
      	}
	
	PQfinish(psql);
	return result;
}


int execute_query_statement_singlevalue(char* dst, char* stmt) {
	int result = 0;
       	PGconn* psql = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'trumanlogs' user = 'trumanbox' password = 'das$)13x!#+23' connect_timeout = '10'");
	if (!psql) {
                msg(MSG_FATAL,"libpq error : PQconnectdb returned NULL.\n\n");
                return result;
        }

        if (PQstatus(psql) != CONNECTION_OK) {
                msg(MSG_FATAL,"libpq error: PQstatus(psql) != CONNECTION_OK\n\n");
                return result;
        }

	PGresult *res = PQexec(psql, stmt);
                
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		{
		msg(MSG_FATAL,"Status: %s",PQresStatus(PQresultStatus(res)));
		msg(MSG_FATAL,"%s",PQresultErrorMessage(res));
		}
	else {
		if (PQntuples(res) > 0 && PQnfields(res) > 0) {
		result = 1;
		char* value =  PQgetvalue(res,0,0);
		msg(MSG_DEBUG,"we have value: %s",value);
		strcpy(dst,value);
		}
		
	 }

	
	PQfinish(psql);


	return result;
}

int execute_statement(char* stmt) {
	return execute_nonquery_statement(stmt);
}


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
                char update_trumanbox_runtime_id[1000] = "update trumanbox_settings set value = value+1 where key = 'CURRENT_SAMPLE'";
                if (!execute_statement(update_trumanbox_runtime_id))
                	return 0;
		//char set_current_id[1000] = "update trumanbox_settings set value = (select t.value from trumanbox_settings t  where t.key = 'SAMPLE_COUNTER') where key = 'CURRENT_SAMPLE'";
                //if (!execute_statement(set_current_id))
		//	return 0;
		char new_malware_dataset[1000] = "insert into malwaresamples (id,beginlogging) values ((select t.value from trumanbox_settings t where t.key = 'CURRENT_SAMPLE'), (select current_timestamp))";
		if (!execute_statement(new_malware_dataset))
			return 0;
	}
	else {
                char set_current_id[1000] = "update trumanbox_settings set value = -1 where key = 'CURRENT_SAMPLE'";
                if(!execute_statement(set_current_id))
			return 0;
	}
	return 1; // by default, all tables are already created
}

// returns: 1 if everything was fine, 0 if error
int lpg_finish_log(struct logger_t* logger)
{
	// set timestamp of logging end
	char set_ending_timestamp[1000] = "update malwaresamples set endlogging = (select current_timestamp) where id = (select t.value from trumanbox_settings t where t. key = 'CURRENT_SAMPLE')";
	if (!execute_statement(set_ending_timestamp))
		return 0;
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
		
		snprintf(statement, MAX_STATEMENT, "insert into SMTP_LOGS (ClientIP,ClientPort,ServerIP,orig_ServerIP,ServerPort,orig_serverport,Message,Type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, %d,'%s', '%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
		conn->source,conn->sport,conn->dest,conn->orig_dest,conn->dport,conn->orig_dport,logdata->Message,tag,conn->timestamp
		);
		execute_statement(statement);


		break;
	}
	case FTP:
	{
		struct ftp_struct* logdata =  (struct ftp_struct *) data;
		
		snprintf(statement, MAX_STATEMENT, "insert into FTP_LOGS (ClientIP,ClientPort,ServerIP,orig_ServerIP,ServerPort,orig_serverport,Message,type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d,%d,'%s','%s', (select current_timestamp),'%s', (select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
		conn->source,conn->sport,conn->dest,conn->orig_dest,conn->dport,conn->orig_dport,logdata->Message,tag,conn->timestamp
		);

		execute_statement(statement);
	 	break;	
	}
	case HTTP:
	{	
		if (strcmp(tag,"client") == 0) {
			struct http_client_struct* logdata =  (struct http_client_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into HTTP_LOGS (ClientIP,ClientPort,orig_ServerIP,ServerIP,orig_serverport,ServerPort,requestedhost,requestedlocation,useragent,method,requestheader,requestbodybinarylocation,responsereturnedtype,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d,%d, '%s', '%s', '%s',  '%s', '%s', '%s','%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,logdata->requestedHost,logdata->requestedLocation,logdata->userAgent,logdata->method,logdata->requestHeader,logdata->requestBodyBinaryLocation,logdata->responseReturnedType,conn->timestamp
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
			snprintf(statement, MAX_STATEMENT, "insert into IRC_CLIENT_LOGS (ClientIP,ClientPort,orig_ServerIP,ServerIP,orig_serverport,ServerPort,Command,Arguments,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d, %d,'%s', '%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,logdata->command,logdata->arguments,conn->timestamp
			);
			execute_statement(statement);

		}
		else if (strcmp(tag,"logfile") == 0) {
			char location[MAX_PATH_LENGTH];
			snprintf(location,MAX_PATH_LENGTH,"irc/%s",conn->timestamp);
			snprintf(statement, MAX_STATEMENT, "insert into IRC_LOGS (ClientIP,ClientPort,orig_ServerIP,ServerIP,orig_serverport,ServerPort,logfilelocation,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d,%d, '%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,location,conn->timestamp
			);
			execute_statement(statement);
	
		}
		else {
			struct irc_server_struct* logdata =  (struct irc_server_struct *) data;
			snprintf(statement, MAX_STATEMENT, "insert into IRC_SERVER_LOGS (ClientIP,ClientPort,orig_ServerIP,ServerIP,orig_serverport,ServerPort,ServerName,NumericReply,RecipientNickname,Message,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d,%d, '%s','%s','%s','%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
			conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,logdata->serverName,logdata->numericReply,logdata->recipientNickname,logdata->message,conn->timestamp
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
		struct unknown_struct* logdata =  (struct unknown_struct *) data;

		snprintf(statement, MAX_STATEMENT, "insert into UNKNOWN_LOGS (ClientIP,ClientPort,orig_ServerIP,ServerIP,orig_serverport,ServerPort,binaryLocation,type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d,%d,'%s','%s', (select current_timestamp),'%s', (select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
		conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,logdata->binaryLocation,tag,conn->timestamp
		);

		msg(MSG_DEBUG,"try to execute: %s",statement);
		execute_statement(statement);
	 	break;	
	}

	case DNS:
	{
		struct dns_struct* logdata =  (struct dns_struct *) data;
		snprintf(statement, MAX_STATEMENT, "insert into DNS_LOGS (clientIP,orig_ServerIP,ServerIP,DomainName,date,trumantimestamp,sample_id) Values (inet('%s'),inet('%s'),inet('%s'),'%s', (select current_timestamp),'%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
		logdata->clientIP,logdata->serverIP,logdata->realServerIP,logdata->domain,conn->timestamp
		);
		execute_statement(statement);

		break;
	}
	case FTP_data:
	{
		struct ftp_data_struct* logdata = (struct ftp_data_struct *) data;
		snprintf(statement, MAX_STATEMENT, "insert into FTP_Passive_Logs (clientIP,clientport,orig_ServerIP,ServerIP,orig_serverport,Serverport,binarylocation, type, filename, date,Trumantimestamp,sample_id) VALUES \
		(inet('%s'),%d,inet('%s'),inet('%s'),%d,%d,'%s', \
		(select case when type = '' then null else type end from awaited_pasv where serverport = %d and serverIP = inet('%s')), \
		(select case when filename = '' then null else filename end from awaited_pasv where serverport = %d and serverIP = inet('%s')), (select current_timestamp), '%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
		conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,logdata->binaryLocation,conn->dport,conn->dest,conn->dport,conn->dest,conn->timestamp
		);
		msg(MSG_DEBUG,"try to insert: %s",statement);
		execute_statement(statement);
		snprintf(statement, MAX_STATEMENT, "delete from awaited_pasv where serverport = %d and serverip = inet('%s')",
		conn->dport,conn->dest);
		execute_statement(statement);	
		break;
	}
	case SSL_Proto:
	{
		struct ssl_struct* logdata = (struct ssl_struct *) data;
		snprintf(statement, MAX_STATEMENT, "insert into SSL_Logs (clientIP,clientport,orig_ServerIP,ServerIP,orig_serverport,Serverport, Client_Hello_SSL_Version, server_certificate_location, http_request_location,date, Trumantimestamp,sample_id) VALUES (inet('%s'),%d,inet('%s'),inet('%s'),%d,%d,'%s', '%s', '%s', (select current_timestamp), '%s',(select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
		conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,logdata->sslVersion,logdata->server_cert,logdata->http_request,conn->timestamp
		);
		msg(MSG_DEBUG,"try to insert: %s",statement);
		execute_statement(statement);
		break;
	}
	case UNKNOWN_UDP:
	{
		struct unknown_struct* logdata =  (struct unknown_struct *) data;

		snprintf(statement, MAX_STATEMENT, "insert into UNKNOWN_UDP_LOGS (ClientIP,ClientPort,orig_ServerIP,ServerIP,orig_serverport,ServerPort,binaryLocation,type,date,TrumanTimestamp,sample_id) Values (inet('%s'),%d,inet('%s'),inet('%s'),%d,%d,'%s','%s', (select current_timestamp),'%s', (select distinct value from trumanbox_settings where key = 'CURRENT_SAMPLE'))",
		conn->source,conn->sport,conn->orig_dest,conn->dest,conn->orig_dport,conn->dport,logdata->binaryLocation,tag,conn->timestamp
		);
		msg(MSG_DEBUG,"try to execute: %s",statement);
		execute_statement(statement);
	 	break;	
	}default:
		{
		msg(MSG_DEBUG, "Protocol not yet handled, abort...");
		}


 	} // end of switch
	return 0;
} // end of lpg_log_struct

