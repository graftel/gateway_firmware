#include "rs485_acq.h"


static int uart_cmd_len;
static char uart_cmd[MAX_UART_PACKET_LEN];
static int uart_resp_len;
static char uart_resp[MAX_UART_PACKET_LEN];

void write_with_debug(int fd, char *buffer, int len)
{
	if (write(fd, buffer, len) != 0)
	{
		DEBUG_PRINT("UART write failed\n");
	}
}

int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                DEBUG_PRINT ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                DEBUG_PRINT ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                DEBUG_PRINT ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                DEBUG_PRINT ("error %d setting term attributes", errno);
}

data_code_def read_uart_with_response(char *cmd, int cmd_len, char *resp, int *resp_len)
{
    int uart0_filestream = -1;
    int read_status = 0;
    data_code_def err_no;
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
    uart0_filestream = open(UART_DEV_NAME, O_RDWR | O_NOCTTY | O_SYNC);		//Open in non blocking read/write mode
  	if (uart0_filestream < 0)
  	{
  		//ERROR - CAN'T OPEN SERIAL PORT
  		DEBUG_PRINT("Error - Unable to open UART.  Ensure it is not in use by another application\n");
			err_no = PROTOCOL_UART_INTERNAL_ERR;
			goto FAIL;
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
    set_interface_attribs (uart0_filestream, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (uart0_filestream, 0);                // set no blocking

    DEBUG_PRINT("uart_cmd: ");
    int i;
  	for (i = 0; i < cmd_len; i++)
  	{
  		DEBUG_PRINT("0x%02x ",cmd[i]);
  	}
  	DEBUG_PRINT("\n");

    write(uart0_filestream, cmd, cmd_len);
    char rx_buffer[MAX_UART_PACKET_LEN];
    int retry = FAST_RETRY_TIMES;
    int read_size;
	 	uint8_t x = 0;
		int index = 0;
		int read_time_out;
		while(retry > 0 && read_status == 0)
		{
			DEBUG_PRINT("enter while retry loop\n");
			DEBUG_PRINT("retry=%d\n",retry);

			read_time_out = 1000;
			index = 0;
			while (x != 0x0d && index < MAX_UART_PACKET_LEN && read_time_out > 0)
			{
				if(read (uart0_filestream, &x, 1) > 0)
				{
					rx_buffer[index] = x;
					index++;
				}
				usleep(1000);
				read_time_out--;
			}

			if (index > 0)
			{
				read_size = index;

				memcpy(resp, rx_buffer, read_size);

				DEBUG_PRINT("read data size=%d\n",read_size);

				i = 0;
				while(i < read_size)
				{
					DEBUG_PRINT("%02x ", rx_buffer[i]);
					i++;
				}
				DEBUG_PRINT("\n");

				(*resp_len) = read_size;
				// validate resp'

				char *chr_pos = strchr((char*)rx_buffer,'#');
				int sharp_pos = 0;

				if (chr_pos == NULL)
				{
					DEBUG_PRINT("resp error\n");
				}
				else{
					sharp_pos = (int)(chr_pos - (char *)rx_buffer);
					if (memcmp(cmd + 1, resp + sharp_pos + 1, UART_I2C_PROBE_CMD_LENGTH - 2) == 0)
					{
						DEBUG_PRINT("Address verified\n");
						read_status = 1;
					}
				}
			}
			retry--;
		}

    if (retry == 0)
  	{

  		DEBUG_PRINT("no response or address not match, retry out\n");
  		err_no = PROTOCOL_UART_NO_RESPONSE;
  		goto FAIL;
  	}

close(uart0_filestream);
return OK;

FAIL:
  close(uart0_filestream);
	return err_no;
}

int acq_via_rs485(core_module *cm)
{
  DEBUG_PRINT3("start Rs485 read, ADDR=%s\n",cm->addr);
  data_code_def err_no;
  int i;

  for (i = 0; i < cm->size_sen; i++)
  {

    if (strncmp(cm->sen[i].addr,SENSOR_TEMP_TSYS01,strlen(SENSOR_TEMP_TSYS01)) == 0)	// RS485 TSYS01 Probe
    {
        DEBUG_PRINT("detect tsys01 sensor\n");
        sensor *sen = &cm->sen[i];
        if(get_uart_cmd_temp_probe_RS485_tsys01(sen, uart_cmd, &uart_cmd_len, cm->addr) != 0)
        {
           DEBUG_PRINT("cannot get uart command\n");
           goto FAIL;
        }

        cm->sen[i].data_code = read_uart_with_response(uart_cmd, uart_cmd_len, uart_resp, &uart_resp_len);

        if (cm->sen[i].data_code == OK)
        {
          DEBUG_PRINT("data ok\n");
          cm->sen[i].data_code = process_uart_resp_temp_probe_RS485_tsys01(sen, uart_resp, uart_resp_len);
        }

        if (cm->sen[i].data_code != OK)
        {
          DEBUG_PRINT("data error\n");
          cm->sen[i].data = ERROR_DATA_VALUE;
        }

//        DEBUG_PRINT("data ok\n");
    }
		else if (strncmp(cm->sen[i].addr,SENSOR_TEMP_12_CH,strlen(SENSOR_TEMP_12_CH)) == 0)	// RS485 TSYS01 Probe
    {
        DEBUG_PRINT("detect 12_CH sensor\n");
        sensor *sen = &cm->sen[i];
        if(get_uart_cmd_12_ch(sen, uart_cmd, &uart_cmd_len, cm->addr) != 0)
        {
           DEBUG_PRINT("cannot get uart command\n");
           goto FAIL;
        }

        cm->sen[i].data_code = read_uart_with_response(uart_cmd, uart_cmd_len, uart_resp, &uart_resp_len);

        if (cm->sen[i].data_code == OK)
        {
          DEBUG_PRINT("data ok\n");
          cm->sen[i].data_code = process_uart_resp_12_ch(sen, uart_resp, uart_resp_len);
        }

        if (cm->sen[i].data_code != OK)
        {
          DEBUG_PRINT("data error\n");
          cm->sen[i].data = ERROR_DATA_VALUE;
        }

//        DEBUG_PRINT("data ok\n");
    }
		else if (strncmp(cm->sen[i].addr,SENSOR_DP,strlen(SENSOR_DP)) == 0)	// RS485 TSYS01 Probe
		{
				DEBUG_PRINT("detect 12_CH sensor\n");
				sensor *sen = &cm->sen[i];
				if(get_uart_cmd_dp_sensor(sen, uart_cmd, &uart_cmd_len, cm->addr) != 0)
				{
					 DEBUG_PRINT("cannot get uart command\n");
					 goto FAIL;
				}

				cm->sen[i].data_code = read_uart_with_response(uart_cmd, uart_cmd_len, uart_resp, &uart_resp_len);

				if (cm->sen[i].data_code == OK)
				{
					DEBUG_PRINT("data ok\n");
					cm->sen[i].data_code = process_uart_resp_dp_sensor(sen, uart_resp, uart_resp_len);
				}

				if (cm->sen[i].data_code != OK)
				{
					DEBUG_PRINT("data error\n");
					cm->sen[i].data = ERROR_DATA_VALUE;
				}

//        DEBUG_PRINT("data ok\n");
		}
    else																			// Unknown probe
    {
      cm->sen[i].data = ERROR_DATA_VALUE;
      cm->sen[i].data_code = SENSOR_GENERAL_UNKNOWN_DEFINITION;
    }

  }

  return 0;
FAIL:
  return err_no;
}
