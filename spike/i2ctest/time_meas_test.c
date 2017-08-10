#include <errno.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define RTCaddress 0x6F

main(){
	 struct timespec start;
	  struct timespec end;
	  int n, m = 0;
//	MCP79410_Setup_Date(curSec, curMin, curHour, curDayofweek, is12, curDay, curMonth, curYear);
	clock_gettime(CLOCK_MONOTONIC, &start); // get initial time-stamp
	

	usleep(500000);
	
	clock_gettime(CLOCK_MONOTONIC, &end);   // get final time-stamp

	double t_ns = (double)(end.tv_sec - start.tv_sec) * 1.0e9  +
                  (double)(end.tv_nsec - start.tv_nsec);
                                                 // subtract time-stamps and
                                                 // multiply to get elapsed
                                                 // time in ns
	printf("start_sec=%d\n",start.tv_sec);
   	printf("start_nsec=%d\n",start.tv_nsec);
	printf("end_sec=%d\n",end.tv_sec);
	printf("end_nsec=%d\n",end.tv_nsec);
	printf("time_diff=%g\n",t_ns);
	
}