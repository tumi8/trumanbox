#include "ftp_data.h"

#include <stdlib.h>
#include <string.h>
#include "log_postgres.h"
#include "wrapper.h"
#include "logger.h"
#include "helper_file.h"
#include "msg.h"

//PGconn *psql;

struct ph_ftp_data {
	struct configuration_t* config;
};

void* ph_ftp_data_create()
{
	void* ret = malloc(sizeof(struct ph_ftp_data));
	return ret;
}

int ph_ftp_data_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_ftp_data_init(void* handler, struct configuration_t* c)
{
	struct ph_ftp_data* ftp_data = (struct ph_ftp_data*)handler;
	ftp_data->config = c;
	return 0;
}

int ph_ftp_data_deinit(void* handler)
{
	return 0;
}

int ph_ftp_data_handle_payload_stc(void* handler, connection_t* conn, const char* payload, ssize_t* len)
{
	struct ftp_data_struct* data;
	if (conn->multiple_server_chunks == 0) {
		// PREPARE BINARY FILENAME
       		char timestamp[200];
		create_timestamp(timestamp);
		data = (struct ftp_data_struct*) malloc(sizeof(struct ftp_data_struct));
		sprintf(data->binaryLocation,"ftp_data/rcvd/%s",timestamp);
	

			
		logger_get()->log_struct(logger_get(), conn, "server", data);
			
		conn->multiple_server_chunks = 1;
	}
	else {
	  	// append the incoming data to the file
	}


	msg(MSG_DEBUG,"rcvd %d",*len);
 	return 1;

}

int ph_ftp_data_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len)
{
	struct ftp_data_struct* data;
	if (conn->multiple_client_chunks == 0) {
		// do the logging stuff
		char timestamp[200];
		create_timestamp(timestamp);
	
		data = (struct ftp_data_struct*) malloc(sizeof(struct ftp_data_struct));
		
		sprintf(data->binaryLocation,"ftp_data/sent/%s",timestamp);
			
		
		logger_get()->log_struct(logger_get(), conn, "client", data);
		conn->multiple_client_chunks = 1;
	}
	else {
	  	// append the incoming data to the file
	}


 	msg(MSG_DEBUG,"sent %d",*len);
	return 1;
}

int ph_ftp_data_handle_packet(void* handler, const char* packet, ssize_t len)
{
	return 0;
}

int ph_ftp_data_determine_target(void* handler, struct sockaddr_in* addr)
{
	return 0;
}

