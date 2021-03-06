#include <sys/time.h>
#include <time.h>
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

int create_timestamp(char* destination) {
        
	struct timeval currentStart;
	gettimeofday(&currentStart,0);
	if (sprintf(destination,"%ld-%ld",currentStart.tv_sec,currentStart.tv_usec) < 0) 
		return 0;

	return 1;
		
}


/* Saves the data given as second argument into a file with an unique filename in the folder specified in the first argument.
 * returns 0 on failure, 1 on success
 * */
int save_binarydata_to_file_generic(char* fileLocation, const char* dataToWrite, int dataLength,const char* mode) {
	size_t count;
	FILE * pFile;
	pFile = fopen ( fileLocation , mode );
	
	
	if(pFile == NULL) {
		msg(MSG_FATAL,"Error opening %s",fileLocation);
		return 0;
	}
	else  {
		count = fwrite (dataToWrite , 1,dataLength , pFile );
		if (count != dataLength)  
			msg(MSG_FATAL,"Could not write %d bytes, wrote only %d",dataLength,count);
		fclose (pFile);

	}
	return 1;

}


int save_binarydata_to_file(char* fileLocation, const char* dataToWrite, int dataLength) {
	return save_binarydata_to_file_generic(fileLocation,dataToWrite,dataLength,"wb");
}

int append_binarydata_to_file(char* fileLocation, const char* dataToWrite, int dataLength) {
	return save_binarydata_to_file_generic(fileLocation,dataToWrite,dataLength,"ab");
}



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

/*
int create_path_tree_with_file(char* file_location, char* httplocation, int user_id, int_group_id) {
	return 0;
}
*/
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




/*
 * This method extracts the path and the filename from the request
 * destpath: Pointer to the location where the path should be stored
 * destfilename: Pointer to the location where the filename should be stored
 * request: Location requested in a FTP/HTTP request, we need it to be in the form of something like that: '/index/bla/malware.cgi?auth=true&cc=false', '/index.html' or '/i/have/no/filename/'
 * */
void extract_dir_and_filename_from_request(char* destpath, char* destfilename, char* request) {
	
	bzero(destpath,MAX_PATH_LENGTH);
	bzero(destfilename,MAX_PATH_LENGTH);
	char* ptrToHttpArgs =  strchr(request,'?');
	if (ptrToHttpArgs != NULL) {
			
		// requested location has the form /bla/x?a1=4&a2=8

		char* beginFilename = ptrToHttpArgs;
		while (*beginFilename !=  '/') {
			beginFilename --;
		}
		strncpy(destfilename,beginFilename+1,ptrToHttpArgs-beginFilename-1);
		destfilename[ptrToHttpArgs-beginFilename-1] = 0;
	}
	else {
		int len = strlen(request);
		char* endFilename = &request[len];
		char* beginFilename = endFilename;
		while (*beginFilename != '/') {
			beginFilename --;
		}
		strcpy(destfilename,beginFilename+1);
	}
	int len = strlen(request);
	char* endFilename = &request[len];
	char* beginFilename = endFilename;
	while (*beginFilename != '/') {
		beginFilename --;
	}
	beginFilename ++;
	strncpy(destpath,request,beginFilename-request);
	destpath[beginFilename-request] = 0;

	msg(MSG_DEBUG,"extracted path: '%s', filename '%s'",destpath,destfilename);
}



int copy(char* src, char* dest) {
	FILE *from, *to;
	msg(MSG_DEBUG,"we would like to copy '%s' to '%s'",src,dest);
	char line[512];
	int bytes = 0;
	/* open source file */
	  if((from = fopen(src, "rb"))==NULL) {
	    	msg(MSG_FATAL,"Cannot open source file - Error: %s",strerror(errno));
	   	 return -1;
	  }

	  /* open destination file */
	  if((to = fopen(dest, "wb"))==NULL) {
	    	msg(MSG_FATAL,"Cannot open destination file - Error: %s",strerror(errno));
	    	return -1;
	  }

	  /* copy the file */
		
	while ((bytes = fread(line,1,512,from)) > 0) {
		if (fwrite(line,1,bytes,to) <= 0) {
			msg(MSG_FATAL,"Error writing destination file");
			return -1;
		}

	}

	if(fclose(from)==EOF) {
		msg(MSG_FATAL,"Error closing source file.");
	    	return -1;
	}

	if(fclose(to)==EOF) {
	    	msg(MSG_DEBUG,"Error closing destination file.");
	    	return -1;
	}

	return 0;
}

void build_tree(const connection_t *conn, const char *cmd_str) {
	char base_dir[MAX_PATH_LENGTH]; // htdocs directory of webserver
	char full_path[MAX_PATH_LENGTH]; // full path requested, e.g. "/img/src/docs/malware.cgi?cc=149131&year=2010" 
	char *path;
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

	/*if (conn->app_proto == HTTP)
		create_index_file(filename);
*/
	if (chdir(saved_cwd) == -1) {
		msg(MSG_ERROR, "could not change back to %s: %s", saved_cwd, strerror(errno));
		return;
	}
	return;
}

