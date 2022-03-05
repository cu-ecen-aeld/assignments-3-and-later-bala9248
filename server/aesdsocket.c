/*
*File: aesdsocket.c
*
*Opens a stream socket bound to port 9000, listens and accepts a connection and receives data and stores it at
*"/var/tmp/aesdsocketdata" and returns the full content stored in the file to the client as soon as the full packet has been received 
*
*Author: Balapranesh Elango
*
*References: 
*https://beej.us/guide/bgnet/html/#acceptman
*https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
*/

/*Header files*/
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



/*Macros*/
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



/*
 * static void graceful_exit(int status)
 *
 * Used to cleanup and exit from the program
 *
 * Parameters:
 *   status - 0=> Successful exit
 *           -1=> Failure
 *
 * Returns:
 *   None
 */
static void graceful_exit(int status){

	if(sockfd > -1){ //socket fd closed
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
	}
	
	if(newsockfd > -1){//client socket fd closed
		shutdown(newsockfd, SHUT_RDWR);
		close(newsockfd);
	}
	
	if(fd > -1){ //file fd closed
		close(fd);
	}
	
		
	unlink(FILE_PATH); //Unlinking the file
	if (access(FILE_PATH, F_OK) == 0)  
       	remove(FILE_PATH);
    	
	closelog(); 
	
	exit (status);
}


/*
 * static void daemon_start()
 *
 * Used to daemonize the program
 *
 * Parameters:
 *	None
 * Returns:
 *   None
 */
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


/*
 * Signal handler to catch and handle SIGINT and SIGTERM
 */
static void signal_handler(int signum){
	syslog(LOG_INFO, "Caught singal, exiting; Signal==>%d", signum);
	graceful_exit(0); 
}


/* Application entry point */
int main(int argc, char *argv[]) {

	int rc; 
	struct addrinfo hints;
	struct sockaddr_in cli_addr;
	bool daemon_fl = FALSE;
	socklen_t clilen;
	
	
	if( argc > 1 && !strcmp("-d", (char *)argv[1]) ) //Check to see if -d was specified
		daemon_fl = TRUE;	
	
	
	/*Initializing the signal handlers for SIGINT and SIGTERM*/
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
	
	//Populate the hints structure
	memset(&hints, 0, sizeof(hints) );
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo(NULL, MYPORT, &hints, &res);
	
	//getaddrinfo gets address that we need to bind to.
	
	if(rc){
		log_message(LOG_ERR, "ERROR: getaddrinfo() fail");
		graceful_exit(-1);
	}
	
	sockfd = socket(res->ai_family, res->ai_socktype, 0);
	if(sockfd == -1) {
		log_message(LOG_ERR, "ERROR: socket() fail"); //socket() failed
		graceful_exit(-1);
	}	
	
	rc = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if (rc) {
		log_message(LOG_ERR, "ERROR: bind() fail");
		graceful_exit(-1);
	}
	
	freeaddrinfo(res); 
	
	if(daemon_fl == TRUE){ //If -d was specified, run program as daemon
		syslog(LOG_INFO, "Running as daemon");
		daemon_start();
	}

	//listen for connection
	rc = listen(sockfd, BACKLOG);
	if(rc == -1){
		log_message(LOG_ERR, "ERROR: listen() fail");
		graceful_exit(-1);
	}
	
	
	clilen = sizeof(cli_addr);
	
	fd = creat(FILE_PATH, 0644); //create a file to store the received content
	if( fd == -1 ){
		log_message(LOG_ERR, "ERROR: creat() fail");
		graceful_exit(-1);
	}
	close(fd);

	int total_size = 0;
	
	while (1) {				

		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //accept the connection
    	        if (newsockfd < 0) {
      			log_message(LOG_ERR, "ERROR: accept() fail");
			graceful_exit(-1);
     		}
	
		
		log_message(LOG_INFO, "Accepted connection from %s", inet_ntoa(cli_addr.sin_addr) );
		
		
		char temp_buf[BUF_SIZE] = {0}; 
		
		buf = (char*) calloc (BUF_SIZE, sizeof(char)); //malloc an array of arbitrary size to be realloced later depending on the size of received data
		
		if (buf == NULL){
			log_message(LOG_ERR, "ERROR: calloc() fail");
			graceful_exit(-1);
		}
		
		int req_size = 0;
		bool recv_flag = FALSE;
		while (recv_flag == FALSE) { //Recieve until "\n" is received
		
			memset(temp_buf, 0, sizeof(temp_buf));
			int numbytes = recv(newsockfd, temp_buf, BUF_SIZE, 0); //Receive data		
			if(numbytes == -1){
				log_message(LOG_ERR, "ERROR: recv() fail");
				graceful_exit(-1);
			}
				
			req_size += numbytes; 
			total_size += req_size;
			buf = (char *)realloc(buf, req_size+1); //Realloc data as required
			
			if (buf == NULL) {
				log_message(LOG_ERR, "ERROR: malloc() fail");
				graceful_exit(-1);
			}

			
			strncat(buf, temp_buf, numbytes);	//Append to the buffer
			
			if(strchr(temp_buf, '\n') != NULL) //Check if "\n" is specified
				recv_flag = TRUE;
			
			
		}
	 	
	 	
	 	//writing to location
	 	fd = open(FILE_PATH, O_RDWR);
	 	
	 	if( fd == -1 ){
			log_message(LOG_ERR, "ERROR: open() fail");
			graceful_exit(-1);
		}
		lseek(fd, 0, SEEK_END); //Write to EOF
		rc = write(fd, buf, req_size);
		if(rc == -1){
			log_message(LOG_ERR, "ERROR: write() fail");
			graceful_exit(-1);
		}
		

		lseek(fd, 0, SEEK_SET); //Read from beginning of file
		int temp = total_size;
		free(buf);
		buf = (char *) malloc(temp*(sizeof(char) )); //Malloc buf to read from file
		
		int read_size = read(fd, buf, temp);
			
		rc = send(newsockfd, buf, read_size, 0); //send to client
			
		if(rc == -1) {
			log_message(LOG_ERR, "ERROR: send() fail");
			graceful_exit(-1);
		}

		free(buf); //free recently malloced buffer
		close(fd);
		log_message(LOG_INFO, "Closed connection from %s", inet_ntoa(cli_addr.sin_addr) );
	}
	
	graceful_exit(0);
	
	
}
