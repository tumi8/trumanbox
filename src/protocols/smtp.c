#include "smtp.h"
#include "wrapper.h"
#include "logger.h"
#include "msg.h"

#include <stdlib.h>
#include <string.h>


struct ph_smtp {
	struct configuration_t* config;
};

void* ph_smtp_create()
{
	void* ret = malloc(sizeof(struct ph_smtp));
	return ret;
}

int ph_smtp_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_smtp_init(void* handler, struct configuration_t* c)
{
	struct ph_smtp* smtp = (struct ph_smtp*)handler;
	smtp->config = c;
	return 0;
}

int ph_smtp_deinit(void* handler)
{
	return 0;
}

int ph_smtp_handle_payload_stc(void* handler, connection_t* conn, const char* payload, size_t* len)
{
	char msgCopy[MAXLINE];
	char* linePtr = NULL;
	strcpy(msgCopy,payload);
	linePtr = strtok (msgCopy,"\n");

	while (linePtr!= NULL) {
		// parse all response lines from server
		struct smtp_server_struct* data = (struct smtp_server_struct*) malloc(sizeof(struct smtp_server_struct));
	
		//extract status code (3 numbers)
		strncpy(data->statusCode,linePtr,3);
		data->statusCode[3] = '\0';
		linePtr = linePtr + 4;

		//extract server message (arbitrary length possible)
		int Msglength = strcspn(linePtr,"\r\n");
		strncpy(data->serverMsg,linePtr,Msglength);
		msg(MSG_DEBUG,"we have code '%s':'%s'",data->statusCode,data->serverMsg);
		linePtr = strtok(NULL,"\n");
		logger_get()->log_struct(logger_get(), conn, "server", data);
	}

	return logger_get()->log(logger_get(), conn, "content-stc", payload);
}

int ph_smtp_handle_payload_cts(void* handler, connection_t* conn, const char* payload, size_t* len)
{
	struct smtp_client_struct* data = (struct smtp_client_struct*) malloc(sizeof(struct smtp_client_struct));
	
	strncpy(data->clientMsg,payload,strlen(payload)); 
	int lengthMsg= strlen(data->clientMsg);
	data->clientMsg[lengthMsg-2] = '\0'; // we want to discard the last two characters : \r\n

	// change payload in order to avoid spam attacks
	char* ptr;
	if (strncasecmp(payload, "rcpt to:", 8) == 0) {
		ptr = strchr(payload, ':');
		ptr++;
		sprintf(ptr, LOCAL_EMAIL_ADDRESS);
		msg(MSG_DEBUG, "changed payload from client:%s", payload);
			*len = strlen(payload);
	}
	logger_get()->log_struct(logger_get(), conn, "client", data);
	return logger_get()->log(logger_get(), conn, "content-stc", payload);
}

int ph_smtp_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_smtp_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_smtp* smtp = (struct ph_smtp*)handler;
	if (conf_get_mode(smtp->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(smtp->config, "smtp", "smtp_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)25);
	}
	return 0;
}

