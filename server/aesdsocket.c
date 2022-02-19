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
#define FILE_PATH "/var/tmp/aesdsocketdata"
#define BUF_SIZE (100)

struct addrinfo *res ;
int sockfd, newsockfd;
int fd;
char* buf = NULL; 

static void graceful_exit(int status){

	if(sockfd > -1){
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
	}
	
	if(newsockfd > -1){
		shutdown(newsockfd, SHUT_RDWR);
		close(newsockfd);
	}
	
	if(fd > -1){
		close(fd);
	}
	
	//if(buf != NULL)
	//	free(buf);
	
	//if(res != NULL)
	//	free(res);
		
	unlink(FILE_PATH);
	closelog();
	
	exit (status);
}

static void daemon_start(){
 	/* create new process */
 	pid_t pid = fork ( );
 	if (pid == -1)
 		graceful_exit(-1);
 		
 	else if (pid != 0)
 		exit (EXIT_SUCCESS);
 	/* create new session and process group */
	 if (setsid ( ) == -1)
 		graceful_exit(-1);
 	/* set the working directory to the root directory */
 	if (chdir ("/") == -1)
 		graceful_exit(-1);

 	/* redirect fd's 0,1,2 to /dev/null */
 	open ("/dev/null", O_RDWR); /* stdin */
 	dup (0); /* stdout */
 	dup (0); /* stderror */
 	
}

static void signal_handler(int signum){
	syslog(LOG_INFO, "Signal Caught==>%d", signum);
	graceful_exit(0);
}

/* Application entry point */
int main(int argc, char *argv[]) {

	int rc; 
	struct addrinfo hints;
	struct sockaddr cli_addr;
	bool daemon_fl = FALSE;
	socklen_t clilen;
	
	
	if( argc > 1 && !strcmp("-d", (char *)argv[1]) ) //Check to see if -d was specified
		daemon_fl = TRUE;	
	
	sig_t sig_rc;
	sig_rc = signal(SIGTERM, signal_handler);
	if(sig_rc == SIG_ERR){
		log_message(LOG_ERR, "ERROR: Sigterm signal() fail");
		graceful_exit(-1);
	}
	
	sig_rc = signal(SIGINT, signal_handler);
	if(sig_rc == SIG_ERR){
		log_message(LOG_ERR, "ERROR: sigint signal() fail");
		graceful_exit(-1);
	}
	
	
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
	
	freeaddrinfo(res);
	
	if(daemon_fl == TRUE){
		syslog(LOG_INFO, "Running as daemon");
		daemon_start();
	}


	rc = listen(sockfd, BACKLOG);
	if(rc == -1){
		log_message(LOG_ERR, "ERROR: listen() fail");
		graceful_exit(-1);
	}
	
	
	clilen = sizeof(cli_addr);
	
	fd = creat(FILE_PATH, 0644);
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
	

		log_message(LOG_INFO, "“Accepted connection from %s", cli_addr.sa_data);
		
		
		char temp_buf[BUF_SIZE] = {0};
		
		buf = (char*) calloc (BUF_SIZE, sizeof(char));
		
		if (buf == NULL){
			log_message(LOG_ERR, "ERROR: calloc() fail");
			graceful_exit(-1);
		}
		
		int req_size = 0;
		bool recv_flag = FALSE;
		while (recv_flag == FALSE) {
		
			memset(temp_buf, 0, sizeof(temp_buf));
			int numbytes = recv(newsockfd, temp_buf, BUF_SIZE, 0);		
			if(numbytes == -1){
				log_message(LOG_ERR, "ERROR: recv() fail");
				graceful_exit(-1);
			}
				
			req_size += numbytes;
			total_size += req_size;
			buf = (char *)realloc(buf, req_size+1);
			if (buf == NULL) {
				log_message(LOG_ERR, "ERROR: malloc() fail");
				graceful_exit(-1);
			}

			
			strncat(buf, temp_buf, numbytes);	
			
			if(strchr(temp_buf, '\n') != NULL)
				recv_flag = TRUE;
			
			
		}
	 	
	 	
	 	//writing to location
	 	fd = open(FILE_PATH, O_RDWR);
	 	
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
		free(buf);
		buf = (char *) malloc(temp*(sizeof(char) ));
		
		int read_size = read(fd, buf, temp);
			
		rc = send(newsockfd, buf, read_size, 0);
			
		if(rc == -1) {
			log_message(LOG_ERR, "ERROR: send() fail");
			graceful_exit(-1);
		}

		free(buf);
		close(fd);
	}
	
	graceful_exit(0);
	
	
}
