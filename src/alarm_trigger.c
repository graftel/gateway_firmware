//alarm

#include <i2c_rtc.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <alarm_trigger.h>

void *start_alarm_trigger_wrapper(void *args)
{
	alarm_trigger_thread_args *arguments = args;
	
	return (void *)start_alarm_trigger(arguments->interval);
}

int start_alarm_trigger(int interval){
	
sigval_t value;
int start_time,current_time;
MCP79410_Read_Epoch_Time(&start_time);

while(1)
{
  MCP79410_Read_Epoch_Time(&current_time);
  if ((current_time - start_time) % interval == 0 && (current_time - start_time) > 0)
  {
	  value.sival_int = current_time;
	  sigqueue(getpid(),SIGUSR2,value);
  }

  sleep(1);
}

pthread_exit(NULL);
return 0;
}