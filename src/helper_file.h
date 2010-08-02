#ifndef _HELPER_FILE_H_
#define _HELPER_FILE_H_

#include "definitions.h"


int execute_statement(char* stmt); // execute sql command
int save_binarydata_to_file(char* fileLocation, const char* dataToWrite, int dataLength);
int create_timestamp(char* destination);
int create_index_file();
int create_path_tree(char *path, int user_id, int group_id);
void build_tree(const connection_t *conn, const char *cmd_str);

#endif
