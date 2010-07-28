#include "log_sqlite.h"
#include "configuration.h"
#include "msg.h"
#include "definitions.h"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sqlite3.h>

#define TABLE_NAME_LENGTH 100
#define MAX_STATEMENT 8000

static char statement[MAX_STATEMENT];

struct lsq_data {
	const char* db_file;
	sqlite3* db;
	char tables[UNKNOWN + 1][TABLE_NAME_LENGTH];
};


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  NotUsed=0;

  for(i=0; i<argc; i++){
    msg(MSG_DEBUG, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  return 0;
}


int create_db(struct lsq_data* data)
{
	char* err = 0;
	int rc;
	int i;

	for (i = 0; i <= UNKNOWN; ++i) {
		snprintf(statement, MAX_STATEMENT, "DROP TABLE %s;", data->tables[i]);
		rc = sqlite3_exec(data->db, statement, callback, 0, &err);
		//if (rc != SQLITE_OK) {
		//	msg(MSG_ERROR, "Cannot drop table %s: %s", data->tables[i], err);
		//	return -1;
		//}
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (domain TEXT, original TEXT, returned TEXT);", data->tables[DNS]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
        if (rc != SQLITE_OK) {
                msg(MSG_ERROR, "Error createing table currentdns: %s", err);
                return -1;
        }

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (RequestedHost TEXT, RequestedLocation TEXT, UserAgent TEXT, RequestHeader TEXT, RequestBody TEXT, RequesterIP TEXT, DestinationIP TEXT, TrueDestinationIP TEXT, Date TEXT, Method TEXT, ResponseHeader TEXT, ResponseBody TEXT, ResponseLastModified TEXT, ServerType TEXT, Timestamp TEXT);", data->tables[HTTP]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currenthttp: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (domain TEXT);", data->tables[FTP]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentftp: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (server TEXT, rctp TEXT);", data->tables[SMTP]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentsmtp: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE IRC_CLIENT_MSGS (ClientIP TEXT, ServerIP TEXT, RealServerIP TEXT, Command TEXT, Arguments TEXT, Date TEXT, Timestamp TEXT);");
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentirc: %s", err);
		return -1;
	}

	snprintf(statement, MAX_STATEMENT, "CREATE TABLE IRC_SERVER_MSGS (ClientIP TEXT, ServerIP TEXT, RealServerIP TEXT, ServerName TEXT, NumericReply TEXT, RecipientNickname TEXT, Message TEXT, Date TEXT, Timestamp TEXT);");
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentirc: %s", err);
		return -1;
	}


	snprintf(statement, MAX_STATEMENT, "CREATE TABLE %s (content TEXT);", data->tables[UNKNOWN]);
	rc = sqlite3_exec(data->db, statement, callback, 0, &err);
	if (rc != SQLITE_OK) {
		msg(MSG_ERROR, "Error createing table currentunknown: %s", err);
		return -1;
	}
	return 0;
}

int lsq_init(struct logger_t* logger)
{
	struct lsq_data* data = (struct lsq_data*)malloc(sizeof(struct lsq_data));
	struct stat buf;
	int new_db = 0;

	strncpy(data->tables[SMTP], "currentsmtp", TABLE_NAME_LENGTH);
	strncpy(data->tables[FTP], "currentftp", TABLE_NAME_LENGTH);
	strncpy(data->tables[FTP_anonym], "currentftp", TABLE_NAME_LENGTH);
	strncpy(data->tables[FTP_data], "currentftp", TABLE_NAME_LENGTH);
	strncpy(data->tables[HTTP], "currenthttp", TABLE_NAME_LENGTH);
	strncpy(data->tables[IRC], "currentirc", TABLE_NAME_LENGTH);
	strncpy(data->tables[DNS], "currentdns", TABLE_NAME_LENGTH);
	strncpy(data->tables[UNKNOWN], "currentunknown", TABLE_NAME_LENGTH);

	data->db_file = conf_get(logger->config, "logging", "db_file");
	if (!data->db_file) {
		msg(MSG_FATAL, "No db_file given in configuration file!");
		goto out1;
	}
	if (-1 == stat(data->db_file, &buf)) {
		if (errno == ENOENT) {
			msg(MSG_DEBUG, "Database %s does not exist. Will be created during startup!", data->db_file);
			new_db = 1;
		} else {
			msg(MSG_FATAL, "Error stating database %s: %s", data->db_file, strerror(errno));
			goto out1;
		}
	}

	if ( sqlite3_open(data->db_file, &data->db)) {
		msg(MSG_FATAL, "Cannot open sqlite database file %s: %s", data->db_file, sqlite3_errmsg(data->db));
		goto out2;
	}

	// TODO: create permanent tables
	//if (new_db) {
	//	create_db(data->db);
	//}

	logger->data = (void*) data;
	return 0;

out2:
	sqlite3_close(data->db);
out1:
	free(data);
	return -1;
}

int lsq_deinit(struct logger_t* logger)
{
	struct lsq_data* data = (struct lsq_data*)logger->data;
	sqlite3_close(data->db);
	free(data);
	logger->data = NULL;
	return 0;
}

int lsq_create_log(struct logger_t* logger)
{
	struct lsq_data* data = (struct lsq_data*)logger->data;
	return create_db(data);
}

int lsq_finish_log(struct logger_t* logger)
{
	//struct lsq_data* data = (struct lsq_data*)logger->data;
	return 0;
}

int lsq_log_text(struct logger_t* logger, connection_t* conn, const char* tag, const char* message)
{
	struct lsq_data* data = (struct lsq_data*)logger->data;
	char* err;
	int rc;

	// BIG FAT TODO: sql injection ....
	switch (conn->app_proto) {
	case SMTP:
		snprintf(statement, MAX_STATEMENT, "INSERT into %s (\"%s\" \"%s\");", data->tables[SMTP], conn->orig_dest, message);
		rc = sqlite3_exec(data->db, statement, callback, 0, &err);
		if (rc) {
			msg(MSG_ERROR, "Error performing '%s': %s", statement, err);
		}
		break;
	case FTP:
		break;
	case FTP_anonym:
		break;
	case FTP_data:

		break;
	case HTTP:
		if (strcmp(tag,"client") == 0) {
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

		snprintf(statement,MAX_STATEMENT, "INSERT into %s (RequesterIP, DestinationIP, TrueDestinationIP, RequestHeader, Date, RequestedHost, RequestedLocation, UserAgent, Method,timestamp) VALUES ('%s','%s','%s','%s',(select current_timestamp),'%s','%s','%s','%s','%s');",data->tables[HTTP],conn->source,conn->orig_dest,conn->dest,header,requestedHost,requestedLocation,userAgent,method,conn->timestamp);

		
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


		// now check if the server response has content-type of kind text/*
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

		snprintf(statement,MAX_STATEMENT, "update %s set ResponseHeader = '%s' , ResponseBody = '%s' , ResponseLastModified = '%s' , ServerType = '%s' where timestamp = '%s';",data->tables[HTTP],header,bodyText,lastModified,serverType,conn->timestamp);

		
		msg(MSG_DEBUG,"try to execute: \n %s",statement);
		}
/*
 
		snprintf(statement,MAX_STATEMENT, "INSERT into %s (RequesterIP, DestinationIP, TrueDestinationIP, Header, Date) VALUES ('%s','%s','%s','%s',(select current_timestamp));",data->tables[HTTP_GET],conn->source,conn->orig_dest,conn->dest,message);
		msg(MSG_DEBUG,"INSERT into %s (RequesterIP, DestinationIP, TrueDestinationIP, Header, Date) VALUES ('%s','%s','%s','%s',(select current_timestamp));",data->tables[HTTP_GET],conn->source,conn->orig_dest,conn->dest,message);
*/		//snprintf(statement, MAX_STATEMENT, "INSERT into %s (domain, direction, content) values ('%s','%s', '%s');", data->tables[HTTP_GET], conn->orig_dest, tag, message);
		rc = sqlite3_exec(data->db, statement, callback, 0, &err);
		if (rc) {
			msg(MSG_ERROR, "Error performing '%s': %s", statement, err);
		}		
		break;
	case IRC:
		{
		char msgCopy[MAXLINE];
		char* currentLinePtr = NULL;
		strcpy(msgCopy,message);
		currentLinePtr = strtok (msgCopy,"\n");


		if (strcmp(tag,"client") == 0) {

			msg(MSG_DEBUG,"We have an IRC client request log");
			

			while(currentLinePtr != NULL) {
			// we have to iterate throughout the whole received IRC message (which may contain several lines)
	
				char* argPtr; // pointer to the argument(s) of the IRC command
				int commandNameLength = 0;

				commandNameLength = strcspn(currentLinePtr," "); 
				char command[commandNameLength+1]; // allocate array with sufficient space
				strncpy(command,currentLinePtr,commandNameLength);
				command[commandNameLength] = '\0';


				// now check if we got additional arguments for this client IRC command
				
				argPtr = strstr(currentLinePtr," "); // pointer to the first space character
				char arguments[MAXLINE];
				if (argPtr != NULL) {
					argPtr ++;
					int argsLength = 0;
					argsLength = strcspn(argPtr,"\r\n");
					//char arguments[argsLength+1]; // allocate char array (string) with sufficient space
					strncpy(arguments,argPtr,argsLength);
					arguments[argsLength] = '\0';
				}
				else {
					//char arguments[20];
					strcpy(arguments,"N/A");
				}
				msg(MSG_DEBUG,"arguments extracted: '%s'",arguments);
				currentLinePtr = strtok(NULL,"\r");
				snprintf(statement, MAX_STATEMENT, "INSERT into IRC_CLIENT_MSGS (ClientIP, ServerIP, RealServerIP, Command, Arguments, Date, Timestamp) values ('%s', '%s', '%s','%s','%s',(select current_timestamp),'%s');", conn->source, conn->orig_dest,  conn->dest, command, arguments, conn->timestamp);
					
				rc = sqlite3_exec(data->db, statement, callback, 0, &err);
				if (rc) {
				msg(MSG_ERROR, "Error performing '%s': %s", statement, err);
				}
			
			}// end of while (Finished the current line)

		

		} // end of  if (client msg handling)
		else {

			msg(MSG_DEBUG,"We have an IRC server response log");
	
	
			while(currentLinePtr != NULL) {
			// we have to iterate throughout the whole received IRC message (which may contain several lines)
				
				// check if we have a server reply of the type [:host] [code] [nickname] [message]
				if (strncmp(currentLinePtr,":xxx",1) == 0 || strncmp(currentLinePtr,"\n:xxx",2) == 0) {
					// ok everything's fine
					
					if (strncmp(currentLinePtr,"\n:",2) == 0)
					 currentLinePtr++; // we have a leading '\n' character, skip it

					//char* tmpPtr = NULL;
					//int tmpLength = 0;

					char* codePtr = NULL; 
					int nameLength = 0;


					
					// first we extract the server name
					codePtr = strstr(currentLinePtr," ");
					if (codePtr == NULL)
						{
						msg(MSG_DEBUG,"Abort: %s",currentLinePtr);
						break;
						}

					codePtr ++;
					nameLength = strcspn(currentLinePtr," "); 
					char serverName[nameLength+1]; // allocate array with sufficient space
					strncpy(serverName,currentLinePtr,nameLength);
					serverName[nameLength] = '\0';


					// second, we extract the numeric raw event
					char* nickPtr  = NULL;
					nickPtr = strstr(codePtr," "); 
					if (nickPtr == NULL) {

						msg(MSG_DEBUG,"Aborted! %s",codePtr);	
						break;
					}
					nickPtr++;

					int codeLength = strcspn(codePtr," "); 
					 
					 // as specified in rfc1459 / 2.4 only numeric replies / codes with 3 digits are allowed
					if (codeLength != 3) {
						msg(MSG_DEBUG,"invalid numeric reply");
						break;
					}
					char code[codeLength+1]; // allocate array with sufficient space
					strncpy(code,codePtr,codeLength);
					code[codeLength] = '\0';


		
					// third, we extract the nickname of the recipient
					char* msgPtr = NULL;
					int nickLength = 0;
					msgPtr = strstr(nickPtr," "); 
					if (msgPtr == NULL) {
						msg(MSG_DEBUG,"Aborted! %s",nickPtr);	
						break;
					}
					msgPtr++;


					nickLength = strcspn(nickPtr," "); 
					char nick[nickLength+1]; // allocate array with sufficient space
					strncpy(nick,nickPtr,nickLength);
					nick[nickLength] = '\0';

			
					// finally, we extract the actual server message
					char msg[MAXLINE];
					strcpy(msg,msgPtr);

					msg(MSG_DEBUG,"check: '%s' and ptr: '%s'",msg,msgPtr);
					snprintf(statement, MAX_STATEMENT, "INSERT into IRC_SERVER_MSGS (ClientIP, ServerIP, RealServerIP, ServerName, NumericReply, RecipientNickname, Message, Date, Timestamp) values ('%s', '%s','%s', '%s','%s','%s','%s',(select current_timestamp),'%s');", conn->source, conn->orig_dest,  conn->dest, serverName, code, nick, msg, conn->timestamp);
					
					rc = sqlite3_exec(data->db, statement, callback, 0, &err);
						if (rc) {
							msg(MSG_ERROR, "Error performing '%s': %s", statement, err);
						}
					

				} // end of if (valid server msg)	
				currentLinePtr = strtok(NULL,"\r");
			}// end of while(finished the line)

	
		}	// end of else (finished server msg handling)
		// end of case IRC
		}
		break;
	case DNS:
		msg(MSG_DEBUG,"DNS logger entered");
		snprintf(statement, MAX_STATEMENT, "INSERT into %s (domain, original, returned) values ('%s', '%s', '%s');", data->tables[DNS], message, conn->orig_dest, conn->dest);
		rc = sqlite3_exec(data->db, statement, callback, 0, &err);
		if (rc) {
			msg(MSG_ERROR, "Error performing '%s': %s", statement, err);
		}

		break;

	case UNKNOWN:
	{	
		break;
	};
	case UNKNOWN_UDP:
	{
	break;
	}
	}
	return 0;
}


// this method searches for the 'headername' string in the 'header' string and if the search was successful, it tries to save the value for this headername into the string 'destination'
void extract_header_field(char* destination, char* headername, char* header) {

char* ptrToHeaderField = NULL;
int nameLength = strlen(headername)+1; // we have to add +1 because typically http requests look like this: 'HeaderField: Value' - thus we have to add +1 because of the space character
int valueLength = 0;
ptrToHeaderField = strstr(header,headername);

if (ptrToHeaderField == NULL) {
	//  the headername was not found - but we put anyway a value into the destination string
	strcpy(destination,"N/A");
}
else {
	// headername was found, now get its value and save it into the destination string
	ptrToHeaderField = ptrToHeaderField + nameLength;
	valueLength  = strcspn(ptrToHeaderField,"\r\n"); // go to the end of the line
	strncpy(destination,ptrToHeaderField,valueLength);
	destination[valueLength] = '\0'; // null termination character for end of the string
	msg(MSG_DEBUG,"extracted %s '%s'",headername,destination);
}




}

int lsq_log_struct(struct logger_t* log, connection_t* conn, const char* tag, void* data)
{
	msg(MSG_FATAL, "Das hier wird aufgerufen!");
	return 0;
}

