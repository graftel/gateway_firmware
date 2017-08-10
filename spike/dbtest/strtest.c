#include <string.h>
#include <stdio.h>
#include <stdlib.h>
void main()
{
	char test[1000];
	char test1[50];
	int b = 86;
	
//	strcpy(test1,"bb");
	sprintf(test1,"%d",b);
	
	strcpy(test,"aaaaaaaa");
	
	strcat(test,test1);
	
	
	
	
	
	
	printf("char=%s\n",test);
	
	printf("size=%d\n",strlen(test));
	
}