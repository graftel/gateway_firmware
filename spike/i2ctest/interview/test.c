#include <stdio.h>
#include <string.h>

int main() {
   int i = 0, j = 1, temp;

   while(j  < 100)
  {
     printf("%d\n",i + j);
    temp = i + j;
    i = j;
    j = temp;
  }
}
