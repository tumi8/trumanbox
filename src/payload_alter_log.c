#include "payload_alter_log.h"
#include "dispatching.h"
#include "wrapper.h"
#include "helper_file.h"
#include "helper_net.h"
#include "payload_ident.h"


void print_payload(const u_char *payload, int len) {
	int i;
	const u_char *ch;
	ch = payload;
	for(i = 0; i < len; i++) {
//		if (isascii(*ch))
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}
	return;
}

int print_timestamp(const connection_t *connection, char *protocol_dir) {
	char		outstr[100];
	time_t		t;
	struct tm	*tmp;

	t = time(NULL);
	tmp = localtime(&t);

	if (tmp == NULL) {
		fprintf(stderr, "localtime error\n");
		return 0;
	}

	if (strftime(outstr, sizeof(outstr), "%Y-%m-%d-%H-%M-%S\n", tmp) == 0) {
		fprintf(stderr, "strftime returned 0\n");
		return 0;
	}

	append_to_file(outstr, connection, protocol_dir);
	return 1;
}

void delete_row_starting_with_pattern(char *datastring, const char *pattern) {
	char *ptr;

	ptr = strstr(datastring, pattern);

	if ( ptr == NULL ) 
		return;

	ptr = strchr(ptr, ':');

	if ( ptr == NULL ) 
		return;

	ptr++;

	while (*ptr != '\n') {
		*ptr = ' ';
		ptr++;
	}
	return;
}

// server to client content substitution
void content_substitution_and_logging_stc(const connection_t *conn, char *data, int *data_len) {
	char 	*ptr,
		filename[25];
	int ip1, ip2, ip3, ip4, p1, p2;

	memset(filename, 0, sizeof(filename));
	
	if (conn->app_proto == FTP || conn->app_proto == FTP_anonym) {
		if (strncmp(data, "227 ", 4) == 0) {
			sscanf(strrchr(data, '('), "(%*d,%*d,%*d,%*d,%d,%d).", &p1, &p2);
			sscanf((char *) conn->dest, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
			ptr = strrchr(data, '(');
			ptr++;
			sprintf(ptr, "%d,%d,%d,%d,%d,%d).\r\n", ip1, ip2, ip3, ip4, p1, p2);
			printf("p1: %d\np2: %d\n\n", p1, p2);  // for debugging
			printf("changed payload from server:\n%s\n\n", data);
			*data_len = strlen(data);
			sprintf(filename, "%d.%d.%d.%d:%d", ip1, ip2, ip3, ip4, ((256*p1)+p2));
			write_to_file("FTP_data\n", filename, RESPONSE_COLLECTING_DIR);
		}
	}
	return;
}

// client to server content substitution
void content_substitution_and_logging_cts(const connection_t *conn, char *data, int *data_len) {
	char 	*ptr,
		username[50],
		password[50],
		catched_data[100];
	//int 	ip1, ip2, ip3, ip4, p1, p2,
	int	more_logging = 1;

	switch (conn->app_proto) {
		case FTP:
			if (strncmp(data, "USER ", 5) == 0) {
				printf("we catched a USER token\n");
				strncpy(username, data, sizeof(username)-1);
				printf("and username is: %s\n", username);
				printf("now we append the username: %s to our accountfile\n", username);
				append_to_file(username, conn, FTP_COLLECTING_DIR);
		
				if (strncmp(data, "USER anonymous", 14) == 0)
					return;
		
				ptr = strchr(data, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_USER);
				printf("changed payload from client:\n%s\n\n", data);
				*data_len = strlen(data);
			}
			else if (strncmp(data, "PASS ", 5) == 0) {
				printf("we catched a PASS token\n");
				strncpy(password, data, sizeof(password)-1);
				printf("now we append the pwd: %s to our accountfile\n", password);
				append_to_file(password, conn, FTP_COLLECTING_DIR);
				ptr = strchr(data, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_PASS);
				printf("changed payload from client:\n%s\n\n", data);
				*data_len = strlen(data);
			}
			else if (more_logging) 
				append_to_file(data, conn, FTP_COLLECTING_DIR);

			break;
		case FTP_anonym:
			if (strncmp(data, "USER ", 5) == 0) {
				printf("we catched a USER token\n");
				strncpy(username, data, sizeof(username)-1);
				printf("and username is: %s\n", username);
				printf("now we append the username: %s to our accountfile\n", username);
				append_to_file(username, conn, FTP_COLLECTING_DIR);		
				ptr = strchr(data, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_USER);
				printf("changed payload from client:\n%s\n\n", data);
				*data_len = strlen(data);
			}
			else if (strncmp(data, "PASS ", 5) == 0) {
				printf("we catched a PASS token\n");
				strncpy(password, data, sizeof(password)-1);
				printf("now we append the pwd: %s to our accountfile\n", password);
				append_to_file(password, conn, FTP_COLLECTING_DIR);
				ptr = strchr(data, ' ');
				ptr++;
				sprintf(ptr, VALID_FTP_PASS);
				printf("changed payload from client:\n%s\n\n", data);
				*data_len = strlen(data);
			}
			else if (more_logging)
				append_to_file(data, conn, FTP_COLLECTING_DIR);

			break;	
		case IRC:
			if (strncmp(data, "USER ", 5) == 0) {
				printf("we catched a USER token\n");
				strncpy(username, data, sizeof(username)-1);
				printf("and username is: %s\n", username);
				printf("now we append the username: %s to our accountfile\n", username);
				append_to_file(username, conn, IRC_COLLECTING_DIR);
			}
			else if (strncmp(data, "PASS ", 5) == 0) {
				printf("we catched a PASS token\n");
				strncpy(password, data, sizeof(password)-1);
				printf("now we append the pwd: %s to our accountfile\n", password);
				append_to_file(password, conn, IRC_COLLECTING_DIR);
			}
			else if (strncmp(data, "JOIN ", 5) == 0) {
				printf("we catched a JOIN token\n");
				strncpy(catched_data, data, sizeof(catched_data)-1);
				printf("now we append: %s to our accountfile\n", catched_data);
				append_to_file(catched_data, conn, IRC_COLLECTING_DIR);
			}
			else if (strncmp(data, "WHO ", 4) == 0) {
				printf("we catched a WHO token\n");
				strncpy(catched_data, data, sizeof(catched_data)-1);
				printf("now we append: %s to our accountfile\n", catched_data);
				append_to_file(catched_data, conn, IRC_COLLECTING_DIR);
			}
			else if (strncmp(data, "NICK ", 5) == 0) {
				printf("we catched a NICK token\n");
				strncpy(catched_data, data, sizeof(catched_data)-1);
				printf("now we append: %s to our accountfile\n", catched_data);
				append_to_file(catched_data, conn, IRC_COLLECTING_DIR);
			}
			else if (strncmp(data, "PROTOCTL ", 9) == 0) {
				printf("we catched a PROTOCTL token\n");
				strncpy(catched_data, data, sizeof(catched_data)-1);
				printf("now we append: %s to our accountfile\n", catched_data);
				append_to_file(catched_data, conn, IRC_COLLECTING_DIR);
			}
			else if (strncmp(data, "PING ", 5) == 0) {
				printf("we catched a PING token\n");
				strncpy(catched_data, data, sizeof(catched_data)-1);
				printf("now we append: %s to our accountfile\n", catched_data);
				append_to_file(catched_data, conn, IRC_COLLECTING_DIR);
			}
			else if (strncmp(data, "MODE ", 5) == 0) {
				printf("we catched a MODE token\n");
				strncpy(catched_data, data, sizeof(catched_data)-1);
				printf("now we append: %s to our accountfile\n", catched_data);
				append_to_file(catched_data, conn, IRC_COLLECTING_DIR);
			}
			else if (more_logging)
				append_to_file(data, conn, IRC_COLLECTING_DIR);
			break;
		case SMTP:
			if (more_logging)
				append_to_file(data, conn, SMTP_COLLECTING_DIR);

			if (strncasecmp(data, "rcpt to:", 8) == 0) {
				ptr = strchr(data, ':');
				ptr++;
				sprintf(ptr, LOCAL_EMAIL_ADDRESS);
				printf("changed payload from client:\n%s\n\n", data);
				*data_len = strlen(data);
				
			}
			break;
		case HTTP:
			if (more_logging)
				append_to_file(data, conn, HTTP_COLLECTING_DIR);
			break;
		default:
			break;
	}
	return;
}


