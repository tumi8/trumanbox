#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "definitions.h"

struct logger_t;
struct configuration_t;

typedef int (lg_init)(struct logger_t*); // initializes the logging module. Returns 0 on sucess, a value < 0 otherwise
typedef int (lg_deinit)(struct logger_t*);// cleans up the logger module. Returns 0 on sucess, a value < 0 otherwise
typedef int (lg_create_log)(struct logger_t*);
typedef int (lg_finish_log)(struct logger_t*);
typedef int (lg_log_text)(struct logger_t*, connection_t* conn, const char* tag, const char* message);
typedef int (lg_log_struct)(struct logger_t*, connection_t* conn, const char* tag, void* data);

enum logger_type { directory };

/**
 * Note: Every logging module MUST expect concurrent access to the logging
 * data by several *processes*, as the trumanbox forks several processes 
 * in order to do its jobs.
 */
struct logger_t {
	void* data;
	struct configuration_t* config;

	lg_init* init; // will be called by logger_create
	lg_deinit* deinit; // will be called by logger_destroy
	lg_create_log* create_log;
	lg_finish_log* finish_log;
	lg_log_text* log;
	lg_log_struct* log_struct;
};


struct http_client_struct {
	char requestedHost[1000];
	char requestedLocation[1000];
	char userAgent[1000];
	char method[20];
	char requestHeader[MAXLINE];
	char requestBodyBinaryLocation[1000];
        u_int32_t sent_content_length; // the conteht length of the whole chunk of data we expect to send
	u_int32_t sent_content_done;
	char* sent_content_done_ptr; // pointer to memory where we store the already sent data
};

struct http_server_struct {
	char responseHeader[MAXLINE];
	char responseBodyBinaryLocation[1000];
	char responseLastModified[MAXLINE];
	char responseContentType[1000];
	char serverType[1000];
	u_int32_t rcvd_content_length; // the content length of the whole chunk of data we expect to receive 
	u_int32_t rcvd_content_done;
	char* rcvd_content_done_ptr; // pointer to memory where we store the already received data
};

struct smtp_client_struct {
	char clientMsg[MAXLINE];
};

struct smtp_server_struct {
	char statusCode[20];
	char serverMsg[MAXLINE];
};

struct irc_client_struct {
	char command[MAXLINE];
	char arguments[MAXLINE];
};

struct irc_server_struct {
	char serverName[1000];
	char numericReply[1000];
	char recipientNickname[1000];
	char message[MAXLINE];
};

struct unknown_client_struct {
	char clientMsg[MAXLINE];
	char clientMsgBinaryLocation[1000];
};

struct unknown_server_struct {
	char serverMsg[MAXLINE];
	char serverMsgBinaryLocation[1000];
};


struct dns_struct {
	char domain[1000];
	char realServerIP[100];
	char serverIP[100];
	char clientIP[100];
};


/**
 * This creates the logging object. There may be an arbitrary number of logging objets
 * The logging module *MUST* expect that different *processes* and modules want concurrent 
 * access to the logs. 
 */
int logger_create(struct configuration_t* config);

/**
 * Get the global logger object. logger_create MUST be called once before 
 * this function is called.
 */
inline struct logger_t* logger_get();

int logger_destroy();

#endif
