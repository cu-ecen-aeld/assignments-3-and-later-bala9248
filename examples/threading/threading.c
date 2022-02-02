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
    thread_func_args->thread_complete_success = TRUE;
    
    status = usleep(thread_func_args->wait_obtain_time_ms * 1000);
    if ( status != 0){
    	thread_func_args->thread_complete_success = FALSE;
    	ERROR_LOG("usec failed during wait to obtain mutex");
    	return thread_param;
    }
    status = pthread_mutex_lock( thread_func_args->mutex_g); 
    
    if ( status != 0){
    	thread_func_args->thread_complete_success = FALSE;
    	ERROR_LOG("Mutex Lock failed");
    	return thread_param;
    }
    status = usleep(thread_func_args->wait_release_time_ms * 1000);
    if ( status != 0){
   	ERROR_LOG("usec failed during wait to release mutex");
    	thread_func_args->thread_complete_success = FALSE;
    	return thread_param;
    }
    status = pthread_mutex_unlock( thread_func_args->mutex_g); 
    if ( status != 0){
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
    struct thread_data *threadParam = (struct thread_data *)malloc(sizeof(struct thread_data*));
    
    if( threadParam == NULL){
    	ERROR_LOG("Malloc failed- returned NULL");
    	return FALSE;
    }
    
    threadParam->wait_obtain_time_ms = wait_to_obtain_ms;
    threadParam->wait_release_time_ms = wait_to_release_ms;
    threadParam->mutex_g = mutex;
    
    int status; 
    
    status = pthread_create( thread,   // thread 
                             NULL,     //Default Attributes
                             threadfunc, //Starting point for thread
                             (void*)threadParam);  //Parameters
    
    if (status == 0) {
        DEBUG_LOG("Thread Created successfully");
    	return TRUE;
    }
    				  
    else {
       ERROR_LOG("Thread could not be created");
    	return FALSE;
    }
}

