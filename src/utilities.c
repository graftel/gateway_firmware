// global general utilies functions
#include <utilities.h>

uint8_t singlechar2hex(char din)
{
	if (din >= 0x30 && din <= 0x39)
	{
		din -= 0x30;
	}
	else if (din >= 41 && din <= 0x46)
	{
		din -= 0x37;
	}

	return din;
}

uint16_t char2hex16(uint8_t *data, uint8_t start_pos)
{

    uint16_t out = 0x00;

    out += (singlechar2hex(data[start_pos]) << 12);
    out += (singlechar2hex(data[start_pos + 1]) << 8);
    out += (singlechar2hex(data[start_pos + 2]) << 4);
    out += singlechar2hex(data[start_pos + 3]);
    return out;

}

uint8_t char2hex(uint8_t *data, uint8_t start_pos)
{
	uint8_t out = 0x00;

	out = (singlechar2hex(data[start_pos]) << 4) + singlechar2hex(data[start_pos + 1]);

	return out;

}
char nibbleToHexCharacter(uint8_t nibble) {
                                   /* Converts 4 bits into hexadecimal */
  if (nibble < 10) {
    return ('0' + nibble);
  }
  else {
    return ('A' + nibble - 10);
  }
}
uint8_t hex2char(uint8_t data_in, char *out)
{
	out[0] = nibbleToHexCharacter((data_in & 0b11110000) >> 4);
	out[1] = nibbleToHexCharacter(data_in & 0b00001111);

	return 0;
}

int resolve_ip(char *host_name, struct addrinfo *addr)
{
	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
	
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

	printf("host_name=%s\n",host_name);
    s = getaddrinfo(host_name, "http", &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

   /* getaddrinfo() returns a list of address structures.
	  Try each address until we successfully connect(2).
	  If socket(2) (or connect(2)) fails, we (close the socket
	  and) try the next address. */

   for (rp = result; rp != NULL; rp = rp->ai_next) {
	   sfd = socket(rp->ai_family, rp->ai_socktype,
					rp->ai_protocol);
	   if (sfd == -1)
		   continue;

	   if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
		{
			printf("debug 1\n");
		   memcpy(addr, rp, sizeof(struct addrinfo));
		   printf("debug 2\n");
		   break;                  /* Success */
	   }
   }
	printf("debug 3\n");
   if (rp == NULL) {               /* No address succeeded */
	   fprintf(stderr, "Could not connect\n");
	   close(sfd);
	   return -1;
   }
	printf("debug 4\n");
   freeaddrinfo(result);           /* No longer needed */
   printf("debug 5\n");
   close(sfd);
   printf("debug 6\n");
   return 0;
}

int check_internet()
{	
    int sockfd;
    char buffer[MAX_LINE];
	struct addrinfo re_addr;

    sockfd = -1;

	if (resolve_ip(INTERNET_CHECK_HOST, &re_addr) == 0)
	{
        if((sockfd = socket(re_addr.ai_family,SOCK_STREAM,IPPROTO_TCP)) != -1)
        {
			if( connect(sockfd,re_addr.ai_addr,re_addr.ai_addrlen) == 0)
			{
				if(write(sockfd,"GET /index.html HTTP/1.1\r\n\r\n", 29) >= 28)
				{
					shutdown(sockfd, SHUT_WR);
					if(read(sockfd, buffer, MAX_LINE) != -1) // all right!
					{
						close(sockfd);
						return 0;
					}
				}
			}
        }
    }
	
    if(sockfd!=-1)
        close(sockfd);
    return -1; // no internet
}