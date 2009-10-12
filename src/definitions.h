#include <sys/types.h>

#define VALID_FTP_USER		"ftpin\r\n"
#define VALID_FTP_PASS		"123\r\n"
#define LOCAL_EMAIL_ADDRESS	" christiangorecki@localhost\r\n"   // email address with a leading blank

#define MAX_PATH_LENGTH 	4096
#define HTTP_BASE_DIR 		"/var/www"
#define FTP_BASE_DIR 		"/home/ftp"
//#define TRUMAN_CONFIG_DIR 	"/etc/trumanbox"
#define TMP_TRUMANBOX 		"/var/local/truman-box"
#define RESPONSE_COLLECTING_DIR "/var/local/truman-box/server-responses"
#define FTP_COLLECTING_DIR 	"/var/local/truman-box/ftp"
#define IRC_COLLECTING_DIR 	"/var/local/truman-box/irc"
#define SMTP_COLLECTING_DIR	"/var/local/truman-box/smtp"
#define HTTP_COLLECTING_DIR	"/var/local/truman-box/http"
#define DUMP_FOLDER		"/var/local/truman-box/dump"
#define BUFSIZE 		65535
#define	BUFFSIZE		20000	/* buffer size for reads and writes */
#define MAXLINE 		10000
#define NUM_BUF_PACKETS 	3
#define IPQ_READ_TIMEOUT 	0
#define PERMS 			0600
#define TIMEOUT_READ_FROM_CONN 	2
#define DISP_SERV_PORT 		400
#define TB_LISTEN_PORT		400  /* this is going to replace DISP_SERV_PORT*/
/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	MAXSOCKADDR  		128	/* max socket address structure size */
#define IPLENGTH	25
#define MAX_LINE_LENGTH 1024
#define DNS_LISTEN_PORT 53
#define DNS_LISTEN_ADDRESS  "127.0.0.1"
#define DNS_RESOLVE_ADDRESS "127.0.0.1" // default resoponse for dns requests

#define	SA	struct sockaddr

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef PROTOCOLS_APP_H
#define PROTOCOLS_APP_H
enum e_protocols_app {SMTP, FTP, FTP_anonym, FTP_data, HTTP, IRC, UNKNOWN};  // so if protocol is <= 3 we start from server side
typedef enum e_protocols_app protocols_app;
#endif

#ifndef PROTOCOLS_NET_H
#define PROTOCOLS_NET_H
enum e_protocols_net {TCP, UDP, ICMP, UNSUPPORTED, ERROR};
typedef enum e_protocols_net protocols_net;
#endif

#ifndef CONNECTION_H
#define CONNECTION_H
struct s_connection {
	char source[IPLENGTH];
	char dest[IPLENGTH];
	u_int16_t sport;
	u_int16_t dport;
	protocols_net net_proto;
	protocols_app app_proto;
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

