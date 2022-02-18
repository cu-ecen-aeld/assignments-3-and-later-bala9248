#include<string.h>
#include<stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>
#include <dirent.h>
#include <libgen.h>


//https://beej.us/guide/bgnet/html/#acceptman
//https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm

#define MYPORT   "9000"
#define BACKLOG  (10)
#define FALSE    (0)
#define TRUE     (1)
#define log_message syslog
static void graceful_exit(int status){
	//close all fds
	//close log
	closelog();
	exit (status);
}

static void daemon_start(){
 	/* create new process */
 	pid_t pid = fork ( );
 	if (pid == -1)
 		exit -1;
 	else if (pid != 0)
 		exit (EXIT_SUCCESS);
 	/* create new session and process group */
	 if (setsid ( ) == -1)
 		exit -1;
 	/* set the working directory to the root directory */
 	if (chdir ("/") == -1)
 		exit -1;

 	/* redirect fd's 0,1,2 to /dev/null */
 	open ("/dev/null", O_RDWR); /* stdin */
 	dup (0); /* stdout */
 	dup (0); /* stderror */
}

/* Application entry point */
int main(int argc, char *argv[]) {

	int rc; 
	struct addrinfo hints, *res ;
	struct sockaddr cli_addr;
	bool daemon = FALSE;
	int sockfd, newsockfd;
	socklen_t clilen;
	
	printf("Starting aesdsocket!!!");
	
	//if( !strcmp("-d", argv[1]) ) //Check to see if -d was specified
	//	daemon = TRUE;	

	openlog("aesdscoket.c - LOG", LOG_PID, LOG_USER); //Opening syslog for logging using LOG_USER facility
	
	memset(&hints, 0, sizeof(hints) );
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo(NULL, MYPORT, &hints, &res);
	
	if(rc){
		log_message(LOG_ERR, "ERROR: getaddrinfo() fail");
		graceful_exit(-1);
	}
	
	sockfd = socket(res->ai_family, res->ai_socktype, 0);
	if(sockfd == -1) {
		log_message(LOG_ERR, "ERROR: socket() fail");
		graceful_exit(-1);
	}	
	
	rc = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if (rc) {
		log_message(LOG_ERR, "ERROR: bind() fail");
		graceful_exit(-1);
	}
	
	if(daemon == TRUE)
		daemon_start();
	
	free(res);

	rc = listen(sockfd, BACKLOG);
	if(rc == -1){
		log_message(LOG_ERR, "ERROR: listen() fail");
		graceful_exit(-1);
	}
	
	
	clilen = sizeof(cli_addr);
	
	int fd = creat("/var/tmp/aesdsocketdata", 0644);
	if( fd == -1 ){
		log_message(LOG_ERR, "ERROR: creat() fail");
		graceful_exit(-1);
	}
	close(fd);

	int total_size = 0;
	while (1) {				

		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    	        if (newsockfd < 0) {
      			log_message(LOG_ERR, "ERROR: accept() fail");
			graceful_exit(-1);
     		}
	
		printf("Accepted connection from XXX");
		log_message(LOG_INFO, "“Accepted connection from %s", cli_addr.sa_data);
		
		
		int buf_size = 100;
		char temp_buf[buf_size];

				
		char* buf; 
		buf = (char*) calloc (0, buf_size*sizeof(char));
		int req_size = 0;
		while (1) {
		
			memset(temp_buf, 0, buf_size);
			int numbytes = recv(newsockfd, temp_buf, buf_size, 0);		
			if(numbytes == -1){
				log_message(LOG_ERR, "ERROR: recv() fail");
				graceful_exit(-1);
			}
			
			syslog(LOG_DEBUG, "received string = %s", temp_buf);
			
			
			if(numbytes > 0) {
				
				req_size += numbytes;
				total_size += req_size;	
				buf = (char*)realloc(buf, req_size);
				if (buf == NULL) {
					log_message(LOG_ERR, "ERROR: malloc() fail");
					graceful_exit(-1);
				}
			}
			
			strncat(buf, temp_buf, numbytes);	
			
		        if(strchr(temp_buf, '\n') != NULL)
				break;
			
		}
	 	
	 	syslog(LOG_DEBUG, "Full string = %s", buf);
	 	//Packet fully received
	 	
	 	//writing to location
	 	fd = open("/var/tmp/aesdsocketdata", O_RDWR);
	 	
	 	if( fd == -1 ){
			log_message(LOG_ERR, "ERROR: open() fail");
			graceful_exit(-1);
		}
		lseek(fd, 0, SEEK_END);
		rc = write(fd, buf, req_size);
		if(rc == -1){
			log_message(LOG_ERR, "ERROR: write() fail");
			graceful_exit(-1);
		}
		

		lseek(fd, 0, SEEK_SET);
		int temp = total_size;

		int read_size = read(fd, temp_buf, temp);
		
		syslog(LOG_DEBUG, "Read string = %s", temp_buf);
			
		rc = send(newsockfd, &temp_buf, read_size, 0);
			
		if(rc == -1) {
			log_message(LOG_ERR, "ERROR: send() fail");
			graceful_exit(-1);
		}

		free(buf);
		close(fd);
	}
	
	graceful_exit(0);
	
	
}
