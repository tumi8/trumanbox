#ifndef _PAYLOAD_ALTER_LOG_H_
#define _PAYLOAD_ALTER_LOG_H

#include "definitions.h"

void print_payload(const u_char *payload, int len);
int print_timestamp(const connection_t *connection, char *protocol_dir);
void delete_row_starting_with_pattern(char *datastring, const char *pattern);
void content_substitution_and_logging_stc(const connection_t *conn, char *data, int *data_len);
void content_substitution_and_logging_cts(const connection_t *conn, char *data, int *data_len);

#endif
