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

#include "definitions.h"
#include "semaphore.h"

void create_tmp_folders();
void change_to_tmp_folder();
int create_index_file();
int create_path_tree(char *path, int user_id, int group_id);
void build_tree(const connection_t *conn, char *cmd_str);
void append_to_file(char *str, const connection_t *connection, char *base_dir);
int write_to_nonexisting_file(char *str, const connection_t *connection, char *base_dir);
int write_to_file(char *str, char *filename, char *base_dir);
void creat_file(const connection_t *connection, char *base_dir);
