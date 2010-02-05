#include "helper_file.h"
#include "semaphore.h"
#include "dispatching.h"
#include "wrapper.h"
#include "helper_net.h"
#include "proto_ident.h"
#include "payload_alter_log.h"
#include "msg.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>


int create_index_file() {
	FILE *fd;

	if ((fd = fopen("index.html", "wx")) != NULL)
		fprintf(fd, "<html><body><h1>ur file has been removed. so get out of here!</h1></body></html>\n");
	else if (errno == EEXIST) {
		msg(MSG_ERROR, "index.html already exists\n");
		return -1;
	}
	else {
		msg(MSG_ERROR, "could not create the index.html: %s", strerror(errno));
		return -1;
	}
	
	fflush(fd);
	Close_file(fd);
	return 0;
}

int create_path_tree(char *path, int user_id, int group_id) {
	//int status = 0;
	//char *tmp_ptr;
	char *new_dir_name;

	path++;

	if ((new_dir_name = strsep(&path, "/")) != NULL) {
		do {
		//while((new_dir_name = strsep(&path, "/")) != NULL) {
			msg(MSG_DEBUG, "new_dir_name is: %s and path is: %s", new_dir_name, path);
			if (chdir(new_dir_name) != 0) {
				msg(MSG_DEBUG, "create folder: %s", new_dir_name);
				if (mkdir(new_dir_name, 0755) == -1) {
					msg(MSG_ERROR, "could not create directory with name %s: %s", new_dir_name, strerror(errno));
					return -1;
				}
				if (chown(new_dir_name, user_id, group_id) == -1) {
					msg(MSG_ERROR, "could not change ownership of %s: %s", new_dir_name, strerror(errno));
					return -1;
				}
				if (chdir(new_dir_name) == -1) {
					msg(MSG_ERROR, "could not change into directory with name %s: %s", new_dir_name, strerror(errno));
					return -1;
				}
			} 
			else {
				msg(MSG_DEBUG, "we changed into %s", new_dir_name);
			}
			new_dir_name = strsep(&path, "/");
		} while (path != NULL);
	}
	return 0;
}

void build_tree(const connection_t *conn, char *cmd_str) {
	char base_dir[MAX_PATH_LENGTH];
	char full_path[MAX_PATH_LENGTH];
	//char tmp2[MAX_PATH_LENGTH];
	char *path;
	char *tmp1 = NULL;
	char saved_cwd[MAX_PATH_LENGTH];
	int user_id, group_id;

	strncpy(full_path, cmd_str, MAX_PATH_LENGTH-1);
	memset(base_dir, 0, sizeof(base_dir));
	
	full_path[MAX_PATH_LENGTH-1] = 0;

	if (conn->app_proto == HTTP) {    // if we are in a http connection
		strncpy(base_dir, HTTP_BASE_DIR, sizeof(base_dir)-1);
		user_id = 0;	//FIXME
		group_id = 0;		
//		user_id = 33;
//		group_id = 33;
	}
	else if (conn->app_proto == FTP || conn->app_proto == FTP_anonym) {    // if we are in a ftp connection
		strncpy(base_dir, FTP_BASE_DIR, sizeof(base_dir)-1);
		user_id = 111;	//FIXME
		group_id = 65534;
	}
	else {
		msg(MSG_ERROR, "no filesystem structure creation for this protocol\n");
		return;
	}

	path = strchr(full_path, '/');

	if (path == NULL)
		return;

	if (conn->app_proto == FTP || conn->app_proto == FTP_anonym)
		tmp1 = strrchr(path, '\r');
	else if (conn->app_proto == HTTP) {
		tmp1 = strchr(path, ' ');
		tmp1 = strchr(tmp1, ' ');
	}

	if (tmp1 == NULL)
		return;
	
	tmp1--;
	if (*tmp1 != '/') {
		tmp1++;
		*tmp1 = '/';
	}
	tmp1++;
	*tmp1 = 0;

	if (getcwd(saved_cwd, sizeof(saved_cwd)) == NULL) {
		msg(MSG_ERROR, "getcwd failed: %s", strerror(errno));
		return;
	}

	if (chdir(base_dir) == -1) {
		msg(MSG_ERROR, "could not change to %s: %s", base_dir, strerror(errno));
		return;
	}

	if (create_path_tree(path, user_id, group_id) == -1) {
		msg(MSG_ERROR, "error during tree creation\n");
		return;
	}

	if (conn->app_proto == HTTP)
		create_index_file();

	if (chdir(saved_cwd) == -1) {
		msg(MSG_ERROR, "could not change back to %s: %s", saved_cwd, strerror(errno));
		return;
	}
	return;
}

void append_to_file(char *str, const connection_t *connection, char *base_dir) {
	char full_path[256], *ptr;
	int fd, w, r;

	sprintf(full_path, "%s/%s:%d", base_dir, connection->dest, connection->dport);

	msg(MSG_DEBUG, "now we open %s for appending the string: %s", full_path, str);

	semaph_alloc();
	if ( (fd = open(full_path, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
		msg(MSG_ERROR, "cant open file %s for appending data: %s ", full_path, strerror(errno));
		return;
	}

	r = strlen(str);
	ptr = str;

	while (r > 0 && (w = write(fd, ptr, r)) > 0) {
		ptr += w;
		r -= w;
	}

	Close(fd);	
	semaph_free();
	return;
}

int write_to_nonexisting_file(char *str, const connection_t *connection, char *base_dir) { 
	// returns 1 on success, 0 if file exists, and -1 if something goes totally wrong ;-)
	char full_path[256], *ptr;
	int fd, w, r;

	sprintf(full_path, "%s/%s:%d", base_dir, connection->dest, connection->dport);

	msg(MSG_DEBUG, "now we open %s for appending the string: %s", full_path, str);


	if ( (fd = open(full_path, O_WRONLY | O_CREAT | O_EXCL | O_SYNC, S_IRUSR | S_IWUSR)) == -1) {
		msg(MSG_ERROR, "cant create file %s, ", full_path);
		if (errno == EEXIST) {
			msg(MSG_ERROR, "but the file exists already");
			return 0;
		}
		else {
			fprintf(stderr, "so we cant fetch the response: %s\n", strerror(errno));
			return -1;
		}
	}

	r = strlen(str);
	ptr = str;

	while (r > 0 && (w = write(fd, ptr, r)) > 0) {
		ptr += w;
		r -= w;
	}

	Close(fd);	
	return 1;
}

int write_to_file(char *str, char *filename, char *base_dir) { 
	// returns 1 on success, and -1 if file can not be created
	char full_path[256], *ptr;
	int fd, w, r;

	sprintf(full_path, "%s/%s", base_dir, filename);

	msg(MSG_DEBUG, "now we open %s for appending the string: %s", full_path, str);


	if ( (fd = creat(full_path, S_IRUSR | S_IWUSR)) == -1) {
		msg(MSG_ERROR, "can not create file %s: %s, ", full_path, strerror(errno));
		return -1;
	}

	r = strlen(str);
	ptr = str;

	while (r > 0 && (w = write(fd, ptr, r)) > 0) {
		ptr += w;
		r -= w;
	}

	Close(fd);	
	return 1;
}

void creat_file(const connection_t *connection, char *base_dir) { 
	// returns 1 on success and -1 if something goes wrong ;-)
	int 	fd;
	char	full_path[256];

	sprintf(full_path, "%s/%s:%d", base_dir, connection->dest, connection->dport);

	msg(MSG_DEBUG, "now we create the file: %s:%d", connection->dest, connection->dport);

	if ( (fd = creat(full_path, S_IRUSR | S_IWUSR)) == -1) {
		msg(MSG_ERROR, "cant create file %s: %s", full_path, strerror(errno));
		return;
	}
	else
		

	Close(fd);
	msg(MSG_DEBUG, "...done");
	return;
}

