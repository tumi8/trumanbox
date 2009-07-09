#include "payload_ident.h"
#include "helper_net.h"


char *strcasestr(const char *haystack, const char *needle);

void protocol_identified_by_port(int mode, connection_t *conn, char *payload) {
	// here we will still implement the check, if we already know the answer by checking if we have a reponse file with corresponding ip:port name and feed payload with it
	
	// FIXME: unused variables
	// int 			fd, n, 
	int			anonym_ftp;

	switch (conn->dport) {
		case 21:
			conn->app_proto = FTP;
			break;
		case 25:
			conn->app_proto = SMTP;
			break;
		case 80:
			conn->app_proto = HTTP;
			break;
		case 6667:
			conn->app_proto = IRC;
			break;
		default:
			conn->app_proto = UNKNOWN;
			break;
	}
	if (conn->app_proto == FTP || conn->app_proto == SMTP)
		fetch_banner(mode, conn, payload, &anonym_ftp);

	if (conn->app_proto == FTP && anonym_ftp)
		conn->app_proto = FTP_anonym;

	printf("protocol identified by port is: %d\n", conn->app_proto);
}

void protocol_identified_by_payload(int mode, connection_t *conn, int inconnfd, char *payload) {
	// here we need to implement logging of responses to file
	int			r, anonym_ftp;
	char			filename[30];

	conn->app_proto = UNKNOWN;

	while (readable_timeout(inconnfd, 1)) {
		if ((r = read(inconnfd, payload, MAXLINE-1)) <= 0) {
			printf("no characters have been read from client\n");
			break;
		} 
		printf("%d characters have been read\n", r);
	}

	if (!strlen(payload)) {
		fetch_banner(mode, conn, payload, &anonym_ftp);
		printf("the payload we fetched is:\n%s\n", payload);
	}

	if (strlen(payload)) {
		if (strncmp(payload, "GET /", 5) == 0)
			conn->app_proto = HTTP;
		else if (strncmp(payload, "NICK ", 5) == 0)
			conn->app_proto = IRC;
		else if (strncmp(payload, "FTP_data", 8) == 0) {
			conn->app_proto = FTP_data;
			sprintf(filename, "%s:%d", conn->dest, conn->dport);
			if (0 > remove((char *)filename))
				fprintf(stderr, "could not remove the file: %s\n", (char *) filename);
			else
				printf("and now we removed the file: %s\n", (char *) filename);
		}
		else if (strncmp(payload, "220 ", 4) == 0) {
			if ( strcasestr(payload, "ftp") != 0 ) {
				conn->app_proto = FTP;
				if (anonym_ftp)
					conn->app_proto = FTP_anonym;
			}
			else if ( (strcasestr(payload, "mail") != 0) || (strcasestr(payload, "smtp") != 0) ) {
				conn->app_proto = SMTP;
			}
			else {
				conn->app_proto = UNKNOWN;
			}
		}
		else {
			conn->app_proto = UNKNOWN;
		}
	}

	printf("protocol identified by payload is: %d\n", conn->app_proto);
}

