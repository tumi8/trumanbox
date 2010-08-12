#include "ftp.h"

#include <stdlib.h>
#include <string.h>
#include "log_postgres.h"
#include "wrapper.h"
#include "logger.h"
#include "helper_file.h"
#include "msg.h"

//PGconn *psql;

struct ph_ftp {
	struct configuration_t* config;
};

void* ph_ftp_create()
{
	void* ret = malloc(sizeof(struct ph_ftp));
	return ret;
}

int ph_ftp_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_ftp_init(void* handler, struct configuration_t* c)
{
	struct ph_ftp* ftp = (struct ph_ftp*)handler;
	ftp->config = c;
	return 0;
}

int ph_ftp_deinit(void* handler)
{
	return 0;
}

int ph_ftp_handle_payload_stc(void* handler, connection_t* conn, const char* payload, ssize_t* len)
{
	struct ftp_struct* data;

	if (conn->log_server_struct_initialized == 0) {
              	data = (struct ftp_struct*) malloc(sizeof(struct ftp_struct)); 
		conn->log_server_struct_ptr = data;
		conn->log_server_struct_initialized = 1;
		bzero(data,sizeof(struct ftp_struct));
		msg(MSG_DEBUG,"initalized a new serverstrcut and we have %d",data->pasvPort);
	}
	else {
		data = (struct ftp_struct*) conn->log_server_struct_ptr;
	}

        char msgCopy[MAXLINE]; 
        char* linePtr = NULL; 
        strcpy(msgCopy,payload); 
        linePtr = strtok (msgCopy,"\n"); 
 
        while (linePtr!= NULL) { 
                // parse all response lines from server 
        	bzero(data->Message,MAXLINE); 
                //extract server message (arbitrary length possible) 
                int Msglength = strcspn(linePtr,"\r\n"); 
                strncpy(data->Message,linePtr,Msglength);

		if (strncmp(linePtr,"227",3) == 0)  {
			// we found as passive command!
			// try to extract the information...
			char* ptr = strstr(linePtr,"(");
			ptr =  ptr + 1; // now we see the first part of the ip
			msg(MSG_DEBUG,"lets go [%s]",linePtr);
			char IP[100] = ""; // initialize as empty string because we want directly append to it!
			//u_int16_t port;
			int i;
		
			for (i = 0 ; i < 4;  i++) {
				int len = strcspn(ptr,",");
				strncat(IP,ptr,len);
				if (i < 3) 
					strncat(IP,".",1);
				ptr = ptr + len +1;
			}
			msg(MSG_DEBUG,"IP: %s",IP);
		
			// TRY to extract port
			
			char portPart1[100] = "";
			char portPart2[100] = "";

			int len1 = strcspn(ptr,",");
			strncat(portPart1,ptr,len1);
			ptr = ptr + len1 + 1;
			
			int len2 =  strcspn(ptr,")");
			strncat(portPart2,ptr,len2);
			

			int port1,port2;
			u_int16_t port;
			port1 = atoi(portPart1);
			port2 = atoi(portPart2);
			port = port1 * 256 + port2;
			
			strcpy(data->serverIP,IP);
			data->pasvPort = port;
			msg(MSG_DEBUG,"PASV Mode will be opened at IP: [%s] Port: [%d]",data->serverIP,data->pasvPort);
		}
		else if (strncmp(linePtr,"229",3) == 0) {
			// we found a extended passive command reply
			char* ptr = strstr(linePtr,"(");
			ptr = ptr + 4; // we have to skip the character sequence (||| 
			char* ptrEnd = strstr(ptr,"|");
			int len = ptrEnd - ptr;
			char portString[100] = "";
			strncat(portString,ptr,len);
			strcpy(data->serverIP,conn->dest);
			data->pasvPort = atoi(portString);


			msg(MSG_DEBUG,"extracted port: %s",portString);

		}


                linePtr = strtok(NULL,"\n"); 
                logger_get()->log_struct(logger_get(), conn, "server", data); 
        } 

        return 1;

}

int ph_ftp_handle_payload_cts(void* handler, connection_t* conn, const char* payload, ssize_t* len)
{
	struct ftp_struct* data;

	if (conn->log_server_struct_initialized == 0) {
              	data = (struct ftp_struct*) malloc(sizeof(struct ftp_struct)); 
		conn->log_server_struct_ptr = data;
		conn->log_server_struct_initialized = 1;
	}
	else {
		data = (struct ftp_struct*) conn->log_server_struct_ptr;
	}
        char msgCopy[MAXLINE];
        char* linePtr = NULL;
        strcpy(msgCopy,payload);
        linePtr = strtok (msgCopy,"\n");

        while (linePtr!= NULL) {
                // parse all response lines from server
              	bzero(data->Message,MAXLINE); 
                //extract server message (arbitrary length possible)
                int Msglength = strcspn(linePtr,"\r\n");
                strncpy(data->Message,linePtr,Msglength);
 
		if (data->pasvPort > 0) {
			char* ptr = strstr(linePtr," ");
		
			char transferType[100] = "";
			char filename[MAX_PATH_LENGTH] = "";
			if (ptr != NULL) {
				int lenTransferType = strcspn(linePtr," ");
				strncat(transferType,linePtr,lenTransferType);
				ptr = ptr + 1;

				strcpy(filename,ptr);

			}
			else {
				strncat(transferType,linePtr,Msglength);
			}
			char statement[1000] = "";
			snprintf(statement, 1000, "insert into AWAITED_PASV (ServerIP,ServerPort,Type,filename) Values (inet('%s'),%d,'%s', '%s')",
			data->serverIP,data->pasvPort,transferType,filename
			);
			msg(MSG_DEBUG,"insertion: %s",statement);
			if (execute_statement(statement)) {
				msg(MSG_DEBUG,"inserted new pasv information successfully");
			}
			else {
				msg(MSG_DEBUG,"insertion not succcessful!");
			}
			data->pasvPort = 0;

			bzero(data->serverIP,100); 
		}


                linePtr = strtok(NULL,"\n");
                logger_get()->log_struct(logger_get(), conn, "client", data);
        }

	/*char 	*ptr,
		username[50],
		password[50];
	build_tree(conn, payload);
	switch(conn->app_proto) {
		case FTP:
			if (strncmp(payload, "USER ", 5) == 0) {
				msg(MSG_DEBUG, "we caught a USER token");
				strncpy(username, payload, sizeof(username)-1);
				msg(MSG_DEBUG, "and username is: %s", username);
				msg(MSG_DEBUG, "now we append the username: %s to our accountfile", username);
				//append_to_file(username, conn, FTP_COLLECTING_DIR);
		
				if (strncmp(payload, "USER anonymous", 14) == 0)
					return 0;
		
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_USER);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				*len = strlen(payload);
			} else if (strncmp(payload, "PASS ", 5) == 0) {
				msg(MSG_DEBUG, "we caught a PASS token");
				strncpy(password, payload, sizeof(password)-1);
				msg(MSG_DEBUG, "now we append the pwd: %s to our accountfile", password);
				//append_to_file(password, conn, FTP_COLLECTING_DIR);
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_PASS);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				*len = strlen(payload);
			}
			break;
		case FTP_anonym:
			if (strncmp(payload, "USER ", 5) == 0) {
				msg(MSG_DEBUG, "we caught a USER token");
				strncpy(username, payload, sizeof(username)-1);
				msg(MSG_DEBUG, "and username is: %s", username);
				msg(MSG_DEBUG, "now we append the username: %s to our accountfile", username);
				//append_to_file(username, conn, FTP_COLLECTING_DIR);		
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_USER);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				*len = strlen(payload);
			} else if (strncmp(payload, "PASS ", 5) == 0) {
				msg(MSG_DEBUG, "we caught a PASS token");
				strncpy(password, payload, sizeof(password)-1);
				msg(MSG_DEBUG, "now we append the pwd: %s to our accountfile", password);
				//append_to_file(password, conn, FTP_COLLECTING_DIR);
				ptr = strchr(payload, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_PASS);
				msg(MSG_DEBUG, "changed payload from client:\n%s", payload);
				*len = strlen(payload);
			}
			break;	
		default:
			break;
	}
	return logger_get()->log(logger_get(), conn, "content-cts", payload);
	*/
	return 1;
}

int ph_ftp_handle_packet(void* handler, const char* packet, ssize_t len)
{
	return 0;
}

int ph_ftp_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_ftp* ftp = (struct ph_ftp*)handler;
	if (conf_get_mode(ftp->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(ftp->config, "ftp", "ftp_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)21);
	}
	return 0;
}

