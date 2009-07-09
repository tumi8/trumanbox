#include "helper_file.h"
#include "dispatching.h"
#include "wrapper.h"
#include "helper_net.h"
#include "payload_ident.h"
#include "payload_alter_log.h"

void create_tmp_folders() {
	int preexisting_folders = 0;

	if (mkdir(TMP_TRUMANBOX, PERMS) < 0) {
		if (errno == EEXIST)
			preexisting_folders = 1;
		else {
			printf("%s could not be created!\n", TMP_TRUMANBOX);
			exit(1);
		}
	}

	if (mkdir(RESPONSE_COLLECTING_DIR, PERMS) < 0) {
		if (errno == EEXIST)
			preexisting_folders = 1;
		else {
			printf("%s could not be created!\n", RESPONSE_COLLECTING_DIR);
			exit(1);
		}
	}	

	if (mkdir(FTP_COLLECTING_DIR, PERMS) < 0) {
		if (errno == EEXIST)
			preexisting_folders = 1;
		else {
			printf("%s could not be created!\n", FTP_COLLECTING_DIR);
			exit(1);
		}
	}

	if (mkdir(IRC_COLLECTING_DIR, PERMS) < 0) {
		if (errno == EEXIST)
			preexisting_folders = 1;
		else {
			printf("%s could not be created!\n", IRC_COLLECTING_DIR);
			exit(1);
		}
	}

	if (mkdir(SMTP_COLLECTING_DIR, PERMS) < 0) {
		if (errno == EEXIST)
			preexisting_folders = 1;
		else {
			printf("%s could not be created!\n", SMTP_COLLECTING_DIR);
			exit(1);
		}
	}

	if (mkdir(HTTP_COLLECTING_DIR, PERMS) < 0) {
		if (errno == EEXIST)
			preexisting_folders = 1;
		else {
			printf("%s could not be created!\n", HTTP_COLLECTING_DIR);
			exit(1);
		}
	}

	if (mkdir(DUMP_FOLDER, PERMS) < 0) {
		if (errno == EEXIST)
			preexisting_folders = 1;
		else {
			printf("%s could not be created!\n", DUMP_FOLDER);
			exit(1);
		}
	}

	if (preexisting_folders)
		printf("\t\t\tProbably there are preexisting logfiles. Those do not\n \
			affect the TrumanBox, but migh disturb during later\n \
			analyses.\n\n\n");
	
	return;
}

void change_to_tmp_folder() {
	if (chdir(RESPONSE_COLLECTING_DIR) < 0) {
		printf("cannot change working dir to: %s :-(\n", RESPONSE_COLLECTING_DIR);
		exit(1);
	}
}

int create_index_file() {
	FILE *fd;

	if ((fd = fopen("index.html", "wx")) != NULL)
		fprintf(fd, "<html><body><h1>ur file has been removed. so get out of here!</h1></body></html>\n");
	else if (errno == EEXIST) {
		printf("index.html already exists\n");
		return -1;
	}
	else {
		printf("could not create the index.html\n");
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
			printf("new_dir_name is: %s\nand path is: %s\n", new_dir_name, path);
			if (chdir(new_dir_name) != 0) {
				printf("create folder: %s\n", new_dir_name);
				if (mkdir(new_dir_name, 0755) == -1) {
					fprintf(stderr, "could not create directory with name: %s\n", new_dir_name);
					return -1;
				}
				if (chown(new_dir_name, user_id, group_id) == -1) {
					fprintf(stderr, "could not change ownership of %s\n", new_dir_name);
					return -1;
				}
				if (chdir(new_dir_name) == -1) {
					fprintf(stderr, "could not change into directory with name: %s\n", new_dir_name);
					return -1;
				}
			} 
			else {
				printf("we changed into %s\n", new_dir_name);
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
	char *tmp1;
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
		fprintf(stderr, "no filesystem structure creation for this protocol\n");
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
		fprintf(stderr, "getcwd failed\n");
		return;
	}

	if (chdir(base_dir) == -1) {
		fprintf(stderr, "could not change to %s\n", base_dir);
		return;
	}

	if (create_path_tree(path, user_id, group_id) == -1) {
		fprintf(stderr, "error during tree creation\n");
		return;
	}

	if (conn->app_proto == HTTP)
		create_index_file();

	if (chdir(saved_cwd) == -1) {
		fprintf(stderr, "could not change back to %s\n", saved_cwd);
		return;
	}
	return;
}

void append_to_file(char *str, const connection_t *connection, char *base_dir) {
	char full_path[256], *ptr;
	int fd, w, r;

	sprintf(full_path, "%s/%s:%d", base_dir, connection->dest, connection->dport);

	printf("now we open %s for appending the string: %s\n", full_path, str);

	semaph_alloc();
	if ( (fd = open(full_path, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
		fprintf(stderr, "cant open file %s for appending data, ", full_path);
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

	printf("now we open %s for appending the string: %s\n", full_path, str);


	if ( (fd = open(full_path, O_WRONLY | O_CREAT | O_EXCL | O_SYNC, S_IRUSR | S_IWUSR)) == -1) {
		fprintf(stderr, "cant create file %s, ", full_path);
		if (errno == EEXIST) {
			fprintf(stderr, "but the file exists already\n");
			return 0;
		}
		else {
			fprintf(stderr, "so we cant fetch the response\n");
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

	printf("now we open %s for appending the string: %s\n", full_path, str);


	if ( (fd = creat(full_path, S_IRUSR | S_IWUSR)) == -1) {
		fprintf(stderr, "can not create file %s, ", full_path);
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

	printf("now we create the file: %s:%d\n", connection->dest, connection->dport);

	if ( (fd = creat(full_path, S_IRUSR | S_IWUSR)) == -1) {
		fprintf(stderr, "cant create file %s, ", full_path);
		return;
	}
	else
		

	Close(fd);
	printf("...done\n");
	return;
}

