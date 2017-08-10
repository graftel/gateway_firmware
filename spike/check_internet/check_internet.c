
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_LINE 4096
#define INTERNET_CHECK_HOST_NAME "www.google.com"

int main()
{
    int sockfd,val;
    char buffer[MAX_LINE];
    struct hostent *google_ent=NULL;
    struct sockaddr_in google_addr;

    sockfd = -1;

    if((google_ent = gethostbyname(INTERNET_CHECK_HOST_NAME)) != NULL)
    {
        if((sockfd =    socket(google_ent->h_addrtype,SOCK_STREAM,IPPROTO_TCP)) != -1)
        {
            val = 1;
            if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, (char *) &val, sizeof(val)) == 0 && setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY, (char *) &val, sizeof(val)) == 0)
            {
                google_addr.sin_family = google_ent->h_addrtype;
                memcpy(&(google_addr.sin_addr), google_ent->h_addr, google_ent->h_length);
                google_addr.sin_port = htons(80);
                if( connect(sockfd,(struct sockaddr *) &google_addr,sizeof(google_addr)) == 0)
                {
                    if(write(sockfd,"GET /index.html HTTP/1.1\r\n\r\n", 29) >= 28)
                    {
                        shutdown(sockfd, SHUT_WR);
                        if(read(sockfd, buffer, MAX_LINE) != -1) // all right!
                        {
							printf(buffer);
							
                            close(sockfd);
							printf("OK\n");
                            return 1;
                        }
                        else
                            printf("read\n");
                    }
                    else
                        printf("write\n");
                }
                else
                    printf("connect\n");
            }
            else
                printf("aa\n");
        }
        else
            printf("socket\n");
    }
    else
        printf("cannot resolve ip\n");
	
    if(sockfd!=-1)
        close(sockfd);
	printf("no internet\n");
    return 0; // no internet

}