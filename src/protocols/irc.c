#include "irc.h"

#include "wrapper.h"
#include "logger.h"
#include "msg.h"

#include <stdlib.h>
#include <string.h>

struct ph_irc {
	struct configuration_t* config;
};

void* ph_irc_create()
{
	void* ret = malloc(sizeof(struct ph_irc));
	return ret;
}

int ph_irc_destroy(void* handler)
{
	free(handler);
	return 0;
}


int ph_irc_init(void* handler, struct configuration_t* c)
{
	struct ph_irc* irc = (struct ph_irc*)handler;
	irc->config = c;
	return 0;
}

int ph_irc_deinit(void* handler)
{
	return 0;
}

int ph_irc_handle_payload_stc(void* handler, connection_t* conn,  const char* payload, size_t* len)
{
        char msgCopy[MAXLINE];
	char* currentLinePtr = NULL;
	strcpy(msgCopy,payload);
	currentLinePtr = strtok (msgCopy,"\n");


	while(currentLinePtr != NULL) {
	// we have to iterate throughout the whole received IRC message (which may contain several lines)
		struct irc_server_struct* data = (struct irc_server_struct*) malloc(sizeof(struct irc_server_struct));
		
		// check if we have a server reply of the type [:host] [code] [nickname] [message]
		if (strncmp(currentLinePtr,":xxx",1) == 0 || strncmp(currentLinePtr,"\n:xxx",2) == 0) {
			// ok everything's fine
			
			if (strncmp(currentLinePtr,"\n:",2) == 0)
				currentLinePtr++; // we have a leading '\n' character, skip it


			char* codePtr = NULL; 
			int nameLength = 0;


			
			// first we extract the server name
			codePtr = strstr(currentLinePtr," ");
			if (codePtr == NULL)
			{
				msg(MSG_DEBUG,"Abort: %s",currentLinePtr);
				goto nextline;
			}

			codePtr ++;
			nameLength = strcspn(currentLinePtr," "); 
			strncpy(data->serverName,currentLinePtr,nameLength);
			data->serverName[nameLength] = '\0';


			// second, we extract the numeric raw event
			char* nickPtr  = NULL;
			nickPtr = strstr(codePtr," "); 
			if (nickPtr == NULL) {

				msg(MSG_DEBUG,"Aborted! %s",codePtr);	
				goto nextline;
			}
			nickPtr++;

			int codeLength = strcspn(codePtr," "); 
			 
			 // as specified in rfc1459 / 2.4 only numeric replies / codes with 3 digits are allowed
			if (codeLength != 3) {
				msg(MSG_DEBUG,"invalid numeric reply");
				goto nextline;
			}
			strncpy(data->numericReply,codePtr,codeLength);
			data->numericReply[codeLength] = '\0';



			// third, we extract the nickname of the recipient
			char* msgPtr = NULL;
			int nickLength = 0;
			msgPtr = strstr(nickPtr," "); 
			if (msgPtr == NULL) {
				msg(MSG_DEBUG,"Aborted! %s",nickPtr);	
				goto nextline;
			}
			msgPtr++;


			nickLength = strcspn(nickPtr," "); 
			char nick[nickLength+1]; // allocate array with sufficient space
			strncpy(data->recipientNickname,nickPtr,nickLength);
			nick[nickLength] = '\0';

	
			// finally, we extract the actual server message
			strcpy(data->message,msgPtr);

				
				

			} // end of if (valid server msg)	
			
			logger_get()->log_struct(logger_get(),conn,"server",data);

		nextline:
			currentLinePtr = strtok(NULL,"\r");
		}// end of while(finished the line)



	return logger_get()->log(logger_get(), conn, "server", payload);
}

int ph_irc_handle_payload_cts(void* handler, connection_t* conn,  const char* payload, size_t* len)
{
	char msgCopy[MAXLINE];
	char* currentLinePtr = NULL;
	strcpy(msgCopy,payload);
	currentLinePtr = strtok (msgCopy,"\n");


	while(currentLinePtr != NULL) {
			// we have to iterate throughout the whole received IRC message (which may contain several lines)
		struct irc_client_struct* data = (struct irc_client_struct*) malloc(sizeof(struct irc_client_struct));
		char* argPtr; // pointer to the argument(s) of the IRC command
		int commandNameLength = 0;

		commandNameLength = strcspn(currentLinePtr," "); 
		strncpy(data->command,currentLinePtr,commandNameLength);
		data->command[commandNameLength] = '\0';


		// now check if we got additional arguments for this client IRC command
	
		argPtr = strstr(currentLinePtr," "); // pointer to the first space character
		if (argPtr != NULL) {
			argPtr ++;
			int argsLength = 0;
			argsLength = strcspn(argPtr,"\r\n");
			strncpy(data->arguments,argPtr,argsLength);
			data->arguments[argsLength] = '\0';
		}
		else {
			strcpy(data->arguments,"N/A");
		}
		currentLinePtr = strtok(NULL,"\r");


		logger_get()->log_struct(logger_get(),conn,"client",data);	
			
	}// end of while (Finished the current line)

		

		return logger_get()->log(logger_get(), conn, "client", payload);
}

int ph_irc_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_irc_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_irc* irc = (struct ph_irc*)handler;
	if (conf_get_mode(irc->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(irc->config, "irc", "irc_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)6667);
	}
	return 0;
}

