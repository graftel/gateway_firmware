#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

main()
{
	struct timespec start_time;
	
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	
	printf("%d\n",start_time.tv_sec);
	
	printf("%d\n", start_time.tv_nsec);
	
	long a =  start_time.tv_sec * 1e3 + start_time.tv_nsec / 1e6;
	
	uint64_t b = 0ULL;
	
	memcpy(&b, &a, sizeof(uint32_t));
	
	printf("value a = %d\n",a);
	
	printf("value b = %llu\n",b);
	
	long *p_a = &a;
	
	int i;
	char c[8];
	memcpy(c, &b, sizeof(uint64_t));
	
	for(i = 0; i < sizeof(uint64_t); i++)
	{
		
		printf("%02x ", c[i]);
	}
	
	
	//uint32_t *p_a = &a;
	//uint64_t *p_b = &b;
	
	//printf("%04x",*p_a);
	
	printf("\n");
}