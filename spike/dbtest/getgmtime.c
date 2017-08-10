
#include <time.h>
#include <stdio.h>
#include <string.h>

int main()
{
	time_t time_now;
	struct tm *tmp;
	char time_data[20];
	char time_date1[9];
	char time_full[17];
	time_now = time(NULL);
	tmp = gmtime(&time_now);
	
	if (tmp == NULL)
	{
		fprintf(stderr,"Cannot get UTC time\n");
		return 1;
	}
	
	strftime(time_full,sizeof(time_full),"%Y%m%dT%H%M%SZ",tmp);
	
	memset(time_date1,0,sizeof(time_date1));
	
	memcpy(time_date1, time_full, sizeof(time_date1) - 1);
	
	printf("%d\n",sizeof(time_date1));
	
	printf("date=%s\n",time_date1);
	
	printf("full=%s\n",time_full);
	
	return 0;
}