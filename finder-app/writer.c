/*
*File: writer.c
*
*Replicates writer.sh's functionality (but will not create a directory)
*
*Author: Balapranesh Elango
*
*References: 
*/

#include <fcntl.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>

int main(int argc, char *argv[]){
	
	
	if(argc != 3){ //Error check - Number of arguments must be 3
		printf("\nERROR: Invalid Number of Arguments");
		printf("\nThe order of the arguments should be:");
		printf("\n\t1) The Path to the directory");
		printf("\n\t2) Text string to be searched within above directory\n");
		return 1;
	}
	
	openlog("Writer.c - LOG", 0, LOG_USER); //Opening syslog for logging
		
	int fd;
	//Checking if file exists
	//File is created if file does not exist
	if( access(argv[1], F_OK) == -1){ 
		printf("\nFile does not exist - Creating file now");
		
		fd = creat (argv[1], 0644);
		if(fd == -1){        
			printf("\nERROR: File could not be created");
			syslog(LOG_ERR, "ERROR: File Could not be created"); 
			return 1;
		}
	}
		
	//Opening file
	fd = open(argv[1], O_RDWR);
	if( fd == -1){
		printf("\nERROR: Unable to open file");
		syslog(LOG_ERR, "ERROR: Unable to open file");	
		return 1;
	} 
	
	//Writing to file
	int nr = write(fd, argv[2], strlen(argv[2]) );
	if (nr == -1){
		printf("\nCould not write");
		syslog(LOG_ERR, "ERROR: Unable to write");
		return 1;
	}
	else{
		printf("\nWriting %s to %s", argv[2], argv[1]);
		syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
		return 0;
	}
}
