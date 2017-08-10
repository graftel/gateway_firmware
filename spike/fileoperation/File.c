#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>


void main()
{
	FILE *fp;
	fp=fopen("test.cvs", "w");
	fprintf(fp, "Testing...\n");
	
	fclose(fp);
	
}