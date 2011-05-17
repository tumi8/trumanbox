#ifndef _HELPER_NET_H_
#define _HELPER_NET_H_

#include <common/definitions.h>

int readable_timeout(int fd, int sec);
int try_anonymous_login(int conn_fd);
void fetch_response(const connection_t *conn, char *filename, int mode);
void connect_to_orig_target(int *orig_targetservicefd, const connection_t *conn, int *irc_connected_flag);
void set_so_linger(int sd);
void unset_so_linger(int sd);
int fetch_banner(int mode, const connection_t *connection, int *anonym_ftp, char *payload);
int get_irc_banner(const connection_t *conn, char *payload);

#endif
