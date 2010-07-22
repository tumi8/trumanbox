#include "http.h"
#include "wrapper.h"
#include "logger.h"
#include "helper_file.h"
#include "msg.h"
#include <stdlib.h>
#include <string.h>

struct ph_http {
	struct configuration_t* config;
};

void* ph_http_create()
{
	void* ret = malloc(sizeof(struct ph_http));
	return ret;
}

int ph_http_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_http_init(void* handler, struct configuration_t* c)
{
	struct ph_http* http = (struct ph_http*)handler;
	http->config = c;
	return 0;
}

int ph_http_deinit(void* handler)
{
	return 0;
}

int ph_http_handle_payload_stc(void* handler, connection_t* conn, const char* payload, size_t* len)
{

	char* ptrToHeaderEnd = strstr(payload,"\r\n\r\n"); // every proper HTTP header ends with this string


	if (ptrToHeaderEnd != NULL) 
	{
		struct http_server_struct* data = (struct http_server_struct*) malloc(sizeof(struct http_server_struct));
		char* ptrToHeader = data->responseHeader;
		ptrToHeaderEnd = ptrToHeaderEnd + 4; // skip the new lines/ carriage returns ; we have to add + 4 because of the 4 characters \r\n\r\n

		int headerLength = ptrToHeaderEnd - payload;
		int bodyLength = *len - headerLength; // -4 because we have \r\n\r\n after the header

		// HEADER extractor
		strncpy(data->responseHeader,payload,headerLength);
		*(ptrToHeader+headerLength+1) = '\0';
		
		
		// extract header fields
		extract_http_header_field(data->responseContentType,"Content-Type:",data->responseHeader);
		extract_http_header_field(data->serverType,"Server:",data->responseHeader);
		extract_http_header_field(data->responseLastModified,"Last-Modified:",data->responseHeader);

			
		// CONTENT-LENGTH extractor
		char contentLengthStr[100];
		extract_http_header_field(contentLengthStr,"Content-Length:",data->responseHeader);
		int contentLength = atoi(contentLengthStr);
		if (contentLength != 0) {
			//parse was successful
			conn->rcvd_content_length = contentLength;
			conn->rcvd_content_done = bodyLength;
			conn->rcvd_content_done_ptr = (char*) malloc (contentLength);
			if (conn->rcvd_content_done_ptr != NULL) {
				memcpy(conn->rcvd_content_done_ptr,ptrToHeaderEnd,bodyLength);
				msg(MSG_DEBUG,"wrote %d bytes to temporary http chunk collection (total: %d) and (total: %d)",bodyLength,contentLength,conn->rcvd_content_length);
			}
		
		}

		
		
		logger_get()->log_struct(logger_get(), conn, "server", data);
	}

	else if ((conn->rcvd_content_done+*len) < conn->rcvd_content_length) {
		
		msg(MSG_DEBUG,"we still have to collect some data...!");

		if (conn->rcvd_content_done_ptr != NULL) {
			memcpy(conn->rcvd_content_done_ptr+conn->rcvd_content_done,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to temporary http chunk collection",*len);
		}
		conn->rcvd_content_done += *len;

		msg(MSG_DEBUG,"added: %d and now we have: %d",*len,conn->rcvd_content_done);

	}
	else if ((conn->rcvd_content_done+*len) == conn->rcvd_content_length && conn->rcvd_content_length != 0) {
		msg(MSG_DEBUG,"we are finished reading the body!");
	
		if (conn->rcvd_content_done_ptr != NULL) {
			memcpy(conn->rcvd_content_done_ptr+conn->rcvd_content_done,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to temporary http chunk collection",*len);
		}
		conn->rcvd_content_done += *len;
		
		char location[200];
		save_binarydata_to_file(location,"http/rcvd",conn->rcvd_content_done_ptr,conn->rcvd_content_length);	
		free(conn->rcvd_content_done_ptr);
	}
	else {
		msg(MSG_DEBUG,"done: %d, total: %d, lenrcvd: %d",conn->rcvd_content_done,conn->rcvd_content_length,*len);
		msg(MSG_DEBUG,"Malformed http request! Nothing to do");
	}


	return 1;

}


int ph_http_handle_payload_cts(void* handler, connection_t* conn, const char* payload, size_t* len)
{
	
	
	// we perform a log operation for a client request
	// try to extract host, url, user-agent

	char* ptrToHeaderEnd = strstr(payload,"\r\n\r\n"); // every proper HTTP header ends with this string

	
	if (ptrToHeaderEnd != NULL) {		
		struct http_client_struct* data = (struct http_client_struct*) malloc(sizeof(struct http_client_struct));
		char* ptrToHeader = data->requestHeader;
		char* ptrToRequestedLocation = NULL;

		ptrToHeaderEnd = ptrToHeaderEnd + 4; // skip the new lines/ carriage returns ; we have to add + 4 because of the 4 characters \r\n\r\n
		
		int headerLength = ptrToHeaderEnd - payload;
		int bodyLength = *len - headerLength; // -4 because we have \r\n\r\n after the header
		


		// HEADER extractor
		strncpy(data->requestHeader,payload,headerLength);
		*(ptrToHeader+headerLength+1) = '\0';

		// BODY extractor
		strncpy(data->requestBodyText,ptrToHeaderEnd,bodyLength);


		// METHOD extractor
		int methodLength = strcspn(data->requestHeader," ");
		strncpy(data->method,data->requestHeader,methodLength);
		data->method[methodLength] = '\0';

		// LOCATION extractor 
		
		ptrToRequestedLocation = strstr(data->requestHeader,"/");
		int locationLength = strcspn(ptrToRequestedLocation," ");
		strncpy(data->requestedLocation,ptrToRequestedLocation,locationLength);
		data->requestedLocation[locationLength] = '\0';
		
		// CONTENT-LENGTH extractor
		char contentLengthStr[100];
		extract_http_header_field(contentLengthStr,"Content-Length:",data->requestHeader);
		int contentLength = atoi(contentLengthStr);
		if (contentLength != 0) {
			//parse was successful
			conn->sent_content_length = contentLength;
			conn->sent_content_done = bodyLength;
			conn->sent_content_done_ptr = (char*) malloc (contentLength);
			if (conn->sent_content_done_ptr != NULL) {
				memcpy(conn->sent_content_done_ptr,ptrToHeaderEnd,bodyLength);
				msg(MSG_DEBUG,"wrote %d bytes to temporary http chunk collection",bodyLength);
			}
		
		}

		extract_http_header_field(data->requestedHost,"Host:",data->requestHeader);
		extract_http_header_field(data->userAgent,"User-Agent:",data->requestHeader);
		
		msg(MSG_DEBUG,"length comparison: complete: %d header: %d body: %d",*len,headerLength, bodyLength);
	
		build_tree(conn, payload);
		logger_get()->log_struct(logger_get(), conn, "client", data);
	}
	else if ((conn->sent_content_done+*len) < conn->sent_content_length) {
		
		msg(MSG_DEBUG,"we still have to collect some data...!");

		if (conn->sent_content_done_ptr != NULL) {
			memcpy(conn->sent_content_done_ptr+conn->sent_content_done,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to temporary http chunk collection",*len);
		}
		conn->sent_content_done += *len;

		msg(MSG_DEBUG,"added: %d and now we have: %d",*len,conn->sent_content_done);

	}
	else if ((conn->sent_content_done+*len) == conn->sent_content_length && conn->sent_content_length != 0) {
		msg(MSG_DEBUG,"we are finished reading the body!");
	
		if (conn->sent_content_done_ptr != NULL) {
			memcpy(conn->sent_content_done_ptr+conn->sent_content_done,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to temporary http chunk collection",*len);
		}
		conn->sent_content_done += *len;
		
		char location[200];
		save_binarydata_to_file(location,"http/sent",conn->sent_content_done_ptr,conn->sent_content_length);	
		free(conn->sent_content_done_ptr);
	}
	else {
		msg(MSG_DEBUG,"Malformed http request! Nothing to do");
	}


	return 1;
}

int ph_http_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_http_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_http* http = (struct ph_http*)handler;
	if (conf_get_mode(http->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(http->config, "http", "http_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)80);
	}
	return 0;
}

// this method searches for the 'headername' string in the 'header' string and if the search was successful, it tries to save the value for this headername into the string 'destination'
void extract_http_header_field(char* destination, char* headername, char* header) {

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
	}




}

