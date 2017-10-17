#include <uart_module.h>
#include <defines.h>

int enc_status = 0;

int connect_to_wifi(const char *ssid, const char *psk)
{
	  FILE * pFile;
	  pFile = fopen ("/etc/network/interfaces","r");
//    size_t len = 0;
//	  ssize_t read;
		int res = 0;
	  char wpa_file_path[50];

	  if (pFile!=NULL)
	  {
			char *loc;

      fseek(pFile, 0, SEEK_END);
      long fsize = ftell(pFile);
      fseek(pFile, 0, SEEK_SET);  //same as rewind(f);

      char *string = malloc(fsize + 1);
      res = fread(string, fsize, 1, pFile);
			if (res == -1)
			{
				return -1;
			}
      fclose(pFile);

			loc = strstr(string, WIFI_INTERFACE_NAME);
			if (loc != NULL)
			{
        DEBUG_PRINT("found interface\n" );
			}
			loc = strstr(loc, "wpa-conf");
			if (loc != NULL)
			{
        DEBUG_PRINT("found wpa-conf\n");
				int j = 0;
				memset(wpa_file_path,0,sizeof(wpa_file_path));
				loc += 9;
        while(*(loc + j) != 0x0a)
				{
					memcpy(wpa_file_path + j, loc + j, 1);
          j++;
				}
				DEBUG_PRINT("wpa_file_path=%s\n",wpa_file_path);
			}
	  }

    char cmd[BUFSIZE];

    strcpy(cmd, "wpa_passphrase ");
    strcat(cmd, ssid);
    strcat(cmd, " ");
    strcat(cmd, psk);
    strcat(cmd, " > ");
    strcat(cmd, wpa_file_path);
    strcat(cmd, " && ifdown ");
		strcat(cmd, WIFI_INTERFACE_NAME);
		strcat(cmd, " && ifup ");
		strcat(cmd, WIFI_INTERFACE_NAME);

    DEBUG_PRINT("write cmd = %s\n", cmd);

    char buf[BUFSIZE];
    FILE *fp;
    char getSSID[MAX_SSID_LEN];
    if ((fp = popen(cmd, "r")) == NULL) {
        DEBUG_PRINT("Error opening pipe!\n");
        return -1;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
        DEBUG_PRINT("%s\n", buf);
    }

    if(pclose(fp))  {
        DEBUG_PRINT("Command not found or exited with error status\n");
    }

    usleep(5000000);

    int connected = 0;

    memset(cmd,0,sizeof(cmd));

    strcpy(cmd, "iwconfig ");
		strcat(cmd, WIFI_INTERFACE_NAME);

    int retry = RETRY_TIMES;

    while (1)
    {
      if ((fp = popen(cmd, "r")) == NULL) {
          DEBUG_PRINT("Error opening pipe!\n");
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
          DEBUG_PRINT("Command not found or exited with error status\n");
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
          DEBUG_PRINT("connected\n");
          break;
        }
      }
      else
      {
        connected = 0;
        retry--;
        if (retry == 0)
        {
          DEBUG_PRINT("timeout, cannot connect, recheck password\n");
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
	char cmd[CMD_LINE_SIZE];

	strcpy(cmd, "iwlist ");
	strcat(cmd, WIFI_INTERFACE_NAME);
	strcat(cmd, " scan");

  char buf[BUFSIZE];
  FILE *fp;

  if ((fp = popen(cmd, "r")) == NULL) {
      DEBUG_PRINT("Error opening pipe!\n");
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
  DEBUG_PRINT("******************* Results START *******************\n");
  for (i = 0; i < num_cell; i++)
  {
    DEBUG_PRINT("cell #%d\n", i);
    DEBUG_PRINT("ssid=%s\n", (*scan_info)[i].SSID);
    DEBUG_PRINT("mac_addr=%s\n", (*scan_info)[i].mac_addr);
    DEBUG_PRINT("quality=%s\n", (*scan_info)[i].quality);
    DEBUG_PRINT("signal_level=%d\n", (*scan_info)[i].signal_level);
    DEBUG_PRINT("channel=%d\n", (*scan_info)[i].channel);
    DEBUG_PRINT("enc=%d\n", (*scan_info)[i].enc);
    DEBUG_PRINT("------------------------------------------------\n");
  }
  DEBUG_PRINT("******************* Results END *******************\n");

  if(pclose(fp))  {
      DEBUG_PRINT("Command not found or exited with error status\n");
      return -1;
  }

	return 0;
}

void write_with_debug(int fd, char *buffer, int len)
{
	if (write(fd, buffer, len) != 0)
	{
		DEBUG_PRINT("UART write failed\n");
	}
}

void *uart_wrapper(void *args)
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

	uart0_filestream = open(UART_DEV_NAME, O_RDWR | O_NOCTTY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		DEBUG_PRINT("Error - Unable to open UART.  Ensure it is not in use by another application\n");
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
	char rx_buffer[MAX_BUFFER_SIZE];
	char data_buffer[MAX_BUFFER_SIZE];
	int index = 0;
	uint8_t x;
	// parse command end with /r 13
	memset(rx_buffer, 0, sizeof(rx_buffer));

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
					write_with_debug(uart0_filestream, rx_buffer, strlen(rx_buffer));
					write_with_debug(uart0_filestream, "=", 1);
					write_with_debug(uart0_filestream, data_buffer, strlen(data_buffer));
					write_with_debug(uart0_filestream, "\r", 1);
				}
				else if (strcmp(CMD_WIFI_GET_SSID, rx_buffer) == 0)
				{

						write_with_debug(uart0_filestream, rx_buffer, strlen(rx_buffer));
						write_with_debug(uart0_filestream, "=", 1);
		//				write_with_debug(uart0_filestream, essid, strlen(essid));
						write_with_debug(uart0_filestream, "\r", 1);
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
                write_with_debug(uart0_filestream, "ssid=", strlen("ssid="));
                write_with_debug(uart0_filestream, scan_results[i].SSID, strlen(scan_results[i].SSID));
                write_with_debug(uart0_filestream, ",", 1);
                write_with_debug(uart0_filestream, "mac_addr=", strlen("mac_addr="));
                write_with_debug(uart0_filestream, scan_results[i].mac_addr, strlen(scan_results[i].mac_addr));
                write_with_debug(uart0_filestream, ",", 1);
                write_with_debug(uart0_filestream, "quality=", strlen("quality="));
                write_with_debug(uart0_filestream, scan_results[i].quality, strlen(scan_results[i].quality));
                write_with_debug(uart0_filestream, ",", 1);
                memset(data_buffer,0,sizeof(data_buffer));
                sprintf(data_buffer, "%d", scan_results[i].signal_level);
                write_with_debug(uart0_filestream, "signal_level=", strlen("signal_level="));
                write_with_debug(uart0_filestream, data_buffer, strlen(data_buffer));
                write_with_debug(uart0_filestream, ",", 1);
                memset(data_buffer,0,sizeof(data_buffer));
                sprintf(data_buffer, "%d", scan_results[i].channel);
                write_with_debug(uart0_filestream, "channel=", strlen("channel="));
                write_with_debug(uart0_filestream, data_buffer, strlen(data_buffer));
                write_with_debug(uart0_filestream, ",", 1);
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
                write_with_debug(uart0_filestream, "enc=", strlen("enc="));
                write_with_debug(uart0_filestream, data_buffer, strlen(data_buffer));
                write_with_debug(uart0_filestream, "\r", 1);

              }
            }

            if (scan_results != NULL)
            {
              DEBUG_PRINT("scan_results freed\n");
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
                DEBUG_PRINT("loc2-loc1=%d\n", loc2-loc1);
                DEBUG_PRINT("loc2-rx_buffer=%d\n", loc2-rx_buffer);
                DEBUG_PRINT("strlen(rx_buffer) - (loc2-rx_buffer)=%d\n", strlen(rx_buffer) - (loc2-rx_buffer));
                memset(ssid,0,sizeof(ssid));
                memcpy(ssid,loc1 + 1, loc2-loc1 - 1);
                memset(psk,0,sizeof(psk));
                memcpy(psk,loc2 + 1, strlen(rx_buffer) - (loc2-rx_buffer) - 1);

                DEBUG_PRINT("ssid=%s\n", ssid);
                DEBUG_PRINT("psk=%s\n", psk);

                int status = connect_to_wifi(ssid,psk);
                switch (status) {
                  case 0:
                    write_with_debug(uart0_filestream, "Connected with internet\r", strlen("Connected with internet\r"));
                    break;
                  case 1:
                    write_with_debug(uart0_filestream, "Connected no internet\r", strlen("Connected no internet\r"));
                    break;
                  default:
                    write_with_debug(uart0_filestream, "Credentials error\r", strlen("Credentials error\r"));
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
