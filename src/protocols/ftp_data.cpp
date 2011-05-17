#include "ftp_data.h"

#include <stdlib.h>
#include <string.h>

#include "wrapper.h"
#include "helper_file.h"

#include <logging/logbase.h>
#include <logging/log_postgres.h>
#include <common/msg.h>


FTPDataHandler::FTPDataHandler(const Configuration& config)
	: ProtoHandler(config)
{

}

int FTPDataHandler::payloadServerToClient(connection_t* conn, const char* payload, ssize_t* len)
{
	struct ftp_data_struct* data;
	if (conn->log_server_struct_initialized == 0) {
		// PREPARE BINARY FILENAME
       		char timestamp[200];
		create_timestamp(timestamp);
		data = (struct ftp_data_struct*) malloc(sizeof(struct ftp_data_struct));
		conn->log_server_struct_initialized = 1;
		conn->log_server_struct_ptr = data;
		sprintf(data->binaryLocation,"ftp_data/rcvd/%s",timestamp);
		

		append_binarydata_to_file(data->binaryLocation,payload,*len);

		logger_get()->logStruct(conn, "server", data);
			
		conn->multiple_server_chunks = 1;
	}
	else {
		data = (struct ftp_data_struct*) conn->log_server_struct_ptr;

		append_binarydata_to_file(data->binaryLocation,payload,*len);
			
	}


	msg(MSG_DEBUG,"rcvd %d",*len);
 	return 1;

}

int FTPDataHandler::payloadClientToServer(connection_t* conn, const char* payload, ssize_t* len)
{
	struct ftp_data_struct* data;
	if (conn->log_client_struct_initialized == 0) {
		// do the logging stuff
		char timestamp[200];
		create_timestamp(timestamp);
	
		data = (struct ftp_data_struct*) malloc(sizeof(struct ftp_data_struct));
		conn->log_client_struct_initialized = 1;
		conn->log_client_struct_ptr = data;

		sprintf(data->binaryLocation,"ftp_data/sent/%s",timestamp);
		append_binarydata_to_file(data->binaryLocation,payload,*len);
		
		logger_get()->logStruct(conn, "client", data);
		conn->multiple_client_chunks = 1;
	}
	else {
	  	// append the incoming data to the file
		data = (struct ftp_data_struct*) conn->log_client_struct_ptr;
		append_binarydata_to_file(data->binaryLocation,payload,*len);
		
	}


 	msg(MSG_DEBUG,"sent %d",*len);
	return 1;
}

int FTPDataHandler::determineTarget(struct sockaddr_in* addr)
{
	return 0;
}

