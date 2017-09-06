#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <defines.h>
#include <sys/socket.h>
#include <sys/types.h>
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

uint8_t singlechar2hex(char din);

uint16_t char2hex16(uint8_t *data, uint8_t start_pos);

uint8_t char2hex(uint8_t *data, uint8_t start_pos);

uint8_t hex2char(uint8_t data_in, char *out);

int check_internet();

int resolve_ip(char *host_name, struct addrinfo *addr);

#endif