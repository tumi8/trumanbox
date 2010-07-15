#include "http_post.h"
#include "wrapper.h"
#include "logger.h"
#include "helper_file.h"
#include "msg.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ph_http_post {
	struct configuration_t* config;
};

void* ph_http_post_create()
{
	msg(MSG_DEBUG,"HTTP_POST handler created");
	void* ret = malloc(sizeof(struct ph_http_post));
	return ret;
}


int ph_http_post_destroy(void* handler)
{
	free(handler);
	return 0;
}

int ph_http_post_init(void* handler, struct configuration_t* c)
{
	msg(MSG_DEBUG,"HTTP_POST handler initialized");
	struct ph_http_post* http_post = (struct ph_http_post*)handler;
	http_post->config = c;
	return 0;
}

int ph_http_post_deinit(void* handler)
{
	return 0;
}

int ph_http_post_handle_payload_stc(void* handler, connection_t* conn, const char* payload, size_t* len)
{
	msg(MSG_DEBUG,"entered http post handler");
	return logger_get()->log(logger_get(), conn, "server", payload);
}

int ph_http_post_handle_payload_cts(void* handler, connection_t* conn, const char* payload, size_t* len)
{
	msg(MSG_DEBUG,"entered http post handler");
	save_request(conn, payload);
	msg(MSG_DEBUG,"now we enter the build_tree method with %s\n",payload);
	build_tree(conn,payload);
	
	return logger_get()->log(logger_get(), conn, "client", payload);
}

int ph_http_post_handle_packet(void* handler, const char* packet, size_t len)
{
	return 0;
}

int ph_http_post_determine_target(void* handler, struct sockaddr_in* addr)
{
	struct ph_http_post* http_post = (struct ph_http_post*)handler;
	if (conf_get_mode(http_post->config) < full_proxy) {
                bzero(addr, sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                Inet_pton(AF_INET, conf_get(http_post->config, "http", "http_redirect"), &addr->sin_addr);
		addr->sin_port = htons((uint16_t)80);
	}
	return 0;
}

/*
void save_request(const connection_t *conn, const char *cmd_str) {
		char* ptrToBody = NULL; //this pointer contains the address of the body of the POST-Request

		ptrToBody  = strstr(cmd_str, "\r\n\r\n"); // skip the new lines/ carriage returns
				
		if (ptrToBody== NULL) //no body found, this should never happen if the body is not empty - see HTTP RFC 2616
			return;
		
		ptrToBody = ptrToBody + 4; // move 4 characters forward because of the sequence:  \r\n\r\n
		msg(MSG_DEBUG,"Payload POST:%s\n",ptrToBody);
		FILE *fd;

		if ((fd= fopen("test.out","wx")) != NULL) { 
			while(*ptrToBody != '\0') {
				putc(*ptrToBody,fd);
				ptrToBody++;
			}
			putc('\0',fd);
		}
		else if (errno == EEXIST) {
			msg(MSG_ERROR,"test.out already xists...");
			return;
		}
		else {
		 	msg(MSG_ERROR,"could not create file");
			return;
			}
		fflush(fd);
		Close_file(fd);


}
*/
