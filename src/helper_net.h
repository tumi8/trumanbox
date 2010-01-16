#include "definitions.h"
#include "semaphore.h"

#include "dispatching.h"
#include "wrapper.h"
#include "proto_ident.h"
#include "payload_alter_log.h"
#include "helper_file.h"

int readable_timeout(int fd, int sec);
int try_anonymous_login(int conn_fd);
void fetch_response(const connection_t *conn, char *filename, int mode);
void connect_to_orig_target(int *orig_targetservicefd, const connection_t *conn, int *irc_connected_flag);
void set_so_linger(int sd);
void unset_so_linger(int sd);
int fetch_banner(int mode, const connection_t *connection, int *anonym_ftp, char *payload);
int get_irc_banner(const connection_t *conn, char *payload);
int parse_conntrack(connection_t *conn);
