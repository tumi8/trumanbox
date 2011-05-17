#include "http.h"
#include "wrapper.h"
#include "helper_file.h"
#include <stdlib.h>
#include <string.h>

#include <logging/logbase.h>
#include <common/msg.h>

 void HTTPHandler::manipulate_server_payload(const char* payload, connection_t* conn, ssize_t* len) {
	// change the server type if possible
	char* ptr = strstr(payload, "Server: ");
	if (ptr != 0 && conn->timestampEmulation != NULL) {
		char statement[1000];
		char serverType[100];
		snprintf(statement,1000,"select serverType from HTTP_LOGS where trumantimestamp = '%s'",conn->timestampEmulation); // get the server response we once already received
		execute_query_statement_singlevalue(serverType,statement);
		
		if (serverType != NULL) {
			ptr = ptr + 8;
			int valueLength  = strcspn(ptr,"\r\n"); // go to the end of the line
			
			// we have to save the payload following the serverType information
			char* ptrUnaffected = ptr + valueLength;
			int lenAffected = ptrUnaffected - payload;
			int lenUnaffected = *len - lenAffected;
			char saved[MAXLINE*2];
			strncpy(saved,ptrUnaffected,lenUnaffected);
			msg(MSG_DEBUG,"saved {%s}",saved);

			// now set the new serverType
			int newServerTypeLen = strlen(serverType);
			memcpy(ptr,serverType,newServerTypeLen);
		
			// now concatenate the unaffected payload to the modified payload
			ptr = ptr + newServerTypeLen;
			bzero(ptr,strlen(ptr));
			memcpy(ptr,saved,lenUnaffected+1);

			// set the new length of the payload
			*len = strlen(payload);
			msg(MSG_DEBUG,"new payload: [%s] len: %d",payload,*len);
			
		}
	}


}

void HTTPHandler::emulate_server_response(struct http_client_struct* data,connection_t* conn) {
	
	strcpy(data->responseReturnedType,"old");
	msg(MSG_DEBUG,"ok we got all we need for the request: %s || %s || %s",data->requestedHost,data->requestedLocation,data->userAgent);
	char statement[1000],trumantimestamp[100];
	snprintf(statement,1000,"select max(trumantimestamp) from HTTP_LOGS where (requestedHost = '%s' or ServerIP = inet('%s')) and requestedLocation = '%s' and responseReturnedType = 'new'",
		data->requestedHost,conn->orig_dest,data->requestedLocation);
	msg(MSG_DEBUG,"execute: %s",statement);
	execute_query_statement_singlevalue(trumantimestamp,statement);

	char destination[MAX_PATH_LENGTH],path[MAX_PATH_LENGTH],filename[MAX_PATH_LENGTH],location[MAX_PATH_LENGTH];
	strcpy(location,data->requestedLocation);
	char src[MAX_PATH_LENGTH]; // location of the server response
	extract_dir_and_filename_from_request(path, filename, location);
	
	// build path on webserver
	build_tree(conn,path);

	bzero(destination,MAX_PATH_LENGTH);
	if (filename == NULL || strlen(filename) == 0) {
		msg(MSG_DEBUG,"filename not given, set to index.html");
		strcpy(filename,"index.html");
	}

	strncpy(destination, HTTP_BASE_DIR, sizeof(destination)-1);	
	strcat(destination,path);
	strcat(destination,filename);



	if (trumantimestamp != NULL) {
		// we found an old client request that is is similiar/identical to the one just received
			
		strcpy(conn->timestampEmulation,trumantimestamp); // save the timestamp for future purposes
		snprintf(statement,1000,"select ResponseBodyBinaryLocation from HTTP_LOGS where trumantimestamp = '%s'",conn->timestampEmulation); // get the server response we once already received

		execute_query_statement_singlevalue(src,statement);	

		msg(MSG_DEBUG,"Server body binary location: %s",src);

	}

	if (src == NULL || strlen(src) ==0) {
			// we have no old server response we can replay to the client, thus we just send a dummy reply
			strcpy(data->responseReturnedType,"dummy");
			strncpy(src, HTTP_BASE_DIR,sizeof(src)-1);
			strcat(src,"/dummy.html");

	}

	copy(src,destination);



}


HTTPHandler::HTTPHandler(const Configuration& config)
	: ProtoHandler(config)
{

}

int HTTPHandler::payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len)
{
	struct http_server_struct* data;
	msg(MSG_DEBUG,"Received %d and multplechunks is: %d",*len,conn->multiple_server_chunks);
	if (conn->multiple_server_chunks == 0) {
			
		if (conn->destOffline) {
			// we now manipulate the server payload 
			manipulate_server_payload(payload,conn,len);
		}
		
		msg(MSG_DEBUG,"Payload \n%s",payload);
		char* ptrToHeaderEnd = strstr(payload,"\r\n\r\n"); // every proper HTTP header ends with this string
		if (ptrToHeaderEnd == NULL) {
			msg(MSG_DEBUG,"header not ended");
			return 1;
		}
		data = (struct http_server_struct*) malloc(sizeof(struct http_server_struct));
		conn->log_server_struct_ptr = data;

		// we have now to log the header of the http response ( data->rcvd_content_length is always initially 0 
		char* ptrToHeader = data->responseHeader;
		ptrToHeaderEnd = ptrToHeaderEnd + 4; // skip the new lines/ carriage returns ; we have to add + 4 because of the 4 characters \r\n\r\n

		int headerLength = ptrToHeaderEnd - payload;
		int bodyLength = *len - headerLength; // -4 because we have \r\n\r\n after the header

		// HEADER extractor
		strncpy(data->responseHeader,payload,headerLength);
		*(ptrToHeader+headerLength+1) = '\0';
		

		// extract header fields
		extract_http_header_field(data->responseContentType,(char*)"Content-Type:",data->responseHeader);
		extract_http_header_field(data->serverType,(char*)"Server:",data->responseHeader);
		extract_http_header_field(data->responseLastModified,(char*)"Last-Modified:",data->responseHeader);
		
		
			
		// CONTENT-LENGTH extractor
		char contentLengthStr[100];
		extract_http_header_field(contentLengthStr,(char*)"Content-Length:",data->responseHeader);
		int contentLength;
		if (strncmp(contentLengthStr,"N/A",3)==0) {
			// we have no content-length!
			contentLength = -1;
		}
		else {
			contentLength = atoi(contentLengthStr);
		}


		if (contentLength > 0) {
						data->rcvd_content_length = contentLength;
			data->rcvd_content_done = bodyLength;
			
			// PREPARE BINARY FILENAME
			char timestamp[200];
			create_timestamp(timestamp);
			sprintf(data->responseBodyBinaryLocation,"http/rcvd/%s",timestamp);


			append_binarydata_to_file(data->responseBodyBinaryLocation,ptrToHeaderEnd,bodyLength);

			msg(MSG_DEBUG,"wrote %d bytes to %s (total: %d) and (total: %d)",bodyLength,data->responseBodyBinaryLocation,contentLength,data->rcvd_content_length);
			
			if (bodyLength < contentLength) {	
				msg(MSG_DEBUG,"we still have some outstanding chunks, wait for them..!");
				conn->multiple_server_chunks = 1;
			}
		}
		else if (contentLength < 0) {
			// we have no content-Length! That means we have to save the whole body for this and successive connections
			data->rcvd_content_length = -1;
			data->rcvd_content_done = bodyLength;
			
			// PREPARE BINARY FILENAME
			char timestamp[200];
			create_timestamp(timestamp);
			sprintf(data->responseBodyBinaryLocation,"http/rcvd/%s",timestamp);
			
			append_binarydata_to_file(data->responseBodyBinaryLocation,ptrToHeaderEnd,bodyLength);
			msg(MSG_DEBUG,"wrote %d bytes to %s",bodyLength,data->responseBodyBinaryLocation);
			conn->multiple_server_chunks = 1;
		}
		

		// now we are finished with the initial received data, now do the logging
		logger_get()->logStruct(conn, "server", data);

	}
	else {
		data = (struct http_server_struct*) conn->log_server_struct_ptr;

		if ((data->rcvd_content_done+*len) < data->rcvd_content_length) {
			
			msg(MSG_DEBUG,"we still have to collect some data...!");
			append_binarydata_to_file(data->responseBodyBinaryLocation,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to %s",*len,data->responseBodyBinaryLocation);
			data->rcvd_content_done += *len;

			msg(MSG_DEBUG,"added: %d and now we have: %d",*len,data->rcvd_content_done);
		}
		else if ((data->rcvd_content_done+*len) == data->rcvd_content_length && data->rcvd_content_length != 0) {
			append_binarydata_to_file(data->responseBodyBinaryLocation,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to temporary http chunk collection",*len);
			data->rcvd_content_done += *len;
					
			msg(MSG_DEBUG,"we are finished reading the body!");
				
			conn->log_server_struct_ptr = NULL;
			conn->multiple_server_chunks = 0;
		}
		else if (data->rcvd_content_length == 0) {
			append_binarydata_to_file(data->responseBodyBinaryLocation,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to %s",*len,data->responseBodyBinaryLocation);
			data->rcvd_content_done += *len;
		}
		
		else
		{
			msg(MSG_DEBUG,"Malformed http request! Nothing to do");
		}
	}

	return 1;

}


int HTTPHandler::payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len)
{
	struct http_client_struct* data;
	if (conn->multiple_client_chunks == 0) {

		data = (struct http_client_struct*) malloc(sizeof(struct http_client_struct));
		conn->log_client_struct_ptr = data;

		
		// initially, we expect that request to be sent to the *REAL* malware server
		strcpy(data->responseReturnedType,"new");

		char* ptrToHeaderEnd = strstr(payload,"\r\n\r\n"); // every proper HTTP header ends with this string
		char* ptrToHeader = data->requestHeader;
		char* ptrToRequestedLocation = NULL;

		ptrToHeaderEnd = ptrToHeaderEnd + 4; // skip the new lines/ carriage returns ; we have to add + 4 because of the 4 characters \r\n\r\n
		
		int headerLength = ptrToHeaderEnd - payload;
		int bodyLength = *len - headerLength; // -4 because we have \r\n\r\n after the header
		


		// HEADER extractor
		strncpy(data->requestHeader,payload,headerLength);
		*(ptrToHeader+headerLength+1) = '\0';



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
		extract_http_header_field(contentLengthStr,(char*)"Content-Length:",data->requestHeader);
		int contentLength;
		if (strncmp(contentLengthStr,"N/A",3)==0) {
			// we have no content-length!
			contentLength = -1;
		}
		else {
			contentLength = atoi(contentLengthStr);
		}

	
		msg(MSG_DEBUG,"contentLength: %d, cleartext: %s",contentLength,ptrToHeaderEnd);
		
		if (contentLength > 0) {

			data->sent_content_length = contentLength;
			data->sent_content_done = bodyLength;
			// PREPARE BINARY FILENAME
			char timestamp[200];
			create_timestamp(timestamp);
			sprintf(data->requestBodyBinaryLocation,"http/sent/%s",timestamp);
			append_binarydata_to_file(data->requestBodyBinaryLocation,ptrToHeaderEnd,bodyLength);
			msg(MSG_DEBUG,"wrote %d bytes to %s",bodyLength,data->requestBodyBinaryLocation);
			if (bodyLength < contentLength) {
				conn->multiple_client_chunks = 1;
			}
		}
		else if (contentLength < 0 && bodyLength > 0)  {
	
			data->sent_content_length = -1;
			data->sent_content_done = bodyLength;

			// PREPARE BINARY FILENAME
			char timestamp[200];
			create_timestamp(timestamp);
			sprintf(data->requestBodyBinaryLocation,"http/sent/%s",timestamp);
			append_binarydata_to_file(data->requestBodyBinaryLocation,ptrToHeaderEnd,bodyLength);
			msg(MSG_DEBUG,"wrote %d bytes to %s",bodyLength,data->requestBodyBinaryLocation);
			conn->multiple_client_chunks = 1; // we don't know anything about the real content length thus we have to assume there are still outstanding chunks
	
		}

		extract_http_header_field(data->requestedHost,(char*)"Host:",data->requestHeader);
		extract_http_header_field(data->userAgent,(char*)"User-Agent:",data->requestHeader);
		
		
		if (conn->destOffline) {
			emulate_server_response(data,conn);
		}
		
		logger_get()->logStruct(conn, "client", data);
	

	}
	else {
		data = (struct http_client_struct*) conn->log_client_struct_ptr;
		if ((data->sent_content_done+*len) < data->sent_content_length) {
			

			append_binarydata_to_file(data->requestBodyBinaryLocation,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to %s",*len,data->requestBodyBinaryLocation);
			
			data->sent_content_done += *len;

			msg(MSG_DEBUG,"added: %d and now we have: %d",*len,data->sent_content_done);
			msg(MSG_DEBUG,"we still have to collect some data...!");
		}
		else if ((data->sent_content_done+*len) == data->sent_content_length && data->sent_content_length != 0) {
		
			append_binarydata_to_file(data->requestBodyBinaryLocation,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to %s",*len,data->requestBodyBinaryLocation);
			data->sent_content_done += *len;
			
			
			conn->log_client_struct_ptr = NULL;
			conn->multiple_client_chunks = 0;
			msg(MSG_DEBUG,"we are finished reading the body!");
		}
		else if (data->sent_content_length == 0) {
			append_binarydata_to_file(data->requestBodyBinaryLocation,payload,*len);
			msg(MSG_DEBUG,"wrote %d bytes to %s",*len,data->requestBodyBinaryLocation);
		}
		else {
			msg(MSG_DEBUG,"Malformed http request! Nothing to do with chunk of size %d", *len);
		}
	}





	return 1;
}

int HTTPHandler::determineTarget(struct sockaddr_in* addr)
{
	if (config.getMode() < full_proxy) {
		bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, config.get("http", "http_redirect").c_str(), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)80);
		msg(MSG_DEBUG,"set http dummy target!");
	}
	return 0;
}

// this method searches for the 'headername' string in the 'header' string and if the search was successful, it tries to save the value for this headername into the string 'destination'
void HTTPHandler::extract_http_header_field(char* destination, char* headername, char* header) {

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

