#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <netinet/in.h>

#include "helper_net.h"
#include "msg.h"
#include "wrapper.h"
#include "helper_file.h"


int readable_timeout(int fd, int sec) {
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	tv.tv_sec = sec;
	tv.tv_usec = 0;

	return (select(fd+1, &rset, NULL, NULL, &tv));
}

// this function returns 1 if anonymous login is possible, otherwise 0
int try_anonymous_login(int conn_fd) {
	char user_anonym[50] = "USER anonymous\r\n";
	char pwd_anonym[50] = "PASS example@mozilla.org\r\n";
	char response[MAXLINE];
	char *write_ptr;
	int r, w;
	
	write_ptr = user_anonym;
	r = strlen(user_anonym);

	while (r > 0 && (w = write(conn_fd, write_ptr, r)) > 0) {
		write_ptr += w;
		r -= w;
	}
	
	while (readable_timeout(conn_fd, TIMEOUT_READ_FROM_CONN)) {
		r = read(conn_fd, response, MAXLINE-1);
		if (r == 0) {
			msg(MSG_DEBUG, "connection has been closed");
			break;
		} else if (r == -1) {
			msg(MSG_ERROR, "Error on read: %s", strerror(errno));
			break;
		}
		msg(MSG_DEBUG, "%d characters have been read\n", r);
		if (response[0] == '5' || response[0] == '4')  // if negative or temporary neg. response
			return 0;
		memset(response, 0, sizeof(response));
	}

	write_ptr = pwd_anonym;
	r = strlen(pwd_anonym);

	while (r > 0 && (w = write(conn_fd, write_ptr, r)) > 0) {
		write_ptr += w;
		r -= w;
	}

	while (readable_timeout(conn_fd, TIMEOUT_READ_FROM_CONN)) {
		r = read(conn_fd, response, MAXLINE-1);
		if (r == 0) {
			msg(MSG_DEBUG, "connection has been closed");
			break;
		} else if (r == -1) {
			msg(MSG_ERROR, "Error on read: %s", strerror(errno));
			break;
		}
		msg(MSG_DEBUG, "%d characters have been read", r);
		if (response[0] == '5' || response[0] == '4')  // if negative or temporary neg. response
			return 0;
		memset(response, 0, sizeof(response));
	}
	return 1;
}

void fetch_response(const connection_t *conn, char *filename, int mode) {
	struct sockaddr_in sa;
	char response[MAXLINE];
	char new_filename[25];
	int file_fd, conn_fd, r, w, anonym_login = 0;

	memset(response, 0, sizeof(response));
	memset(&sa, 0, sizeof(sa));

	msg(MSG_DEBUG, "check if we already know the answer from the remote side.");

	if ( (file_fd = open(filename, O_WRONLY | O_CREAT | O_EXCL | O_SYNC, S_IRUSR | S_IWUSR)) == -1) {
		msg(MSG_ERROR, "cant create file %s: %s, ", filename, strerror(errno));
		if (errno == EEXIST) 
			msg(MSG_ERROR, "but the file exists already");
		else {
			msg(MSG_ERROR, ": %s. We cant fetch the response", strerror(errno));
			exit(1);
		}
	}
	else {	
		msg(MSG_DEBUG, "file %s has been opened for writing", filename);

		conn_fd = Socket(AF_INET, SOCK_STREAM, 0);
	
		sa.sin_family = AF_INET;
		sa.sin_port = conn->dport;
		Inet_pton(AF_INET, conn->dest, &sa.sin_addr);
	
		if (Connect(conn_fd, (SA *) &sa, sizeof(sa)) == 0) {
			msg(MSG_DEBUG, "now we start reading the welcome message");
			while (readable_timeout(conn_fd, TIMEOUT_READ_FROM_CONN)) {
				r = read(conn_fd, response, MAXLINE-1);
				if (r == 0) {
					msg(MSG_DEBUG, "connection has been closed");
					break;
				} else {
					msg(MSG_ERROR, "Error on read: %s", strerror(errno));
					break;
				}
				msg(MSG_DEBUG, "%d characters have been read", r);
				if ((w = write(file_fd, response, r))) {
					msg(MSG_DEBUG, "and %d characters have been written to the file", w);
				}
				memset(response, 0, sizeof(response));
			}
			if (mode == half_proxy)
				anonym_login = try_anonymous_login(conn_fd);
	
			fsync(file_fd);
			Close_conn(conn_fd, "connection for fetching response");
		} else {
			msg(MSG_ERROR, "Error on Conect: %s", strerror(errno));
		}
		Close(file_fd);

		if (anonym_login) {
			msg(MSG_DEBUG, new_filename, "%s_ano", filename);
			rename(filename, new_filename);
		}
			
		msg(MSG_DEBUG, "file is closed again\n");
	}
}

void connect_to_orig_target(int *orig_targetservicefd, const connection_t *conn, int *irc_connected_flag) {
	struct sockaddr_in servaddr;

	*orig_targetservicefd = Socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_port        = htons((uint16_t)conn->dport);
	Inet_pton(AF_INET, conn->dest, &servaddr.sin_addr);
	//ptr_val = &val;

	if (Connect(*orig_targetservicefd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
		msg(MSG_ERROR, "connenction to orig_targetservice could not be established: %s", strerror(errno));
		Close_conn(*orig_targetservicefd, "connenction to orig_targetservice could not be established");
		*irc_connected_flag = 0;
		return;
	}
	else
		*irc_connected_flag = 1;

	return;
}

void set_so_linger(int sd) {
	struct linger l;
	
	l.l_onoff = 1; 
	l.l_linger = 0;
	
	if (setsockopt(sd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
		msg(MSG_ERROR, "could not set SO_LINGER: %s", strerror(errno));
		exit(1);
	}
}

void unset_so_linger(int sd) {
	struct linger l;
	
	l.l_onoff = 0; 
	l.l_linger = 0;
	
	if (setsockopt(sd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
		msg(MSG_ERROR, "could not unset SO_LINGER: %s", strerror(errno));
		exit(1);
	}
}

int fetch_banner(int mode, const connection_t *connection, int *anonym_ftp, char *payload) {
	struct sockaddr_in	targetservaddr;
	int			targetservicefd,
				fd, r;
	char			full_path[256],
				full_path_ano[256];

	r = 0;
	memset(full_path_ano, 0, sizeof(full_path_ano));
	memset(full_path, 0, sizeof(full_path));
	//sprintf(full_path, "%s/%s:%d", RESPONSE_COLLECTING_DIR, connection->dest, connection->dport);
	//sprintf(full_path_ano, "%s/%s:%d_ano", RESPONSE_COLLECTING_DIR, connection->dest, connection->dport);

	if ( (fd = open(full_path, O_RDONLY)) == -1) {
		msg(MSG_ERROR, "could not open %s readonly: %s", full_path, strerror(errno));
		if ( (fd = open(full_path_ano, O_RDONLY)) == -1) {
			msg(MSG_ERROR, "could not open %s readonly: %s", full_path_ano, strerror(errno));
			if (mode == half_proxy) {
				msg(MSG_DEBUG, "no stored response available\nso we connect to the original server...");
		
				targetservicefd = Socket(AF_INET, SOCK_STREAM, 0);
		
				bzero(&targetservaddr, sizeof(targetservaddr));
				targetservaddr.sin_family = AF_INET;
				targetservaddr.sin_port = htons((uint16_t)connection->dport);
				Inet_pton(AF_INET, connection->dest, &targetservaddr.sin_addr);
		
				if (Connect(targetservicefd, (SA *) &targetservaddr, sizeof(targetservaddr)) < 0)
					msg(MSG_DEBUG, "cant connect to targetservice: %s", strerror(errno));
				else {
					msg(MSG_DEBUG, "now we are connected to the original destination\n");
					while (readable_timeout(targetservicefd, 2)) {
						if ((r = read(targetservicefd, payload, MAXLINE-1)) <= 0) {
							msg(MSG_ERROR, "no characters have been read from client: %s", strerror(errno));
							break;
						}
						else {
							msg(MSG_DEBUG, "we got our payload from the serverside\n");
							//write_to_nonexisting_file(payload, connection, RESPONSE_COLLECTING_DIR);
							break;
						}
					}
			
					if (try_anonymous_login(targetservicefd)) {
						rename(full_path, full_path_ano);
						*anonym_ftp = 1;
					}
					else {
						*anonym_ftp = 0;
					}
			
					Close_conn(targetservicefd, "connection to original destination, because we got the banner");
				}
			}
			else 
				msg(MSG_DEBUG, "cant fetch banner since we are not in half-proxy mode and we have no reponse stored locally\n");
			return r;
		}
		else {   // if there is an anonym response file
			msg(MSG_DEBUG, "successfully opened %s for readonly\n", full_path_ano);
			*anonym_ftp = 1;
		}
	}
	else { // if there is a non-anonym ftp response file
		msg(MSG_DEBUG, "successfully opened %s for readonly\n", full_path);
		*anonym_ftp = 0;
	}
	msg(MSG_DEBUG, "now reading from file\n");
	if (-1 == read(fd, payload, MAXLINE-1)) {
		msg(MSG_ERROR, "Error on file read!");
		r = 0;
	}
	Close(fd);
	return r;
}

int get_irc_banner(const connection_t *conn, char *payload) {
	struct sockaddr_in 	servaddr;
	int			irc_server_fd;
	size_t			r, w;
	char			*ptr,
				response[MAXLINE],
				filename[MAXLINE];
	
	irc_server_fd = Socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_port        = htons((uint16_t)conn->dport);
	Inet_pton(AF_INET, conn->dest, &servaddr.sin_addr);

	if (Connect(irc_server_fd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
		msg(MSG_ERROR, "connenction to real irc server could not be established: %s\n", strerror(errno));
		Close_conn(irc_server_fd, "connenction to real irc server could not be established");
		return 0;
	}
	
	msg(MSG_DEBUG, "now we are connected to the real irc server\ndest_ip: %s\ndest_port: %d\npayload: %s", conn->dest, conn->dport, payload);
	
	ptr = payload;
	r = strlen(payload);

	while (r > 0 && (w = write(irc_server_fd, ptr, r)) > 0) {
		ptr += w;
		r -= w;
	}

	msg(MSG_DEBUG, "payload has been sent there\n");	

	while (readable_timeout(irc_server_fd, 3)) {
		if ((read(irc_server_fd, response, sizeof(response)-1)) <= 0) {
			msg(MSG_DEBUG, "no characters have been read from client");
			Close_conn(irc_server_fd, "connection to real irc server");
			return 0;
		}
		else {
			msg(MSG_DEBUG, "we received irc response from the serverside\n");
			snprintf(filename, MAXLINE-1, "%s:%d", conn->dest, conn->dport);
			//write_to_file(response, filename, RESPONSE_COLLECTING_DIR);
			msg(MSG_DEBUG, "and wrote the response to some file\n");
			Close_conn(irc_server_fd, "connection to real irc server");
			ptr = strchr(response, ' ');
			*ptr = 0;
//			snprintf(irc_fake_string, sizeof(irc_fake_string),"%s", response);
//			printf("after initialising irc_fake_string it is: %s\n", irc_fake_string);
			return 1;
		}
	}
	Close_conn(irc_server_fd, "connection to real irc server");
	return 0;
}


