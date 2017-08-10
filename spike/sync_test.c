#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void display_message(int s) {
     printf("copyit: Still working...\n" );
     alarm(2);    //for every second
     signal(SIGALRM, display_message);
}

int main(void) {
    signal(SIGALRM, display_message);
    alarm(1);
    int n = 0;
    while (1) {
        ++n;
    }
    return 0;
}
