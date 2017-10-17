#ifndef _UART_MODULE_H
#define _UART_MODULE_H

#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define UART_DEV_NAME "/dev/ttyxmc4"
#define WIFI_INTERFACE_NAME "wlan0"
#define MAX_BUFFER_SIZE 256
#define CMD_READ_TIME "ReadTime"
#define CMD_WIFI_SCAN "WifiScan"
#define CMD_WIFI_SET_SSID "WifiSetSSID"
#define CMD_WIFI_SET_PSK  "WifiSetPSK"
#define CMD_WIFI_CONNET  "WifiConnect"
#define CMD_WIFI_GET_SSID   "WifiGetSSID"
#define CMD_READ_RAW_DATA  "ReadRawData"
#define CMD_LINE_SIZE 50

#define BUFSIZE 4028
#define MAX_SSID_LEN 50
#define MAX_MAC_ADDR_LEN 20
#define MAX_ENC_LEN 20
#define MAX_QUALITY_LEN 20
#define RETRY_TIMES 3
#define MAX_PSK_LEN 50

typedef enum {
   OPEN = 0,
   WEP,
   WPA,
   WPA2
}enc_type;

typedef struct{
  char SSID[MAX_SSID_LEN];
  char mac_addr[MAX_MAC_ADDR_LEN];
  int channel;
  int cell_num;
  char quality[MAX_QUALITY_LEN];
  int dbm;
  enc_type enc;
  int signal_level;
} scan_info_def;

int connect_to_wifi(const char *ssid, const char *psk);
int wifi_scan(scan_info_def **scan_info, int *size_info);
void *uart_wrapper(void *args);

#endif
