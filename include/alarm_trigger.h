#ifndef _ALARM_TRIGGER_H
#define _ALARM_TRIGGER_H

typedef struct{
  int interval;
} alarm_trigger_thread_args;

void *start_alarm_trigger_wrapper(void *args);
int start_alarm_trigger(int interval);

#endif