/*
*File: writer.c
*
*Replicates writer.sh's functionality (but will not create a directory)
*
*Author: Balapranesh Elango
*
*References: 
* https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-*a-file-exists-in-c
*/

/*Header files*/
#include <fcntl.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>


/* #define Macros */

#define ERROR        (-1)
#define NUM_ARGS     (3)


/* Application entry point */
int main(int argc, char *argv[]){
	
	openlog("Writer.c - LOG", LOG_PID, LOG_USER); //Opening syslog for logging using LOG_USER facility
	
	if(argc != NUM_ARGS){ //Error check - Number of arguments must be 3
		printf("\nERROR: Invalid Number of Arguments");
		printf("\nThe order of the arguments should be:");
		printf("\n\t1) The Path to the directory");
		printf("\n\t2) Text string to be searched within above directory\n");
		syslog(LOG_ERR, "ERROR: Invalid Number of Arguments");
		syslog(LOG_INFO, "The order of the arguments should be:");
		syslog(LOG_INFO, "1) The path to the directory");
		syslog(LOG_INFO, "2) Text string to be searched within above direcotry");
		return 1;
	}
	
	char* path = argv[1];
	char* writestr = argv[2];
	
	char *filename = basename(path); //Extracting filename to print in the final debug message
	
	int fd;//File descriptor for File I/O
	
	//creating a file with read-write for user, read for group and world
	fd = creat (path, 0644); //creating file in the path
	if(fd == ERROR){        
		printf("\nERROR: %s could not be created", filename);
		syslog(LOG_ERR, "ERROR: %s Could not be created", filename); 
		close(fd);
		return 1;
	}
	
		
	//Opening file
	fd = open(path, O_RDWR); //Opening the file with read-write access
	if( fd == ERROR){
		printf("\nERROR: Unable to open file");
		syslog(LOG_ERR, "ERROR: Unable to open file");
		close(fd);	
		return 1;
	} 
	
	//Writing to file
	int nr = write(fd, writestr, strlen(writestr) );
	if (nr == ERROR){ //Error check
		printf("\nERROR: Unable to write");
		syslog(LOG_ERR, "ERROR: Unable to write");
		close(fd);
		return 1;
	}
	else if (nr != strlen(writestr) ) { //Partial write check 
		printf("\nERROR: File partially written with %s", writestr);
		syslog(LOG_ERR, "ERROR: File partially written with %s", writestr );
		close(fd);
		return 1;
	}
	else {  //Write successful
		printf("\nWriting %s to %s\n", writestr, filename);
		syslog(LOG_DEBUG, "Writing %s to %s", writestr, filename);
		close(fd);
		return 0;
	}
}
