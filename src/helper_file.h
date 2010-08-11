#ifndef _HELPER_FILE_H_
#define _HELPER_FILE_H_

#include "definitions.h"
#include <postgresql/libpq-fe.h>


//int execute_statement(char* stmt); // execute sql command
//PGconn* connect_to_db();
int execute_nonquery_statement(char* stmt); 
int execute_statement(char* stmt);
char* execute_query_statement_singlevalue(char* stmt);
int save_binarydata_to_file_generic(char* fileLocation, const char* dataToWrite, int dataLength, const char* mode);
int save_binarydata_to_file(char* fileLocation, const char* dataToWrite, int dataLength);
int append_binarydata_to_file(char* fileLocation, const char* dataToWrite, int dataLength);
int create_timestamp(char* destination);
int create_index_file();
int create_path_tree(char *path, int user_id, int group_id);
void build_tree(const connection_t *conn, const char *cmd_str);

#endif
