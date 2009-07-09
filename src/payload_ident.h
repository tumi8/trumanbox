#include <string.h>
#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "definitions.h"

void protocol_identified_by_port(int mode, connection_t *conn, char *payload);
void protocol_identified_by_payload(int mode, connection_t *conn, int inconnfd, char *payload);
