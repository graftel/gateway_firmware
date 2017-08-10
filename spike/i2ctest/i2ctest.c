#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

void Read_Date(struct tm *result)
{
	time_t cur_time;
	int status;
	status = MCP79410_Read_Date(result);
	
	printf("status=%d\n", status);
	
	if (status < 0){
		cur_time = time(NULL);
		*result = *localtime(&cur_time);
	}		
}
int main()
{
   int result, i;
   uint8_t read_out;
   uint8_t reg_value = 0x00;
   
   struct tm tm_info;
	char buffer[40];
	
	Read_Date(&tm_info);
	
	strftime(buffer,sizeof(buffer),"BLE_LOG_%Y_%m_%d_%H_%M_%S.cvs",&tm_info);
	FILE *fp;
	
	fp=fopen(buffer, "w");
	
	fprintf(fp,"data1\n");
	
	fclose(fp);
	
}
