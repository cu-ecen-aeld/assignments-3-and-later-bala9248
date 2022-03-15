#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

#define TRUE  (1)
#define FALSE (0)
void* threadfunc(void* thread_param) {

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data *thread_func_args = (struct thread_data *) thread_param;
    int status;
    thread_func_args->thread_complete_success = TRUE; //Setting the default  value as true
    
    status = usleep(thread_func_args->wait_obtain_time_ms * 1000); //Sleep before obtaining mutex
    if ( status){ //error check for sleep
    	thread_func_args->thread_complete_success = FALSE;
    	ERROR_LOG("usec failed during wait to obtain mutex");
    	return thread_param;
    }
    status = pthread_mutex_lock( thread_func_args->mutex_g); //obtain mutex
    
    if ( status){ //error check for mutex_lock
    	thread_func_args->thread_complete_success = FALSE;
    	ERROR_LOG("Mutex Lock failed");
    	return thread_param;
    }
    status = usleep(thread_func_args->wait_release_time_ms * 1000); //sleep until release time
    if ( status){ //error check for sleep
   	ERROR_LOG("usec failed during wait to release mutex");
    	thread_func_args->thread_complete_success = FALSE;
    	return thread_param;
    }
    status = pthread_mutex_unlock( thread_func_args->mutex_g); //unlock mutex
    if ( status ){ //error check for mutex unlock
        ERROR_LOG("Mutex unlock fail");
    	thread_func_args->thread_complete_success = FALSE;
    	return thread_param;
    }	
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /*
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     * 
     * See implementation details in threading.h file comment block
     */
    struct thread_data *threadParam = (struct thread_data *)malloc(sizeof(struct thread_data*));  //malloc a pointer to the structure
    
    if( threadParam == NULL){ //error check to see if malloc passed
    	ERROR_LOG("Malloc failed- returned NULL");
    	return FALSE;
    }
    
    //Params for the pthread
    threadParam->wait_obtain_time_ms = wait_to_obtain_ms;
    threadParam->wait_release_time_ms = wait_to_release_ms;
    threadParam->mutex_g = mutex;
    
    int status; 
    
    //Create a pthread
    status = pthread_create( thread,   // thread 
                             NULL,     //Default Attributes
                             threadfunc, //Starting point for thread
                             (void*)threadParam);  //Parameters
    
    if (status == 0) { //Error check for create
        DEBUG_LOG("Thread Created successfully");
    	return TRUE;
    }
    				  
    else {
       ERROR_LOG("Thread could not be created");
    	return FALSE;
    }
}

