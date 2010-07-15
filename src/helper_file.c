#include "helper_file.h"
#include "semaphore.h"
#include "dispatching.h"
#include "wrapper.h"
#include "helper_net.h"
#include "msg.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>


int create_index_file(char* filename) {
	FILE *fd;

	if ((fd = fopen(filename,"wx")) != NULL)
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
	msg(MSG_DEBUG,"Entered create path: %s",path);
	path++;

	if ((new_dir_name = strsep(&path, "/")) != NULL) {
		do {
		//while((new_dir_name = strsep(&path, "/")) != NULL) {
			msg(MSG_DEBUG, "new_dir_name is: %s and path is: %s", new_dir_name, path);
			if (chdir(new_dir_name) != 0) {
				msg(MSG_DEBUG, "create folder: %s", new_dir_name);
				if (strcmp(new_dir_name,"")==0 || strcmp(new_dir_name," ") == 0) 
	    			{
				// we do not have to create a folder
				break;
				}
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

void build_tree(const connection_t *conn, const char *cmd_str) {
	char base_dir[MAX_PATH_LENGTH]; // htdocs directory of webserver
	char full_path[MAX_PATH_LENGTH];
	char *filename = "index.html";  // filename of the dummyfile to create
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

	char* ptrToHttpArgs = NULL;
	char args[MAX_PATH_LENGTH];
	char* ptrArgs[100];	
	int countArgs = 0;
	

	ptrToHttpArgs = strchr(path,'?');
	// check if the requested location has the form /bla/x?a1=4&a2=)
	if (ptrToHttpArgs != NULL) {
		
		ptrToHttpArgs++; // now we are at the position of the first argument
		msg(MSG_DEBUG,"Args found: \n%s",ptrToHttpArgs);
		strcpy(args,ptrToHttpArgs); 
		tmp1 = strchr(args, ' ');
		*tmp1 = '\0'; // end of string
		char* tmpArg = NULL;

		tmpArg = strtok(args, "&");
		while(tmpArg!=NULL) 
		{
			msg(MSG_DEBUG,"arg length %d: %s\n",strlen(tmpArg),tmpArg);
			ptrArgs[countArgs] = tmpArg;

	/*		char singleArg[strlen(tmpArg)+1]; // we need additional character for NULL
			strcpy(singleArg,tmpArg);

			char* separateArgs;
			separateArgs = strtok(singleArg, "=");
			msg(MSG_DEBUG,"argName: %s\n",separateArgs);
			separateArgs = strtok(NULL, "=");
			msg(MSG_DEBUG,"argValue: %s\n",separateArgs);
			
			
	*	char* sepharateArgs;
			char tmparg2[strlen(tmparg+1)];
			strcpy(tmparg2,tmparg);
			separateArgs = strtok(tmparg2, "=");
			argsNames[countArgs] = separateArgs;
			separateArgs = strtok(NULL, "=");
			argsValues[countArgs] = separateArgs; */
			tmpArg = strtok(NULL,"&");
			countArgs++;
		}

		msg(MSG_DEBUG,"Finished Parsing Args: %d",countArgs);
		        int j;
			        for (j = 0; j < countArgs; j++) {
				         char singleArg[strlen(ptrArgs[j])+1]; // we need additional character for NULL
					          strcpy(singleArg,ptrArgs[j]);
						           char* separateArgs;
							            separateArgs = strtok(singleArg, "=");
								             msg(MSG_DEBUG,"argName: %s\n",separateArgs);
									              separateArgs = strtok(NULL, "=");
										               msg(MSG_DEBUG,"argValue: %s\n",separateArgs);

											               }
                tmp1 = strchr(path, '?');
		tmp1 = strchr(tmp1, '?');

	}
         
       
     
      
    
   
	else {
	                tmp1 = strchr(path, ' ');
			tmp1 = strchr(tmp1, ' ');

	}

/*

	if (conn->app_proto == FTP || conn->app_proto == FTP_anonym)
		tmp1 = strchr(path, '\r');
	else if (conn->app_proto == HTTP) {

		tmp1 = strchr(path, ' ');
		tmp1 = strchr(tmp1, ' ');
	}*/


	if (tmp1 == NULL)
		return;
	
	tmp1--; //tmp1 points now to the ending position of the path


	
	/*if (*tmp1 != '/') {
		msg(MSG_DEBUG,"konkateniere noch ein / hinten dran");
		tmp1++;
		*tmp1 = '/';
	}*/
	
	if (*tmp1 == '/') {
	// requested location has the form /folder/folder2/ -> reply with index.html (default filename)
	}
	else {
	// requested location has the form /folder/filename -> Create folders and file with "filename"
	int filenameLength = 0;
	char customFileName[MAX_PATH_LENGTH];
	filename = customFileName;

	// go back from filename end until we find the first '/'
		while (*tmp1 != '/') {
		tmp1--;
		filenameLength ++;
		}

	strncpy(filename,tmp1+1,filenameLength); // now set the filename
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

	msg(MSG_DEBUG,"calling create path tree with: \n%s",path);
	if (create_path_tree(path, user_id, group_id) == -1) {
		msg(MSG_ERROR, "error during tree creation\n");
		return;
	}

	if (conn->app_proto == HTTP)
		create_index_file(filename);

	if (chdir(saved_cwd) == -1) {
		msg(MSG_ERROR, "could not change back to %s: %s", saved_cwd, strerror(errno));
		return;
	}
	return;
}

