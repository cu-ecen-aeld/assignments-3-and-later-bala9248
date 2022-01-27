#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the commands in ... with arguments @param arguments were executed 
 *   successfully using the system() call, false if an error occurred, 
 *   either in invocation of the system() command, or if a non-zero return 
 *   value was returned by the command issued in @param.
*/
bool do_system(const char *cmd)
{

/*
 *  TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *  and return a boolean true if the system() call completed with success 
 *  or false() if it returned a failure
*/  
   
   openlog("systemcalls.c - LOG", LOG_PID, LOG_USER); //Opening syslog for logging using LOG_USER facility
   
    int status = system(cmd);  //calling system() with command cmd
    
    if(status == -1){
    	printf("\nERROR: system() call failed");
    	syslog(LOG_ERR, "ERROR: system() call failed");
    	return false;
    }
    
    printf("\nsystem() call completed with success");
    syslog(LOG_DEBUG, "system() call completed with success");
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the 
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];



    
/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *   
*/	
    
    int status;
    int ret_status;
    pid_t pid;
    pid = fork(); //fork to create a new child process
    
    openlog("systemcalls.c - LOG", LOG_PID, LOG_USER); //Opening syslog for logging using LOG_USER facility
    if (pid == -1) {   //Fork error check
    	printf("\nERROR: Fork failed");
    	syslog(LOG_ERR, "ERROR: Fork failed");
    	return false;
    }  
	
    else if(!pid) { //Child process
    	 status = execv(command[0], command); //Exec in child process
    	
         //Exec should not return
    	 printf("\nERROR: Exec Returned"); 
    	 syslog(LOG_ERR, "ERROR: Exec Returned");
    	 exit(-1);
    }
    
    else if (pid > 0) {
    
    	 ret_status = waitpid(pid, &status, 0); //Waiting for exec process to terminate
    	 if(ret_status == -1){ //Wait error check
    	     printf("\nERROR: Wait failed");
    	     syslog(LOG_ERR, "ERROR: Wait failed");
    	     return false;
    	 }
    	 
    	 //Wait exit check
    	 if ( ! (WIFEXITED(status)) || WEXITSTATUS(status) ) {
		printf("\nERROR: Wait Exited abnormally");
		syslog(LOG_ERR, "ERROR: Wait Exited abnormally");
		return false;
    	 }
    	 
    	 
    }

    va_end(args);
    

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.  
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *   
*/

    int status;
    int ret_status;
    pid_t pid;
    pid = fork(); //fork to create a new child process
    
    openlog("systemcalls.c - LOG", LOG_PID, LOG_USER); //Opening syslog for logging using LOG_USER facility
    
    //create file to redirect output
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    
    if (fd < 0){ //file open error check
    	printf("\nERROR: Opening/Creating file failed");
    	syslog(LOG_ERR, "ERROR: Opening/Creating file failed");
    }
    
    if (pid == -1) { //fork error check
    	printf("\nERROR: Fork failed");
    	syslog(LOG_ERR, "ERROR: Fork failed");
    	return false;
    }  
	
    else if(!pid) { //child process
    	if (dup2(fd, 1) < 0){ //duplicate fd with fd value 1
    		printf("\n\rFailure");
    		return false;
    	}
    	close(fd);
    	status = execv(command[0], command); //Exec in child process
    	//Exec should not return
    	printf("\nERROR: Exec Returned");
    	syslog(LOG_ERR, "ERROR: Exec Returned");
    	exit(1);
    }
    
    else if (pid > 0) {
    	 //Wait for child process to terminate
    	 ret_status = waitpid(pid, &status, 0);
    	 if(ret_status == -1){
    	     printf("\nERROR: Wait failed");
    	     syslog(LOG_ERR, "ERROR: Wait failed");
    	     close(fd);
    	     return false;
    	 }
    	 
    	 //Wait exit check
    	 if ( ! (WIFEXITED(status)) || WEXITSTATUS(status) ) {
		printf("\nERROR: Wait Exited abnormally\n");
		syslog(LOG_ERR, "ERROR: Wait Exited abnormally");
		close(fd);
		return false;
    	 }
    	 
    	 
    }

    va_end(args);
    
    close(fd);
    return true;
}



