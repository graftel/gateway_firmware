#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <libconfig.h>
#include <pthread.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>


#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>


#define ATT_OP_ERROR			0x01
#define ATT_OP_MTU_REQ			0x02
#define ATT_OP_MTU_RESP			0x03
#define ATT_OP_FIND_INFO_REQ		0x04
#define ATT_OP_FIND_INFO_RESP		0x05
#define ATT_OP_FIND_BY_TYPE_REQ		0x06
#define ATT_OP_FIND_BY_TYPE_RESP	0x07
#define ATT_OP_READ_BY_TYPE_REQ		0x08
#define ATT_OP_READ_BY_TYPE_RESP	0x09
#define ATT_OP_READ_REQ			0x0A
#define ATT_OP_READ_RESP		0x0B
#define ATT_OP_READ_BLOB_REQ		0x0C
#define ATT_OP_READ_BLOB_RESP		0x0D
#define ATT_OP_READ_MULTI_REQ		0x0E
#define ATT_OP_READ_MULTI_RESP		0x0F
#define ATT_OP_READ_BY_GROUP_REQ	0x10
#define ATT_OP_READ_BY_GROUP_RESP	0x11
#define ATT_OP_WRITE_REQ		0x12
#define ATT_OP_WRITE_RESP		0x13
#define ATT_OP_WRITE_CMD		0x52
#define ATT_OP_PREP_WRITE_REQ		0x16
#define ATT_OP_PREP_WRITE_RESP		0x17
#define ATT_OP_EXEC_WRITE_REQ		0x18
#define ATT_OP_EXEC_WRITE_RESP		0x19
#define ATT_OP_HANDLE_NOTIFY		0x1B
#define ATT_OP_HANDLE_IND		0x1D
#define ATT_OP_HANDLE_CNF		0x1E
#define ATT_OP_SIGNED_WRITE_CMD		0xD2

#define ATT_CID					4
#define ATT_PSM					31

double db_cur_time = 0.0;

const int RETRY_TIMES = 2;
const int MAX_PACKET_SIZE = 23;
char desc_addr[18];
int scan_err, scan_dev_id, scan_sock, read_sock;
uint8_t scan_type = 0x01;	//active
uint16_t scan_interval = htobs(0x0010);
uint16_t scan_window = htobs(0x0010);
uint8_t scan_filter_policy = 0x00;
uint8_t scan_own_type = LE_PUBLIC_ADDRESS;
uint8_t scan_filter_dup = 0x00; // duplicate type
unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
struct sigaction sa;
static volatile int signal_received = 0;
int usr_interrupt = 0;
int count_error = 0;
struct tm *tm_info, *config_time_stamp;
char time_chars[40];
struct timespec start_time, cur_time, ts_cycle_init_time, ts_cycle_time, ts_cycle_cur_time;
int num_simblee_control_module = 0;
int ble_scan_time = 0;
int cycle_time = 0; //seconds
int num_cycle = 1;
int ble_scan_status = 0;
int cur_ble_device_index;
time_t next_read_time;
time_t cur_read_time;
time_t read_start_time;
						



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

int discover_char(char *addr)
{
    int sock, status;
	struct sockaddr_l2 addr_des, addr_src;
	static int opt_start = 0x0001;
	static int opt_end = 0xffff;
	char read_buf[1024];
	double flow_rate, temp_reading[128];
	char temp_address[128];
	int num_temp_readings = 0;
	char current_data[20];
	int size_data;
	int current_data_size = 0;
	int current_index = 3;
	int isEnd = 1;
	int isPartial = 0;
	int size_copied = 0;
	uint16_t cid = ATT_CID;
	uint8_t  dst_type = BDADDR_LE_RANDOM;
	uint8_t  src_type = BDADDR_LE_PUBLIC;
	int master = 0;
	int flushable = 0;
	uint32_t priority = 0;
	int sec_level = BT_SECURITY_LOW;
	int i = 0;
	int try = RETRY_TIMES;
	int packet_count = 0;
	unsigned int expect_packet_size = 0;
	bdaddr_t sba;
	fd_set set;
    struct timeval timeout;
	int rc1,rc2;
	
	char desc_packet[5] = {ATT_OP_FIND_INFO_REQ, 0x01, 0x00, 0xff,0xff};
	
	
	bacpy(&sba, BDADDR_ANY);
	
	memset(&addr_src, 0, sizeof(addr_src));
	addr_src.l2_family = AF_BLUETOOTH;
	bacpy(&addr_src.l2_bdaddr, &sba);
	addr_src.l2_cid = htobs(cid);
	
	addr_src.l2_bdaddr_type = src_type;
 //   if(argc < 2)
 //   {
 //       fprintf(stderr, "usage: %s <bt_addr>\n", argv[0]);
 //       exit(EXIT_FAILURE);
 //   }

//    strncpy(dest, argv[1], 18);

    // allocate a socket
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


	
    // set the connection parameters (who to connect to)
	memset(&addr_des, 0, sizeof(addr_des));
    addr_des.l2_family = AF_BLUETOOTH;
    str2ba( addr, &addr_des.l2_bdaddr );
	addr_des.l2_cid = htobs(cid);
	addr_des.l2_bdaddr_type = dst_type;
	
    // connect to server
	try = RETRY_TIMES;
	while(try > 0 && connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des)) != 0)
	{
		try--;
	}
 //   status = connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des));
 
	if (try == 0)
	{
		printf("connect Retry out, cannot connect\n");
		
		goto FAIL;
	}
	else
	{
		printf("connect succeed!\n");
	}

	try = RETRY_TIMES;
	while(try > 0 && write(sock, desc_packet, sizeof(desc_packet)) < 0)
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
	
	packet_count = 0;
	count_error = 0;
	
	FD_ZERO(&set); /* clear the set */
	FD_SET(sock, &set); /* add our file descriptor to the set */
	
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	int read_count = 0;
	while(1)
	{	

		bzero(read_buf,sizeof(read_buf));
		
		//rv = select(sock + 1, &set, NULL, NULL, &timeout);
		// if(rv == -1)
		//	 printf("read error\n");
		//else if(rv == 0)
		//	 printf("timeout\n"); /* a timeout occured */
		//else
		
		status = read( sock, read_buf, sizeof(read_buf) ); /* there was data to read */
		
		printf("status=%d\n",status);
		
		if (status >= 0)
		{
			printf("data=");
			for (i = 0; i < status; i++)
			{
				printf("%02x ",read_buf[i]);
			}
			printf("\n");
			
		
			
			
		}
		else{
			printf("error read\n");
			goto FAIL;
		}
	}
	
	FAIL:
		close(sock);
		return -1;
	DONE:
		close(sock);
		return 0;
	
	
}




int main(){
	
	discover_char("DB:25:5B:27:C2:60");
	
	
	exit(EXIT_SUCCESS);
	
}

