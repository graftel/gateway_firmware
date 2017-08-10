#include <mhash.h>
//#include <stdio.h> /* printf, sprintf */
//#include <stdlib.h> /* exit */
//#include <unistd.h> /* read, write, close */
//#include <string.h> /* memcpy, memset */
//#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <uuid/uuid.h>
#include <defines.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/ioctl.h>

int free_defs(bridge *data);
void *send_data_to_cloud_wrapper(void *args);
int get_def_from_cloud(bridge *data);