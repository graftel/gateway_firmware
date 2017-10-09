#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <iwlib.h>
#include <sys/time.h>

#define WLAN0 "wlan0"
#define MAX_BUFFER_SIZE 256
#define CMD_READ_TIME "ReadTime"
#define CMD_WIFI_SCAN "WifiScan"
#define CMD_WIFI_SET_SSID "WifiSetSSID"
#define CMD_WIFI_SET_PSK  "WifiSetPSK"
#define CMD_WIFI_CONNET  "WifiConnect"
#define CMD_WIFI_STATUS   "WifiGetSSID"
#define CMD_READ_RAW_DATA  "ReadRawData"
//-------------------------
//----- SETUP USART 0 -----
//-------------------------
//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively

#define BUFSIZE 4028
#define MAX_SSID_LEN 50
#define MAX_MAC_ADDR_LEN 20
#define MAX_ENC_LEN 20
#define MAX_QUALITY_LEN 20
#define RETRY_TIMES 3
#define MAX_PSK_LEN 50
typedef enum {
   OPEN = 0,
   WEP ,
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

int enc_status = 0;

int connect_to_wifi(const char *ssid, const char *psk)
{
	  FILE * pFile;
	  pFile = fopen ("/etc/network/interfaces","r");
      size_t len = 0;
	  ssize_t read;

	  char wpa_file_path[50];

	  if (pFile!=NULL)
	  {
			char *loc;
			int found_iface = 0;

      fseek(pFile, 0, SEEK_END);
      long fsize = ftell(pFile);
      fseek(pFile, 0, SEEK_SET);  //same as rewind(f);

      char *string = malloc(fsize + 1);
      fread(string, fsize, 1, pFile);
      fclose(pFile);

			loc = strstr(string, "wlan0");
			if (loc != NULL)
			{
				found_iface = 1;
        printf("found interface\n" );
			}
			loc = strstr(loc, "wpa-conf");
			if (loc != NULL)
			{
        printf("found wpa-conf\n");
				int j = 0;
				memset(wpa_file_path,0,sizeof(wpa_file_path));
				loc += 9;
        while(*(loc + j) != 0x0a)
				{
					memcpy(wpa_file_path + j, loc + j, 1);
          j++;
				}
				printf("wpa_file_path=%s\n",wpa_file_path);
			}
	  }

    char cmd[BUFSIZE];

    strcpy(cmd, "wpa_passphrase ");
    strcat(cmd, ssid);
    strcat(cmd, " ");
    strcat(cmd, psk);
    strcat(cmd, " > ");
    strcat(cmd, wpa_file_path);
    strcat(cmd, " && ifdown wlan0 && ifup wlan0");

    printf("write cmd = %s\n", cmd);

    char buf[BUFSIZE];
    FILE *fp;
    char getSSID[MAX_SSID_LEN];
    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
        printf("%s\n", buf);
    }

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
    }

    usleep(5000000);

    int connected = 0;

    memset(cmd,0,sizeof(cmd));

    strcpy(cmd, "iwconfig wlan0");

    int retry = RETRY_TIMES;

    while (1)
    {
      if ((fp = popen(cmd, "r")) == NULL) {
          printf("Error opening pipe!\n");
      }

      char *loc;
      memset(buf,0,sizeof(buf));
      while (fgets(buf, BUFSIZE, fp) != NULL) {
        loc = strstr(buf, "ESSID:");
        if (loc != NULL)
        {
          int j = 0;
          loc += 6;
          memset(getSSID,0,sizeof(getSSID));
          while(*(loc + j) != 0x0a)
          {
            memcpy(getSSID + j, loc + j, 1);
            j++;
          }
        }
      }

      if(pclose(fp))  {
          printf("Command not found or exited with error status\n");
      }

      char *ssid_check;
      ssid_check = strstr(getSSID, ssid);

      if (ssid_check != NULL)
      {
        retry = RETRY_TIMES;
        connected++;
        usleep(1000000);

        if (connected > 5)
        {
          printf("connected\n");
          break;
        }
      }
      else
      {
        connected = 0;
        retry--;
        if (retry == 0)
        {
          printf("timeout, cannot connect, recheck password\n");
          return -1;
        }
        else
        {
          usleep(2000000);
        }
      }
    }

	 return 0;
}

int wifi_scan(scan_info_def **scan_info, int *size_info)
{
  char *cmd = "iwlist wlan0 scan";

  char buf[BUFSIZE];
  FILE *fp;

  if ((fp = popen(cmd, "r")) == NULL) {
      printf("Error opening pipe!\n");
      return -1;
  }
  *scan_info = malloc(sizeof(scan_info_def));

  int num_cell = 0;
  int cur_cell = 0;
  char *loc;
  while (fgets(buf, BUFSIZE, fp) != NULL) {
      // Do whatever you want here...
      loc = strstr(buf, "Cell");
      if (loc != NULL)
      {
        if (num_cell != 0)
        {
          *scan_info = realloc(*scan_info, sizeof(scan_info_def) * (num_cell+1));
        }
        cur_cell = num_cell;
        char str_cell[5];
        strncpy(str_cell, loc+4, 3);

        num_cell++;
      }

      loc = strstr(buf, "ESSID:");
      if (loc != NULL)
      {
        int j = 0;
        loc += 6;
        memset((*scan_info)[cur_cell].SSID,0,sizeof((*scan_info)[cur_cell].SSID));
        while(*(loc + j) != 0x0a)
        {
          memcpy((*scan_info)[cur_cell].SSID + j, loc + j, 1);
          j++;
        }
      }

      loc = strstr(buf, "Address:");
      if (loc != NULL)
      {
        int j = 0;
        loc += 8;
        memset((*scan_info)[cur_cell].mac_addr,0,sizeof((*scan_info)[cur_cell].mac_addr));
        while(*(loc + j) != 0x0a)
        {
          memcpy((*scan_info)[cur_cell].mac_addr + j, loc + j, 1);
          j++;
        }
      }

      loc = strstr(buf, "Channel:");
      if (loc != NULL)
      {
        char tmp[10];
        int j = 0;
        loc += 8;
        memset(tmp,0,sizeof(tmp));
        while(*(loc + j) != 0x0a)
        {
          memcpy(tmp + j, loc + j, 1);
          j++;
        }
        (*scan_info)[cur_cell].channel = atoi(tmp);
      }


      loc = strstr(buf, "Quality=");
      if (loc != NULL)
      {
        int j = 0;
        loc += 8;
        memset((*scan_info)[cur_cell].quality,0,sizeof((*scan_info)[cur_cell].quality));
        while(*(loc + j) != 0x20)
        {
          memcpy((*scan_info)[cur_cell].quality + j, loc + j, 1);
          j++;
        }
      }

      loc = strstr(buf, "Signal level=");
      if (loc != NULL)
      {
        char tmp[10];
        int j = 0;
        loc += 13;
        memset(tmp,0,sizeof(tmp));
        while(*(loc + j) != 0x20)
        {
          memcpy(tmp + j, loc + j, 1);
          j++;
        }
        (*scan_info)[cur_cell].signal_level = atoi(tmp);
      }

      loc = strstr(buf, "Encryption key:");
      if (loc != NULL)
      {
        char *check;

        if (strstr(loc + strlen("Encryption key:"), "on") != NULL)
        {
          (*scan_info)[cur_cell].enc = WEP;
        }
        else if (strstr(loc + strlen("Encryption key:"), "off") != NULL)
        {
          (*scan_info)[cur_cell].enc = OPEN;
        }
      }

      loc = strstr(buf, "WPA2 Version");
      if (loc != NULL)
      {
        (*scan_info)[cur_cell].enc = WPA2;
      }

      loc = strstr(buf, "WPA Version");
      if (loc != NULL)
      {
        if ((*scan_info)[cur_cell].enc != WPA2)
        {
          (*scan_info)[cur_cell].enc = WPA;
        }
      }
  }
  (*size_info) = num_cell;

  int i;
  printf("******************* Results START *******************\n", i);
  for (i = 0; i < num_cell; i++)
  {
    printf("cell #%d\n", i);
    printf("ssid=%s\n", (*scan_info)[i].SSID);
    printf("mac_addr=%s\n", (*scan_info)[i].mac_addr);
    printf("quality=%s\n", (*scan_info)[i].quality);
    printf("signal_level=%d\n", (*scan_info)[i].signal_level);
    printf("channel=%d\n", (*scan_info)[i].channel);
    printf("enc=%d\n", (*scan_info)[i].enc);
    printf("------------------------------------------------\n", i);
  }
  printf("******************* Results END *******************\n", i);

  if(pclose(fp))  {
      printf("Command not found or exited with error status\n");
      return -1;
  }
}


int main()
{
	int uart0_filestream = -1;

	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.

	uart0_filestream = open("/dev/ttyS0", O_RDWR | O_NOCTTY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

		//----- TX BYTES -----
	char tx_buffer[20];
	char rx_buffer[MAX_BUFFER_SIZE];
	char data_buffer[MAX_BUFFER_SIZE];
	int index = 0;
	unsigned char *p_tx_buffer;
	uint8_t x;
	// parse command end with /r 13
	memset(rx_buffer, 0, sizeof(rx_buffer));

	if (uart0_filestream != -1)
	{
		while(1)
		{
			if (read (uart0_filestream, &x, 1) > 0)
			{
				if (x != 0x0d && index < MAX_BUFFER_SIZE)
				{
					rx_buffer[index] = x;
					index++;
				}
				else
				{
					if (strcmp(CMD_READ_TIME, rx_buffer) == 0)
					{
						long timenow = time(NULL);
						sprintf(data_buffer, "%ld", timenow);
						write(uart0_filestream, rx_buffer, strlen(rx_buffer));
						write(uart0_filestream, "=", 1);
						write(uart0_filestream, data_buffer, strlen(data_buffer));
						write(uart0_filestream, "\r", 1);
					}
					else if (strcmp(CMD_WIFI_STATUS, rx_buffer) == 0)
					{

	//					write(uart0_filestream, rx_buffer, strlen(rx_buffer));
	//					write(uart0_filestream, "=", 1);
	//					write(uart0_filestream, essid, strlen(essid));
	//					write(uart0_filestream, "\r", 1);
					}
					else if (strcmp(CMD_WIFI_SCAN, rx_buffer) == 0)
					{
              scan_info_def *scan_results;
              int size_results;

              if (wifi_scan(&scan_results, &size_results) == 0)
              {
                int i;
                for (i = 0; i < size_results; i++)
                {
                  write(uart0_filestream, "ssid=", strlen("ssid="));
                  write(uart0_filestream, scan_results[i].SSID, strlen(scan_results[i].SSID));
                  write(uart0_filestream, ",", 1);
                  write(uart0_filestream, "mac_addr=", strlen("mac_addr="));
                  write(uart0_filestream, scan_results[i].mac_addr, strlen(scan_results[i].mac_addr));
                  write(uart0_filestream, ",", 1);
                  write(uart0_filestream, "quality=", strlen("quality="));
                  write(uart0_filestream, scan_results[i].quality, strlen(scan_results[i].quality));
                  write(uart0_filestream, ",", 1);
                  memset(data_buffer,0,sizeof(data_buffer));
                  sprintf(data_buffer, "%d", scan_results[i].signal_level);
                  write(uart0_filestream, "signal_level=", strlen("signal_level="));
                  write(uart0_filestream, data_buffer, strlen(data_buffer));
                  write(uart0_filestream, ",", 1);
                  memset(data_buffer,0,sizeof(data_buffer));
                  sprintf(data_buffer, "%d", scan_results[i].channel);
                  write(uart0_filestream, "channel=", strlen("channel="));
                  write(uart0_filestream, data_buffer, strlen(data_buffer));
                  write(uart0_filestream, ",", 1);
                  memset(data_buffer,0,sizeof(data_buffer));
                  switch (scan_results[i].enc) {
                    case OPEN:
                      strcpy(data_buffer,"OPEN");
                      break;
                    case WEP:
                      strcpy(data_buffer,"WEP");
                      break;
                    case WPA:
                      strcpy(data_buffer,"WPA");
                      break;
                    case WPA2:
                      strcpy(data_buffer,"WPA2");
                      break;
                  }
                  write(uart0_filestream, "enc=", strlen("enc="));
                  write(uart0_filestream, data_buffer, strlen(data_buffer));
                  write(uart0_filestream, "\r", 1);

                }
              }

              if (scan_results != NULL)
              {
                printf("scan_results freed\n");
                free(scan_results);
              }

					}
          else if (strncmp(rx_buffer, CMD_WIFI_CONNET, strlen(CMD_WIFI_CONNET)) == 0)
          {
            char *loc1, *loc2;
            char ssid[MAX_SSID_LEN];
            char psk[MAX_PSK_LEN];
            loc1 = strstr(rx_buffer, " ");
            if (loc1 != NULL)
            {
              loc2 = strstr(loc1 + 1, " ");
              if (loc2 != NULL)
              {
                  printf("loc2-loc1=%d\n", loc2-loc1);
                  printf("loc2-rx_buffer=%d\n", loc2-rx_buffer);
                  printf("strlen(rx_buffer) - (loc2-rx_buffer)=%d\n", strlen(rx_buffer) - (loc2-rx_buffer));
                  memset(ssid,0,sizeof(ssid));
                  memcpy(ssid,loc1 + 1, loc2-loc1 - 1);
                  memset(psk,0,sizeof(psk));
                  memcpy(psk,loc2 + 1, strlen(rx_buffer) - (loc2-rx_buffer) - 1);

                  printf("ssid=%s\n", ssid);
                  printf("psk=%s\n", psk);

                  int status = connect_to_wifi(ssid,psk);
                  switch (status) {
                    case 0:
                      write(uart0_filestream, "Connected with internet\r", strlen("Connected with internet\r"));
                      break;
                    case 1:
                      write(uart0_filestream, "Connected no internet\r", strlen("Connected no internet\r"));
                      break;
                    default:
                      write(uart0_filestream, "Credentials error\r", strlen("Credentials error\r"));
                      break;
                  }

              }
            }


          }

					index = 0;
					memset(rx_buffer, 0, sizeof(rx_buffer));
				}
			}
		}
	}

	return 0;
}
