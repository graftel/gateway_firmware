#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <temp_probe_RS485_tsys01.h>
#include <ble_data_acq.h>

int sock;
static int ble_cmd_len;
static char ble_cmd[MAX_BLE_PACKET_LEN];
static int ble_resp_len;
static char ble_resp[MAX_BLE_PACKET_LEN];
int cur_size_err;
static const uint8_t BLE_UUID_OUR_CHARACTERISTC_READ_UUID[]  = {0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x55, 0x47, 0x00, 0x00};
static const uint8_t BLE_UUID_OUR_CHARACTERISTC_WRITE_UUID[] = {0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x56, 0x47, 0x00, 0x00};

static int set_sec_level(int sock, int level)
{
	struct bt_security sec;

	memset(&sec, 0, sizeof(sec));
	sec.level = level;

	if (setsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &sec,
							sizeof(sec)) == 0)
		return 0;

	return -1;
}

int connect_ble(char *addr)   // set up connection
{
	bdaddr_t sba;
	struct sockaddr_l2 addr_des, addr_src;
	uint16_t cid = ATT_CID;
	uint8_t  dst_type = BDADDR_LE_RANDOM;
	uint8_t  src_type = BDADDR_LE_PUBLIC;
//	int try = RETRY_TIMES;
//	int status;
	int sec_level = BT_SECURITY_LOW;
//	fd_set fdset;
//   struct timeval tv;
//	long arg;
	 struct timeval timeout;
    timeout.tv_sec = BLE_TIME_OUT;
    timeout.tv_usec = 0;

	DEBUG_PRINT("BLE addr = %s\n",addr);

	bacpy(&sba, BDADDR_ANY);
	memset(&addr_src, 0, sizeof(addr_src));
	addr_src.l2_family = AF_BLUETOOTH;
	bacpy(&addr_src.l2_bdaddr, &sba);
	addr_src.l2_cid = htobs(cid);
	addr_src.l2_bdaddr_type = src_type;

	DEBUG_PRINT("connecting to BLE device\n");

	sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sock < 0)
	{
		DEBUG_PRINT("ERROR: socket failed\n");
		goto FAIL;
	}

	if (bind(sock, (struct sockaddr *) &addr_src, sizeof(addr_src)) < 0) {
		DEBUG_PRINT("ERROR: BIND failted\n");
		goto FAIL;
	}

	if (set_sec_level(sock, sec_level) < 0)
	{
		DEBUG_PRINT("ERROR: sec_level failted\n");
		goto FAIL;
	}

	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

  if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
      perror("setsockopt failed\n");
	//set non blocking
//	arg = fcntl(sock, F_GETFL, NULL);
//	arg |= O_NONBLOCK;
//	fcntl(sock, F_SETFL, arg);

	 // set the connection parameters (who to connect to)
	memset(&addr_des, 0, sizeof(addr_des));
	addr_des.l2_family = AF_BLUETOOTH;
	str2ba( addr, &addr_des.l2_bdaddr );
	addr_des.l2_cid = htobs(cid);
	addr_des.l2_bdaddr_type = dst_type;

	if (connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des)) == 0)
	{
		DEBUG_PRINT("connect ok\n");
		goto SUCCESS;
	}


	DEBUG_PRINT("timeout\n");
	goto FAIL;

//	FD_ZERO(&fdset);
//   FD_SET(sock, &fdset);
//    tv.tv_sec = BLE_TIME_OUT;             /*timeout */
 //   tv.tv_usec = 0;

	// if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1)
	// {
		// int so_error;
        // socklen_t len = sizeof so_error;

        // getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

        // if (so_error == 0) {
            // DEBUG_PRINT("connect ok\n");
			// arg = fcntl(sock, F_GETFL, NULL);
			// arg &= (~O_NONBLOCK);
			// fcntl(sock, F_SETFL, arg);
			// goto SUCCESS;
        // }

//	}



	// try = RETRY_TIMES;
	// while(try > 0 && connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des)) != 0)
	// {
		// try--;
	// }

	// DEBUG_PRINT("connecting to BLE device3\n");
	// if (try == 0)
	// {
		// DEBUG_PRINT("connect Retry out, cannot connect\n");

		// goto FAIL;
	// }
	// else
	// {
		// DEBUG_PRINT("connect succeed!\n");
	// }


SUCCESS:
//	close(sock);
	return 0;
FAIL:
	close(sock);
	return -1;
}

int write_ble_data(char *data, int datalen)
{
	int i;
	int try = RETRY_TIMES;

	DEBUG_PRINT("Write Data:\n");
	for (i = 0; i < datalen;i++)
	{
		DEBUG_PRINT("%02x ",data[i]);
	}

	DEBUG_PRINT("\n");

	while(try > 0 && write(sock, data, datalen) < 0)
	{
		try--;
	}
 //   status = connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des));

	if (try == 0)
	{
		DEBUG_PRINT("Write try out, cannot write\n");

		goto FAIL;
	}
	else
	{
		DEBUG_PRINT("wrtie succeed!\n");
	}

	return 0;
FAIL:
	close(sock);
	return -1;
}

int read_ble_data(char *data, int *datalen)
{
	int read_size = 0;
	int i;
//	bzero(data,sizeof(data));
	char read_buf[MAX_BLE_PACKET_LEN];

	read_size = read( sock, read_buf, sizeof(read_buf) ); /* there was data to read */

	//DEBUG_PRINT("status=%d\n",read_size);

	(*datalen) = read_size;

	if (read_size > 0)
	{

		memcpy(data, read_buf, read_size);

		DEBUG_PRINT("read data size=%d\n",read_size);

		i = 0;
		while(i < read_size)
		{
			DEBUG_PRINT("%02x ", read_buf[i]);
			i++;
		}
		DEBUG_PRINT("\n");


	}
	else
	{
		goto FAIL;
	}


	return 0;
FAIL:
	close(sock);
	return -1;
}

data_code_def send_ble_cmd_with_response(char *cmd, int cmd_len, char *resp, int *resp_len, core_module *cm)
{
	int i;
	data_code_def err_no;
	char int_wdata[MAX_BLE_PACKET_LEN];
	int int_wcmd_len = 0;

	char int_rdata[MAX_BLE_PACKET_LEN];
	int int_rcmd_len = 0;

	int retry = FAST_RETRY_TIMES;

	int read_status = 0;
	DEBUG_PRINT("ble_cmd: ");
	for (i = 0; i < cmd_len; i++)
	{
		DEBUG_PRINT("0x%02x ",cmd[i]);
	}
	DEBUG_PRINT("\n");

	memset(int_wdata,0,sizeof(int_wdata));

	int_wdata[0] = 0x12;
	int_wdata[1] = cm->write_handle;
	int_wdata[2] = 0x00;

	int_wcmd_len = cmd_len + 3;

	memcpy(int_wdata + 3, cmd, cmd_len);

	// Write Char Request
	if (write_ble_data(int_wdata, int_wcmd_len) != 0 )
	{
		err_no = BLE_WRITE_ERR;
		goto FAIL;
	}

	//verify write success
	memset(int_rdata,0,sizeof(int_rdata));
	if (read_ble_data(int_rdata, &int_rcmd_len) != 0 )
	{
		DEBUG_PRINT("read_error_1\n");
		err_no = BLE_READ_ERR;
		goto FAIL;
	}

	if (!(int_rcmd_len == 1 && int_rdata[0] == 0x13))
	{
		DEBUG_PRINT("read_error_2\n");
		err_no = BLE_READ_ERR;
		goto FAIL;
	}

	while(retry > 0 && read_status == 0)
	{
		DEBUG_PRINT("enter while loop\n");
		usleep(1000);

		memset(int_wdata,0,sizeof(int_wdata));

		int_wdata[0] = 0x0a;
		int_wdata[1] = cm->read_handle;
		int_wdata[2] = 0x00;

		int_wcmd_len = 3;

		if (write_ble_data(int_wdata, int_wcmd_len) != 0 )
		{
			err_no = BLE_WRITE_ERR;
			goto FAIL;
		}

		memset(int_rdata,0,sizeof(int_rdata));

		if (read_ble_data(int_rdata, &int_rcmd_len) != 0 )
		{

			err_no = BLE_READ_ERR;
			goto FAIL;
		}

		// verify send data and resp data
		if (memcmp(cmd + 3, int_rdata + 4, MAX_SENSOR_ADDR) == 0
			&& int_rdata[0] == 0x0b)
		{
			DEBUG_PRINT("Address verified\n");
			read_status = 1;
		}


		retry--;
	}

	if (retry == 0)
	{

		DEBUG_PRINT("no response or address not match, retry out\n");
		err_no = BLE_INTERNAL_CMD_NO_RESPONSE;
		goto FAIL;
	}
	else
	{
		(*resp_len) = int_rcmd_len - 1;
		memcpy(resp, int_rdata + 1, int_rcmd_len - 1);

	}


return 0;

FAIL:
	return err_no;
}


int enc_discover_cmd(uint16_t start, uint16_t end, uint8_t *buf, int *len)
{


	/* Attribute Opcode (1 octet) */
	buf[0] = ATT_OP_READ_BY_TYPE_REQ;

	buf[1] = (uint8_t)start;

	buf[2] = (uint8_t)(start >> 8);

	buf[3] = (uint8_t)end;

	buf[4] = (uint8_t)(end >> 8);

	buf[5] = (uint8_t)GATT_CHARAC_UUID;

	buf[6] = (uint8_t)(GATT_CHARAC_UUID >> 8);

	(*len) = 7;

	return 0;
}

data_code_def dicover_char(core_module *cm) // search handle from 0x0001 to 0xffff
{
	uint8_t buf[MAX_BLE_PACKET_LEN], read_buf[MAX_BLE_PACKET_LEN];
	int plen = 0, rlen = 0, block_size;
	data_code_def err_no = OK;
	memset(buf, 0, sizeof(buf));

	uint16_t start = 0x0001;
	uint16_t end = 0xffff;

	read_buf[0] = 0x00;


	while(read_buf[0] != ATT_OP_ERROR)
	{
		enc_discover_cmd(start, end, buf, &plen);

		if (write_ble_data((char*)buf, plen) != 0 )
		{
			err_no = BLE_WRITE_ERR;
			goto FAIL;
		}

		memset(read_buf,0,sizeof(read_buf));
		if (read_ble_data((char*)read_buf, &rlen) != 0 )
		{
			DEBUG_PRINT("read_error_1\n");
			err_no = BLE_READ_ERR;
			goto FAIL;
		}

		block_size = read_buf[1];

		if (read_buf[0] == ATT_OP_ERROR)
		{
			err_no = OK;
			goto SUCCESS;
		}

		if ((rlen - 2) / block_size >=1)
		{
			DEBUG_PRINT("discover ok\n");
			if (block_size - 5 == MAX_UUID_LEN) // maybe what we need
			{

				if (memcmp(read_buf + 7, BLE_UUID_OUR_CHARACTERISTC_READ_UUID, MAX_UUID_LEN) == 0)
				{
					 DEBUG_PRINT("found read handle\n");
					 cm->read_handle = (read_buf[6] << 8) | (read_buf[5]);
					 DEBUG_PRINT("0x%04x\n", cm->read_handle);
					 cm->discovered++;
				}
				else if (memcmp(read_buf + 7, BLE_UUID_OUR_CHARACTERISTC_WRITE_UUID, MAX_UUID_LEN) == 0)
				{
					DEBUG_PRINT("found write handle: ");
					cm->write_handle = (read_buf[6] << 8) | (read_buf[5]);
					DEBUG_PRINT("0x%04x\n", cm->write_handle);
					cm->discovered++;
				}

			}

			if (block_size == 0x07)
			{
				start = (read_buf[rlen - 3] << 8) | read_buf[rlen - 4];
			}
			else if (block_size == 0x15)
			{
				start = (read_buf[rlen - MAX_UUID_LEN - 1] << 8) | read_buf[rlen - MAX_UUID_LEN - 2];
			}
			else if (block_size == 0x08)
			{
				err_no = OK;
				goto SUCCESS;
			}
			else
			{
				DEBUG_PRINT("Unknown cmd\n");
				err_no = BLE_READ_ERR;
				goto FAIL;
			}

			DEBUG_PRINT("start=0x%04x\n",start);
		}
		else
		{
			DEBUG_PRINT("Read error cmd\n");
			err_no = BLE_READ_ERR;
			goto FAIL;
		}

	}

SUCCESS:
	return err_no;
FAIL:
	return err_no;
}

int ble_data_acq(core_module *cm)
{
		int i;

		DEBUG_PRINT3("start BLE read, ID=%s,address=%s\n",cm->addr,cm->ble_addr);

		if (connect_ble(cm->ble_addr) == 0){
					for (i = 0; i < cm->size_sen; i++)
					{
								if (cm->discovered == 0)
								{
										DEBUG_PRINT3("start finding desc\n");
										if (dicover_char(cm) == OK)
										{
											DEBUG_PRINT3("desc find ok\n");
										}
										else
										{
											DEBUG_PRINT3("desc find error\n");
										}
								}

								if (cm->discovered == 2)
								{

										DEBUG_PRINT3("start read sensor #%d\n", i);
									// rounting sensor protocol functions
										if (strncmp(cm->sen[i].addr,"02",2) == 0)	// RS485 TSYS01 Probe
										{
												DEBUG_PRINT("detect tsys01 sensor\n");
												sensor *sen = &cm->sen[i];
												if(get_ble_cmd_temp_probe_RS485_tsys01(sen, ble_cmd, &ble_cmd_len) != 0)
												{
													 DEBUG_PRINT("cannot get ble command\n");
													 goto FAIL;
												}

												cm->sen[i].data_code = send_ble_cmd_with_response(ble_cmd, ble_cmd_len, ble_resp, &ble_resp_len, cm);

												if (cm->sen[i].data_code == OK)
												{
													cm->sen[i].data_code = process_ble_resp_temp_probe_RS485_tsys01(sen, ble_resp, ble_resp_len);
												}

												if (cm->sen[i].data_code != OK)
												{
													DEBUG_PRINT("data error\n");
													cm->sen[i].data = ERROR_DATA_VALUE;
												}

												DEBUG_PRINT("data ok\n");
										}
										else if (strncmp(cm->sen[i].addr,"03", 2) == 0)	// 12-CH 10K Probe
										{


										}
										else																			// Unknown probe
										{
											cm->sen[i].data = ERROR_DATA_VALUE;
											cm->sen[i].data_code = UNKNOWN_PROBE_DEFINITION;
										}
								}




					}
		}
		else{
				DEBUG_PRINT("ble connect timeout\n");
				for (i = 0; i < cm->size_sen; i++)
				{
						cm->sen[i].data = ERROR_DATA_VALUE;
						cm->sen[i].data_code = BLE_CONN_TIMEOUT;
				}
				goto FAIL;
		}

		close(sock);
		return 0;
FAIL:
		close(sock);
		return -1;
}
