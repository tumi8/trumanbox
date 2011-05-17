#include "smtp.h"
#include "wrapper.h"
#include <stdlib.h>
#include <string.h>

#include <logging/logbase.h>
#include <common/msg.h>
#include <common/configuration.h>

SMTPHandler::SMTPHandler(const Configuration& config)
	: ProtoHandler(config)
{

}

int SMTPHandler::payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len)
{
	char msgCopy[MAXLINE];
	char* linePtr = NULL;
	strcpy(msgCopy,payload);
	linePtr = strtok (msgCopy,"\n");

	while (linePtr!= NULL) {
		// parse all response lines from server
		struct smtp_struct* data = (struct smtp_struct*) malloc(sizeof(struct smtp_struct));
	
	/*
		//extract status code (3 numbers)
		strncpy(data->statusCode,linePtr,3);
		data->statusCode[3] = '\0';
		linePtr = linePtr + 4;
	*/
		//extract server message (arbitrary length possible)
		int Msglength = strcspn(linePtr,"\r\n");
		strncpy(data->Message,linePtr,Msglength);
		linePtr = strtok(NULL,"\n");
		logger_get()->logStruct(conn, "server", data);
	}

	return 1;
}

int SMTPHandler::payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len)
{
	struct smtp_struct* data = (struct smtp_struct*) malloc(sizeof(struct smtp_struct));
	
	strncpy(data->Message,payload,strlen(payload)); 
	int lengthMsg= strlen(data->Message);
	data->Message[lengthMsg-2] = '\0'; // we want to discard the last two characters : \r\n

	// change payload in order to avoid spam attacks
	char* ptr;
	if (strncasecmp(payload, "rcpt to:", 8) == 0) {
		ptr = strchr(payload, ':');
		ptr++;
		bzero(ptr,strlen(ptr));
		sprintf(ptr, LOCAL_EMAIL_ADDRESS);
		msg(MSG_DEBUG, "changed payload from client:%s", payload);
			*len = strlen(payload);
	}
	
	logger_get()->logStruct(conn, "client", data);
	return 1;
}

int SMTPHandler::determineTarget(struct sockaddr_in* addr)
{
	if (config.getMode() < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, config.get("smtp", "smtp_redirect").c_str(), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)25);
	}
	return 0;
}

