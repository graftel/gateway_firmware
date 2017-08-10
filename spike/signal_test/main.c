#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
void sigusr2(int signo, siginfo_t *info, void *extra) 
{
       void *ptr_val = info->si_value.sival_ptr;
       int int_val = info->si_value.sival_int;
       printf("Signal %d, value %d  \n", signo, int_val);
}



int main()
{
struct sigaction action;

action.sa_flags = SA_SIGINFO; 
action.sa_sigaction = &sigusr2;

if (sigaction(SIGUSR2, &action, NULL) == -1) { 
    perror("sigusr: sigaction");
    return 1;
}	

sigval_t value;

while(1)
{
  value.sival_int = time(NULL);
  sigqueue(getpid(),SIGUSR2,value);


sleep(5);
}

	return 0;
}
