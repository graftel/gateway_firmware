
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

#define ATT_SIGNATURE_LEN		12

/* Attribute Protocol Opcodes */
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

/* Error codes for Error response PDU */
#define ATT_ECODE_INVALID_HANDLE		0x01
#define ATT_ECODE_READ_NOT_PERM			0x02
#define ATT_ECODE_WRITE_NOT_PERM		0x03
#define ATT_ECODE_INVALID_PDU			0x04
#define ATT_ECODE_AUTHENTICATION		0x05
#define ATT_ECODE_REQ_NOT_SUPP			0x06
#define ATT_ECODE_INVALID_OFFSET		0x07
#define ATT_ECODE_AUTHORIZATION			0x08
#define ATT_ECODE_PREP_QUEUE_FULL		0x09
#define ATT_ECODE_ATTR_NOT_FOUND		0x0A
#define ATT_ECODE_ATTR_NOT_LONG			0x0B
#define ATT_ECODE_INSUFF_ENCR_KEY_SIZE		0x0C
#define ATT_ECODE_INVAL_ATTR_VALUE_LEN		0x0D
#define ATT_ECODE_UNLIKELY			0x0E
#define ATT_ECODE_INSUFF_ENC			0x0F
#define ATT_ECODE_UNSUPP_GRP_TYPE		0x10
#define ATT_ECODE_INSUFF_RESOURCES		0x11
/* Application error */
#define ATT_ECODE_IO				0x80
#define ATT_ECODE_TIMEOUT			0x81
#define ATT_ECODE_ABORTED			0x82

#define ATT_MAX_VALUE_LEN			512
#define ATT_DEFAULT_L2CAP_MTU			48
#define ATT_DEFAULT_LE_MTU			23

#define ATT_CID					4
#define ATT_PSM					31

/* Flags for Execute Write Request Operation */
#define ATT_CANCEL_ALL_PREP_WRITES		0x00
#define ATT_WRITE_ALL_PREP_WRITES		0x01

/* Find Information Response Formats */
#define ATT_FIND_INFO_RESP_FMT_16BIT		0x01
#define ATT_FIND_INFO_RESP_FMT_128BIT		0x02

#define SIM_STATE_SCAN  0x01
#define SIM_STATE_READ  0x02
#define SIM_STATE_STOP  0x03
#define SIM_DATA_SUMMARY 	0x20
#define SIM_DATA_FLOWRATE   0x21
#define SIM_DATA_TEMP 		0x22

#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */
#define EIR_TX_POWER                0x0A  /* transmit power level */
#define EIR_DEVICE_ID               0x10  /* device ID */

const int RETRY_TIMES = 2;
const int MAX_PACKET_SIZE = 23;
uint8_t Ble_State = SIM_STATE_SCAN;
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
int count_error = 0;
struct tm tm_info;

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
/*
void Read_Date(struct tm *result)
{
	time_t cur_time;
	int status;
	status = MCP79410_Read_Date(result);
	
	if (status < 0){
		cur_time = time(NULL);
		*result = *localtime(&cur_time);
	}		
}*/

static void eir_parse_name(uint8_t *eir, size_t eir_len,
						char *buf, size_t buf_len)
{
	size_t offset;

	offset = 0;
	while (offset < eir_len) {
		uint8_t field_len = eir[0];
		size_t name_len;
//		printf("EIR Length: %d\n",field_len);
		/* Check for the end of EIR */
		if (field_len == 0)
			break;

		if (offset + field_len > eir_len)
			goto failed;

		switch (eir[1]) {
		case EIR_NAME_SHORT:
		case EIR_NAME_COMPLETE:
			name_len = field_len - 1;
			if (name_len > buf_len)
				goto failed;

			memcpy(buf, &eir[2], name_len);
			return;
		}

		offset += field_len + 1;
		eir += field_len + 1;
	}

failed:
	snprintf(buf, buf_len, "(unknown)");
}




void Setup_Scan()
{
		scan_dev_id = hci_get_route(NULL);
		scan_sock = hci_open_dev( scan_dev_id );
    
		if (scan_dev_id < 0 || scan_sock < 0) {
			perror("opening socket");
			exit(1);
		}
		
		scan_err = hci_le_set_scan_parameters(scan_sock, scan_type, scan_interval, scan_window,
						scan_own_type, scan_filter_policy, 10000);

		if (scan_err < 0) {
			perror("Set scan parameters failed");
			exit(1);
		}
	
		scan_err = hci_le_set_scan_enable(scan_sock, 0x01, scan_filter_dup, 10000);
		if (scan_err < 0) {
			perror("Enable scan failed");
			exit(1);
		}
		
		struct hci_filter nf, of;
		socklen_t golen;
		golen = sizeof(of);
		if (getsockopt(scan_sock, SOL_HCI, HCI_FILTER, &of, &golen) < 0) {
			printf("Could not get socket options\n");
			return;
		}

		hci_filter_clear(&nf);
		hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
		hci_filter_set_event(EVT_LE_META_EVENT, &nf);

		if (setsockopt(scan_sock, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
			printf("Could not set socket options\n");
			return;
		}
}

void Close_Scan()
{
		scan_err = hci_le_set_scan_enable(scan_sock, 0x00, scan_filter_dup, 10000);
   		if (scan_err < 0) {
			perror("Disable scan failed");
			return;
		}	

		printf("close dev\n");
		
		hci_close_dev(scan_sock);
}

static void sigint_handler(int sig)
{
	signal_received = sig;
	printf("signal: %d\n",sig);
	switch (sig)
	{
		case SIGINT:
			Ble_State = SIM_STATE_STOP;
			break;
	}
	
}

void Scan_LE_Advertise()
{
	int len, i;
	
	evt_le_meta_event *meta;
	le_advertising_info *info;
	char addr[18];
	len = read(scan_sock, buf, sizeof(buf));
	
	printf("AD data start:\n");
	for (i = 0; i < len; i++)
	{
		printf("%02x ",buf[i]);
	}
	printf("\n");
	printf("AD data end\n");
	
	ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
	len -= (1 + HCI_EVENT_HDR_SIZE);


	
	meta = (void *) ptr;
	if (meta->subevent != 0x02)
	return;
	/* Ignoring multiple reports */
	info = (le_advertising_info *) (meta->data + 1);
	
	char name[30];

	memset(name, 0, sizeof(name));

	ba2str(&info->bdaddr, addr);
	eir_parse_name(info->data, info->length,
						name, sizeof(name) - 1);

//	printf("%s %s\n", addr, name);

//	printf("*\n");

	char des_addr[] = "E7:8C:3E:82:FB:70";
	char des_addr1[] = "CE:F2:C6:AE:A2:C4";
	
	
/*	
	if (strcmp(addr,des_addr) == 0)
	{
		Close_Scan();
		printf("Found simblee 0\n");
		memcpy(desc_addr, des_addr, sizeof(des_addr));
		Ble_State = SIM_STATE_READ;

	}
	else if (strcmp(addr,des_addr1) == 0)
	{
		Close_Scan();
		printf("Found simblee 1\n");
		memcpy(desc_addr, des_addr1, sizeof(des_addr1));
		Ble_State = SIM_STATE_READ;
	}
	*/
}


void Parse_Simblee_Data(double *flow_rate, double *temp_reading, char *temp_address, 
						int *num_temp_readings,	unsigned int *expect_packet_size, char *input)
{
	int i;
	printf("parse_init\n");
	if (sizeof(input) < 3)
	{
		printf("error size=%d\n",sizeof(input));
		return;
	}
	
	switch(input[1])
	{
		case SIM_DATA_SUMMARY:
			printf("parse expected\n");
			memcpy(expect_packet_size, input + 2, sizeof(unsigned int));
			break;
		case SIM_DATA_FLOWRATE:
			printf("parse_flowrate=");
			for (i = 0; i < (int)input[0] + 1; i++)
			{
				printf("%02x ",input[i]);
			}
			printf("\n");
			memcpy(flow_rate, input + 2, sizeof(double));
			printf("parse_flowrate_F=%f\n",*flow_rate);
			break;
		case SIM_DATA_TEMP:
			printf("parse_temp=");
			for (i = 0; i < (int)input[0] + 1; i++)
			{
				printf("%02x ",input[i]);
			}
			printf("\n");
			memcpy(&temp_address[*num_temp_readings], input + 2, sizeof(char));
			memcpy(&temp_reading[*num_temp_readings], input + 3, sizeof(double));
			printf("Address = %02x ", temp_address[*num_temp_readings]);
			printf("parse_temp_T=%f\n", temp_reading[*num_temp_readings]);
			(*num_temp_readings)++;
			break;
	}
	
}


void Read_LE_Data()
{
	struct sockaddr_l2 addr_des, addr_src;
    int sock, status;
	char notification_packet[5] = {ATT_OP_WRITE_REQ, 0x0f, 0x00, 0x01, 0x00};
	char rcv_confirm_packet[5] = {ATT_OP_WRITE_CMD, 0x11, 0x00, 0x01, 0x00};
	char read_buf[512];
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
//	int rv;


	
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
		return;
	}
	
	if (bind(sock, (struct sockaddr *) &addr_src, sizeof(addr_src)) < 0) {
		printf("ERROR: BIND failted\n");
		return;
	}
	
	if (set_sec_level(sock, sec_level) < 0)
	{
		printf("ERROR: sec_level failted\n");
		return;
	}


	
    // set the connection parameters (who to connect to)
	memset(&addr_des, 0, sizeof(addr_des));
    addr_des.l2_family = AF_BLUETOOTH;
    str2ba( desc_addr, &addr_des.l2_bdaddr );
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
		
		goto DONE;
	}
	else
	{
		printf("connect succeed!\n");
	}

	try = RETRY_TIMES;
	while(try > 0 && write(sock, notification_packet, sizeof(notification_packet)) < 0)
	{
		try--;
	}
 //   status = connect(sock, (struct sockaddr *)&addr_des, sizeof(addr_des));
 
	if (try == 0)
	{
		printf("Write try out, cannot write\n");
	
		goto DONE;
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
		
		if (status >= 0 && read_buf[0] == ATT_OP_HANDLE_NOTIFY)
		{
			printf("current packet=%d\n",packet_count);
			
			printf("size=%d\n",status);
			
			i = 0;
			while(i < status)
			{
				printf("%02x ", read_buf[i]);
				i++;
			}
			printf("\n");
			
			isEnd = 1;
			
			while (isEnd )
			{
				if (isPartial)
				{
					memcpy(current_data + size_copied, read_buf + 3, current_data_size - size_copied);
					printf("current_data1: ");
					for (i = 0; i < current_data_size; i++)
					{
						printf("%02x ", current_data[i]);
					}
					isPartial = 0;
					printf("\n");
					current_index = 3 + current_data_size - size_copied;
					Parse_Simblee_Data(&flow_rate,temp_reading,temp_address,&num_temp_readings,&expect_packet_size,current_data);
				}
				else
				{
					current_data_size = (int)read_buf[current_index] + 1;
					
					if (current_data_size <= 1)
					{
						isEnd = 0;
					}
					else
					{
					
						if (current_index + current_data_size < MAX_PACKET_SIZE){
							bzero(current_data,sizeof(current_data));
							memcpy(current_data, read_buf + current_index, current_data_size);
							printf("current_data2: ");
							for (i = 0; i < current_data_size; i++)
							{
								printf("%02x ", current_data[i]);
							}
							isPartial = 0;
							printf("\n");
							current_index = current_index + current_data_size;
							Parse_Simblee_Data(&flow_rate,temp_reading,temp_address,&num_temp_readings,&expect_packet_size,current_data);
						}
						else if (current_index + current_data_size == MAX_PACKET_SIZE){
							bzero(current_data,sizeof(current_data));
							memcpy(current_data, read_buf + current_index, current_data_size);
							printf("current_data3: ");
							for (i = 0; i < current_data_size; i++)
							{
								printf("%02x ", current_data[i]);
							}
							printf("\n");
							isPartial = 0;
							current_index = 3;
							isEnd = 0;
							printf("end\n");
							Parse_Simblee_Data(&flow_rate,temp_reading,temp_address,&num_temp_readings,&expect_packet_size,current_data);
							// reset
						}
						else{	 // partial
							bzero(current_data,sizeof(current_data));
							memcpy(current_data, read_buf + current_index, MAX_PACKET_SIZE - current_index);
							size_copied = MAX_PACKET_SIZE - current_index;
							isPartial = 1;
							isEnd = 0;
							printf("partial end\n");
						}
					}
				}
			//	printf("current_data_size=%d\n", current_data_size);
			//	printf("current index=%d\n", current_index);
			//	printf("is partial=%d\n",isPartial);
			//	printf("size_copied=%d\n", size_copied);
				printf("expected packet size=%d\n",expect_packet_size);
			}
			
			
			
			if (packet_count == 0 && read_buf[3] == 0x07 && read_buf[4] == SIM_DATA_SUMMARY)
			{
				expect_packet_size = 0;
				memcpy(&expect_packet_size, read_buf + 5, 2);
				printf("expected packet size=%d\n",expect_packet_size);
				current_index = 4 + (int)read_buf[3];
				size_data = (int)read_buf[current_index];
			}
			else 
			{
				
				
				if (packet_count == expect_packet_size - 1)
				{
					//Send packet confirm
					try = RETRY_TIMES;
					while(try > 0 && write(sock, rcv_confirm_packet, sizeof(rcv_confirm_packet)) < 0)
					{
						try--;
					}
					
					if (try == 0)
					{
						printf("cannot write confirm data\n");
						
						goto DONE;
					}
					else{
						//Read_Date(&tm_info);
						//char BLE_log_date[15];
						//char BLE_log_time[10];
						//bzero(BLE_log_date,sizeof(BLE_log_date));
						//bzero(BLE_log_time,sizeof(BLE_log_time));
						//strftime(BLE_log_date,sizeof(BLE_log_date),"%m/%d/%Y",&tm_info);
						//strftime(BLE_log_time,sizeof(BLE_log_time),"%H:%M:%S",&tm_info);
						//printf("flow_rate=%f\n",flow_rate);
						//(fp,"%s,%s,Flow Rate,%f,-\n",BLE_log_date,BLE_log_time,flow_rate);
						//for(i = 0;i< num_temp_readings;i++)
						//{
						//	printf("num_temp=%d,temp_reading=%f,address=%02x\n",i,temp_reading[i],temp_address[i]);
						//	fprintf(fp,"%s,%s,Temperature,%f,%02x\n",BLE_log_date,BLE_log_time,temp_reading[i],temp_address[i]);
						//}
						//fflush(fp);
						printf("successfully finish reading\n");
						goto DONE;
					}
				}
			}
			

			packet_count++;
		}
		else if (status < 0)
		{
			printf("cannot read\n");
			goto DONE;	
		}
	}
	
	DONE:
		close(sock);
		Ble_State = SIM_STATE_SCAN;
		Setup_Scan();
		return;
		
}



int main(int argc, char **argv)
{
	
	Setup_Scan();
	
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);
	
	while(Ble_State == SIM_STATE_SCAN ||
		  Ble_State == SIM_STATE_READ)
	{	
		if (Ble_State == SIM_STATE_SCAN)
		{
			Scan_LE_Advertise();
		}
		else if (Ble_State == SIM_STATE_READ)
		{
			Read_LE_Data();
		}

	}

	Close_Scan();

    
}