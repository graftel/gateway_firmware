#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/resource.h>

void *print_message_function( void *ptr );
char *ma_test;
struct rusage r_usage;
int ret1;

struct argument {
	char *test_input;
	int size;
};

main()
{
	
     pthread_t thread1, thread2;
     const char *message1 = "Thread 1";
     const char *message2 = "Thread 2";
     int  iret1, iret2, i;

	 
	
	 
	 for (i = 0; i < 10; i++)
	 {
		 getrusage(RUSAGE_SELF,&r_usage);
		 printf("Memory usage before: %ld bytes\n",r_usage.ru_maxrss);
	 
		 sleep(5);
		 ma_test = malloc(1000000000);
		 sleep(5);
		 getrusage(RUSAGE_SELF,&r_usage);
		 printf("Memory usage after: %ld bytes\n",r_usage.ru_maxrss);
		/* Create independent threads each of which will execute function */

		struct argument test_args;
		
		test_args.test_input = ma_test;
		test_args.size = 1000000000;
		
		
		 iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) &test_args);
		 

		 if(iret1)
		 {
			 fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
			 exit(EXIT_FAILURE);
		 }
		 
		 if (iret1 == 0)
		 {
			 iret1 = pthread_detach(thread1);
		 }
		 
		 printf("pthread_create() for thread 1 returns: %d\n",iret1);
	 
		 /* Wait till threads are complete before main continues. Unless we  */
		 /* wait we run the risk of executing an exit which will terminate   */
		 /* the process and all threads before the threads have completed.   */

		 //pthread_join( thread1, NULL);
		 sleep(1);
	 }
	 
	 sleep(10);
	 
	 printf("program ended\n");
     exit(EXIT_SUCCESS);
}

void *print_message_function( void *ptr )
{
	 
     struct argument *args = ptr;
	 
	 char *message = args->test_input;
	 
     printf("Memory delocation usage before: %ld bytes\n",r_usage.ru_maxrss);
	 free(message);
	 printf("Memory delocation usage after: %ld bytes\n",r_usage.ru_maxrss);
	 printf("end: %ld bytes\n",r_usage.ru_maxrss);
	 
	 //pthread_exit(&ret1);
	 
	 return NULL;
}