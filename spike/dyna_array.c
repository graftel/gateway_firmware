#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct
{
	char name[10];
	int id;
} user_str;

int main(int argc, char** argv)
{
  /* declare a pointer do an integer */
  user_str *data; 
  /* we also have to keep track of how big our array is - I use 50 as an example*/
  int datacount = 1;
  data = malloc(sizeof(user_str) * datacount); /* allocate memory for 50 int's */
  if (!data) { /* If data == 0 after the call to malloc, allocation failed for some reason */
    perror("Error allocating memory");
    abort();
  }
  /* at this point, we know that data points to a valid block of memory.
     Remember, however, that this memory is not initialized in any way -- it contains garbage.
     Let's start by clearing it. */
  
  /* now our array contains all zeroes. */
  memcpy(data[0].name,"aaaa",sizeof("aaaa"));
  data[0].id = 5;
  
  
  datacount = 2;
  data = realloc(data, sizeof(user_str) * datacount); /* allocate memory for 50 int's */
  memcpy(data[1].name,"bbbb",sizeof("bbbb"));
  data[1].id = 2;
  int i = 0;
  for ( i = 0; i < datacount; i++)
  {
	  printf("Element %d: %s, %d\n",i,data[i].name,data[i].id);
  }
  
  
  free(data);
  
}