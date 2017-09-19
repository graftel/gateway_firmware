#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <defines.h>
#include <json-c/json.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>   //ifreq
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <dynamodb_utilities.h>

#define CONFIG_FILE_PATH "/etc/hxmonitor/config.json"
#define CONFIG_FOLDER_PATH "/etc/hxmonitor/"

uint8_t singlechar2hex(char din);

uint16_t char2hex16(uint8_t *data, uint8_t start_pos);

uint8_t char2hex(uint8_t *data, uint8_t start_pos);

char nibbleToHexCharacter(uint8_t nibble);

uint8_t hex2char(uint8_t data_in, char *out);

uint8_t cal_checksum(uint8_t *value, uint8_t len);

int resolve_ip(char *host_name, struct addrinfo *addr);

int check_internet();

int file_exists(const char *fname);

int init_data(bridge *bridge_data);

int free_defs(bridge *data);

#endif
