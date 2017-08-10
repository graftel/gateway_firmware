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


#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <ble_data_acq.h>
#include <core_module_command_util.h>
#include <simulation.h>
#define ATT_CID					4
#define ATT_PSM					31
#define READ_HANDLE				0x0E
#define WRITE_HANDLE			0x11

#define RETRY_TIMES 5
#define FAST_RETRY_TIMES 100
#define BLE_TIME_OUT	10
int sock;
static int ble_cmd_len;
static char ble_cmd[MAX_BLE_PACKET_LEN];
static int ble_resp_len;
static char ble_resp[MAX_BLE_PACKET_LEN];
err_code err;
int cur_size_err;

static int set_sec_level(int sock, int level)
{
	struct bt_security sec;
	int ret;

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
	int try = RETRY_TIMES;
	int status;
	int sec_level = BT_SECURITY_LOW;
//	fd_set fdset;
//   struct timeval tv;
//	long arg;
	 struct timeval timeout;      
    timeout.tv_sec = BLE_TIME_OUT;
    timeout.tv_usec = 0;
	
	printf("BLE addr = %s\n",addr);
	
	bacpy(&sba, BDADDR_ANY);
	memset(&addr_src, 0, sizeof(addr_src));
	addr_src.l2_family = AF_BLUETOOTH;
	bacpy(&addr_src.l2_bdaddr, &sba);
	addr_src.l2_cid = htobs(cid);
	addr_src.l2_bdaddr_type = src_type;
	
	g_print("connecting to BLE device1\n");
	
	sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sock < 0)
	{
		printf("ERROR: socket failed\n");
		goto FAIL;
	}
		
	if (bind(sock, (struct sockaddr *) &addr_src, sizeof(addr_src)) < 0) {
		printf("ERROR: BIND failted\n");
		goto FAIL;
	}
		
	if (set_sec_level(sock, sec_level) < 0)
	{
		printf("ERROR: sec_level failted\n");
		goto FAIL;
	}
	
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        error("setsockopt failed\n");

    if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        error("setsockopt failed\n");
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
	
	g_print("connecting to BLE device2\n");
	
	if (connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des)) == 0)
	{
		g_print("connect ok\n");
		goto SUCCESS;
	}
	
	
	g_print("timeout\n");
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
            // g_print("connect ok\n");
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

	// g_print("connecting to BLE device3\n");
	// if (try == 0)
	// {
		// printf("connect Retry out, cannot connect\n");
		
		// goto FAIL;
	// }
	// else
	// {
		// printf("connect succeed!\n");
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
	
	printf("Write Data:\n");
	for (i = 0; i < datalen;i++)
	{
		printf("%02x ",data[i]);
	}
	
	printf("\n");
	
	while(try > 0 && write(sock, data, datalen) < 0)
	{
		try--;
	}
 //   status = connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des));
 
	if (try == 0)
	{
		printf("Write try out, cannot write\n");
	
		goto FAIL;
	}
	else
	{
		printf("wrtie succeed!\n");
	}
	
SUCCESS:
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
	
	//printf("status=%d\n",read_size);
	
	(*datalen) = read_size;
	
	if (read_size > 0)
	{
		
		memcpy(data, read_buf, read_size);
		
		printf("read data size=%d\n",read_size);
		
		i = 0;
		while(i < read_size)
		{
			printf("%02x ", read_buf[i]);
			i++;
		}
		printf("\n");
		
		
	}
	
	
SUCCESS:
	return 0;
FAIL:
	close(sock);
	return -1;
}

int send_ble_cmd_with_response(char *cmd, int cmd_len, char *resp, int *resp_len)
{
	int i;
	int err_no = -1;
	char int_wdata[MAX_BLE_PACKET_LEN];
	int int_wcmd_len = 0;
	
	char int_rdata[MAX_BLE_PACKET_LEN];
	int int_rcmd_len = 0;
	
	int retry = FAST_RETRY_TIMES;
	
	int read_status = 0;
	g_print("ble_cmd: ");
	for (i = 0; i < cmd_len; i++)
	{
		g_print("0x%02x ",cmd[i]);
	}
	g_print("\n");
	
	memset(int_wdata,0,sizeof(int_wdata));
	
	int_wdata[0] = 0x12;
	int_wdata[1] = WRITE_HANDLE;
	int_wdata[2] = 0x00;
	
	int_wcmd_len = cmd_len + 3;
	
	memcpy(int_wdata + 3, cmd, cmd_len);
	
	// Write Char Request
	if (write_ble_data(int_wdata, int_wcmd_len) != 0 )
	{
		err_no = -1;
		goto FAIL;
	}
	
	//verify write success
	memset(int_rdata,0,sizeof(int_rdata));
	if (read_ble_data(int_rdata, &int_rcmd_len) != 0 )
	{
		err_no = -2;
		goto FAIL;
	}
	
	if (!(int_rcmd_len == 1 && int_rdata[0] == 0x13))
	{
		err_no = -2;
		goto FAIL;
	}
	
	while(retry > 0 && read_status == 0)
	{
		usleep(1000);
		
		memset(int_wdata,0,sizeof(int_wdata));
	
		int_wdata[0] = 0x0a;
		int_wdata[1] = READ_HANDLE;
		int_wdata[2] = 0x00;
		
		int_wcmd_len = 3;
		
		if (write_ble_data(int_wdata, int_wcmd_len) != 0 )
		{
//			err = RS485_TSYS01_WRITE_ERR;
//			cur_size_err = bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err;
//			bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].err_list[cur_size_err] = (int)err;
//			bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err++;
			err_no = -1;
			goto FAIL;
		}
		
		memset(int_rdata,0,sizeof(int_rdata));
		
		if (read_ble_data(int_rdata, &int_rcmd_len) != 0 )
		{
//			err = RS485_TSYS01_READ_ERR;
//			cur_size_err = bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err;
//			bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].err_list[cur_size_err] = (int)err;
//			bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err++;
			err_no = -2;
			goto FAIL;
		}
		
		// verify send data and resp data
		if (memcmp(cmd + 3, int_rdata + 4, MAX_SENSOR_ADDR) == 0
			&& int_rdata[0] == 0x0b)
		{
			g_print("Address verified\n");
			read_status = 1;
		}
		
		
		retry--;
	}

	if (retry == 0)
	{

		g_print("no response or address not match, retry out\n");
		err_no = -3;
		goto FAIL;
	}
	else
	{	
		(*resp_len) = int_rcmd_len - 1;
		memcpy(resp, int_rdata + 1, int_rcmd_len - 1);
		
	}
	
SUCCESS:
	return 0;
FAIL:
	return err_no;
}

int read_all_data_on_single_core_module(bridge *bridge_data)
{
	int i;
	int err1;
	float temp_value;
	if (connect_ble(bridge_data->cm[bridge_data->index_cm].ble_addr) == 0)
	{
		// first read temperature
		bridge_data->cm[bridge_data->index_cm].index_sen = 0;
		
		while(bridge_data->cm[bridge_data->index_cm].index_sen < bridge_data->cm[bridge_data->index_cm].size_sen)
		{	
			g_print("reading sensor #%d\n",bridge_data->cm[bridge_data->index_cm].index_sen);
		
			if (bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].proto == RS485)
			{
				if (get_ble_cmd(bridge_data, ble_cmd, &ble_cmd_len) != 0)
				{
					g_print("cannot get ble cmd\n");
					goto FAIL;
				}
				
				err1 = send_ble_cmd_with_response(ble_cmd, ble_cmd_len, ble_resp, &ble_resp_len);
				if (err1 != 0){
					g_print("read and resp failed\n");
					if (err == -1)
					{
						err = RS485_TSYS01_WRITE_ERR;
						cur_size_err = bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].err_list[cur_size_err] = (int)err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err++; 
					}
					else if (err == -2)
					{
						err = RS485_TSYS01_READ_ERR;
						cur_size_err = bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].err_list[cur_size_err] = (int)err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err++; 
					}
					else if (err == -3)
					{
						err = RS485_TSYS01_ADDRESS_VERIFY_ERR;
						cur_size_err = bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].err_list[cur_size_err] = (int)err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err++;
					}
					goto FAIL;
				}
				
				if (ble_resp_len > 0)
				{
					
					// check response code
					if (ble_resp[2] == 0x41)
					{
						g_print("read ok\n");
						memcpy(&temp_value, ble_resp + 9, sizeof(float));
						g_print("temp_reading=%f\n",temp_value);
						
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].data = temp_value;
					}
					else{
						err = RS485_TSYS01_NO_RESPONSE;
						cur_size_err = bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].err_list[cur_size_err] = (int)err;
						bridge_data->cm[bridge_data->index_cm].sen[bridge_data->cm[bridge_data->index_cm].index_sen].size_err++;
						g_print("read error, no response read sensor in RS485\n");
					}
				}
			}

			
			bridge_data->cm[bridge_data->index_cm].index_sen++;

		}

		
	}
	else{
		err = BLE_CONN_TIMEOUT;
		cur_size_err = bridge_data->cm[bridge_data->index_cm].size_err;
		bridge_data->cm[bridge_data->index_cm].err_list[cur_size_err] = (int)err;
		bridge_data->cm[bridge_data->index_cm].size_err++;
		goto FAIL;
	}
	
	
SUCCESS:
	close(sock);
	return 0;
FAIL:
	close(sock);
	return -1;
}

int ble_data_acq(bridge *bridge_data)
{
	g_print("start_BLE_reading\n");
	int i = 0;
	bridge_data->index_cm = 0;
	while(bridge_data->index_cm < bridge_data->size_cm)
	{
		g_print("start reading core module #%d\n",bridge_data->index_cm);
		
		for (i = 0; i <bridge_data->cm[bridge_data->index_cm].size_sen; i++)
		{
			bridge_data->cm[bridge_data->index_cm].sen[i].data = DEFAULT_ERR_VALUE;
		}
		
		setup_simulation_data_for_current_cm(bridge_data);
		
		if (read_all_data_on_single_core_module(bridge_data) == 0)
		{
			g_print("ble read OK\n");
		}
		else{
			g_print("ble read failed\n");
		}
		bridge_data->index_cm++;
			
		usleep(500000);
	}
}
