#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <sys/types.h>

#define VALID_FTP_USER		"ftpin\r\n"
#define VALID_FTP_PASS		"123\r\n"
#define LOCAL_EMAIL_ADDRESS	" martinmax1860@yahoo.de\r\n"   // email address with a leading blank

#define MAX_PATH_LENGTH 	4096
#define HTTP_BASE_DIR 		"/var/www/localhost/htdocs"
#define FTP_BASE_DIR 		"/home/ftp"
#define BUFSIZE 		65535
#define	BUFFSIZE		20000	/* buffer size for reads and writes */
#define MAXLINE 		10000
#define NUM_BUF_PACKETS 	3
#define IPQ_READ_TIMEOUT 	0
#define PERMS 			0700
#define TIMEOUT_READ_FROM_CONN 	2
#define DISP_SERV_PORT 		400
#define TB_LISTEN_PORT		400  /* this is going to replace DISP_SERV_PORT*/
#define TB_CONTROL_PORT		401
/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	MAXSOCKADDR  		128	/* max socket address structure size */
#define IPLENGTH	25
#define MAX_LINE_LENGTH 1024

#define	SA	struct sockaddr

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef PROTOCOLS_APP_H
#define PROTOCOLS_APP_H

// IMPORTANT: UNKNOWN NEEDS TO BE THE LAST ELEMENT OF THIS ENUM!!!!!! (check proto_handler.c for more information)
enum e_protocols_app {SMTP, FTP, FTP_anonym, FTP_data, HTTP, IRC, DNS, SSL_Proto, UNKNOWN_UDP, UNKNOWN};  // so if protocol is <= 3 we start from server side
typedef enum e_protocols_app protocols_app;
#endif

#ifndef PROTOCOLS_NET_H
#define PROTOCOLS_NET_H
enum e_protocols_net {TCP, UDP, ICMP, CONTROL, UNSUPPORTED, ERROR};
typedef enum e_protocols_net protocols_net;
#endif

#ifndef CONNECTION_H
#define CONNECTION_H
struct s_connection {
	char orig_source[IPLENGTH];
	char source[IPLENGTH];
	char dest[IPLENGTH];
	char orig_dest[IPLENGTH];
	u_int16_t sport;
	u_int16_t dport;
	u_int16_t orig_dport;
	protocols_net net_proto;
	protocols_app app_proto;
	int destOffline; // indicates whether this connection should be handled in emulation mode (that is, manipulating the payload)
	char timestamp[100]; // indicates the system-time when the connection was initialized (form of timestamp: "[secs_msecs]") (since epoch - 1.1.1970)
	u_int32_t multiple_client_chunks;// indicates whether we expect multiple successive chunks from client side that belong together
	u_int32_t multiple_server_chunks; // indicates whether we expect multiple successive chunks from server side that belong together
	void* log_client_struct_ptr; // pointer to client logging structure (CTS)
	void* log_server_struct_ptr; // pointer to server logging structure (STC)
	u_int16_t log_client_struct_initialized; // indicates if the client log_struct (CTS) was already malloced (that is, if the log_struct ptr already points to a valid destination)
	u_int16_t log_server_struct_initialized; // indicates if the server log_struct (STC) was already malloced (that is, if the log_struct ptr already points to a valid destination)
	char timestampEmulation[100]; // If we are in emulation mode, we need a timestamp to refer to when getting data from the the database with old logs (this timestamp serves as a key)
	char sslVersion[100]; // If we are in SSL mode, we obtain the SSL version information in the packet inspection module (proto_truman_ident.c) and we have to write it into the connection struct
	int countReads; // indicates the number of read() operations performed on a single TCP connection (server+client) 
};
typedef struct s_connection connection_t;
#endif


#ifndef OPERATION_MODE_H_
#define OPERATION_MODE_H_
typedef enum {
	invalid,
	full_emulation,
	half_proxy,
	full_proxy,
	quit
} operation_mode_t;
#endif


#endif
