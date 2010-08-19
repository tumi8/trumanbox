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
#include <postgresql/libpq-fe.h>




int execute_statement(char* stmt) {
	return execute_nonquery_statement(stmt);
}


int execute_nonquery_statement(char* stmt) {
 
	int result = 0;
	PGconn* psql  = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'trumanlogs' user = 'trumanbox' password = 'das$)13x!#+23' connect_timeout = '10'");
	if (!psql) {
                msg(MSG_FATAL,"libpq error : PQconnectdb returned NULL.\n\n");
                return result;
        }

        if (PQstatus(psql) != CONNECTION_OK) {
                msg(MSG_FATAL,"libpq error: PQstatus(psql) != CONNECTION_OK\n\n");
                return result;
        }
	
        PGresult *res = PQexec(psql, stmt);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
                {
                msg(MSG_FATAL,"ERROR: %s",PQresultErrorMessage(res));
                result = 0;
                }
                else
                {
                result = 1; // success
      	}
	
	PQfinish(psql);
	return result;
}


int execute_query_statement_singlevalue(char* dst, char* stmt) {
	int result = 0;
       	PGconn* psql = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'trumanlogs' user = 'trumanbox' password = 'das$)13x!#+23' connect_timeout = '10'");
	if (!psql) {
                msg(MSG_FATAL,"libpq error : PQconnectdb returned NULL.\n\n");
                return result;
        }

        if (PQstatus(psql) != CONNECTION_OK) {
                msg(MSG_FATAL,"libpq error: PQstatus(psql) != CONNECTION_OK\n\n");
                return result;
        }

	PGresult *res = PQexec(psql, stmt);
                
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		{
		msg(MSG_FATAL,"Status: %s",PQresStatus(PQresultStatus(res)));
		msg(MSG_FATAL,"%s",PQresultErrorMessage(res));
		}
	else {
		if (PQntuples(res) > 0 && PQnfields(res) > 0) {
		result = 1;
		char* value =  PQgetvalue(res,0,0);
		msg(MSG_DEBUG,"we have value: %s",value);
		strcpy(dst,value);
		}
		
	 }

	
	PQfinish(psql);


	return result;
}





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
	
	msg(MSG_DEBUG,"LengthToWrite: %d, is > 0: %d",dataLength,dataLength>0);
	
	if(pFile == NULL) {
		msg(MSG_FATAL,"Error opening %s",fileLocation);
		return 0;
	}
	else  {
		count = fwrite (dataToWrite , dataLength, 1 , pFile );
		msg(MSG_DEBUG,"wrote %zu item: %s",count,fileLocation);
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

	bzero(destpath,sizeof(destpath));
	bzero(destfilename,sizeof(destfilename));
	msg(MSG_DEBUG,"sizeof destpath and destfilename: %d %d",sizeof(destpath),sizeof(destfilename));
	char* ptrToHttpArgs =  strchr(request,'?');
	char filename[MAX_PATH_LENGTH];
	if (ptrToHttpArgs != NULL) {
			
		// requested location has the form /bla/x?a1=4&a2=8

		char* beginFilename = ptrToHttpArgs;
		while (*beginFilename !=  '/') {
			beginFilename --;
		}
		strncpy(filename,beginFilename+1,ptrToHttpArgs-beginFilename-1);
		filename[ptrToHttpArgs-beginFilename-1] = 0;
	}
	else {
		int len = strlen(request);
		char* endFilename = &request[len];
		char* beginFilename = endFilename;
		while (*beginFilename != '/') {
			beginFilename --;
		}
		strcpy(filename,beginFilename+1);
		
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

