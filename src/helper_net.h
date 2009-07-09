#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "definitions.h"
#include "semaphore.h"

#include "dispatching.h"
#include "wrapper.h"
#include "payload_ident.h"
#include "payload_alter_log.h"
#include "helper_file.h"

int readable_timeout(int fd, int sec);
int try_anonymous_login(int conn_fd);
void fetch_response(const connection_t *conn, char *filename, int mode);
void connect_to_orig_target(int *orig_targetservicefd, const connection_t *conn, int *irc_connected_flag);
void set_so_linger(int sd);
void unset_so_linger(int sd);
void fetch_banner(int mode, const connection_t *connection, char *payload, int *anonym_ftp);
int get_irc_banner(const connection_t *conn, char *payload);
int parse_conntrack(connection_t *conn);
