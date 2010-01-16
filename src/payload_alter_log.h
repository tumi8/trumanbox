#include <stdio.h>
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
#include <ctype.h>

#include "definitions.h"

void print_payload(const u_char *payload, int len);
int print_timestamp(const connection_t *connection, char *protocol_dir);
void delete_row_starting_with_pattern(char *datastring, const char *pattern);
void content_substitution_and_logging_stc(const connection_t *conn, char *data, int *data_len);
void content_substitution_and_logging_cts(const connection_t *conn, char *data, int *data_len);

