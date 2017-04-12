#include <bridge.h>
#include <sqlfunc.h>
#include <dynamoDBfunc.h>

double db_cur_time = 0.0;

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
int usr_interrupt = 0;
int count_error = 0;
struct tm *tm_info, *config_time_stamp;
char time_chars[40];
struct timespec start_time, cur_time, ts_cycle_init_time, ts_cycle_time, ts_cycle_cur_time;
simblee_control_module *user_control_module;
int num_simblee_control_module = 0;
int ble_scan_time = 0;
int cycle_time = 0; //seconds
int num_cycle = 1;
int ble_scan_status = 0;
int cur_ble_device_index;
enum read_state_def read_state;
time_t next_read_time;
time_t cur_read_time;
time_t read_start_time;

static void sigint_handler(int sig);
int get_max_read_time();
int setnonblock(int sock);
void close_scan();
void init_scan();
int check_module_exist(char *addr, int *index);
int init_config_file();
int update_config_file();
void parse_ble_scan_data(unsigned char *buf);
void scan();
static int set_sec_level(int sock, int level);
int send_LE_data(char *addr, char *data, size_t data_len);
int send_LE_sleep_time(char *addr);
int configure_control_module();
int get_max_read_time();
double get_elapsed_time();
void display_time_stamp(struct timespec input_cur_time, char *text_desc);
void parse_simblee_data(double *flow_rate, double *temp_reading, char *temp_address, 
						int *num_temp_readings,	unsigned int *expect_packet_size, char *input);
int read_LE_data(char *addr);
void update_time_stamp();
void reset_time_stamp();
						
static void sigint_handler(int sig)
{
	signal_received = sig;
	printf("signal: %d\n",sig);
	switch (sig)
	{
		case SIGINT:
			usr_interrupt = 1;
			break;
	}
	
}

int setnonblock(int sock) {
   int flags;
   flags = fcntl(sock, F_GETFL, 0);
   if (-1 == flags)
      return -1;
   return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void close_scan()
{
		ble_scan_status = 0;
	
		scan_err = hci_le_set_scan_enable(scan_sock, 0x00, scan_filter_dup, 10000);
   		if (scan_err < 0) {
			perror("Disable scan failed");
			return;
		}	

		printf("close dev\n");
		
		hci_close_dev(scan_sock);
}

void init_scan()
{

		ble_scan_status = 1;
		
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
		
		setnonblock(scan_sock);
}

int check_module_exist(char *addr, int *index)
{
	int i;
	for( i = 0;i < num_simblee_control_module;i++)
	{
		if (strcmp(addr,user_control_module[i].addr) == 0)
		{
			*index =  i;
			return 1;
		}
	}
	
	return 0;
}

int init_config_file()
{
		time_t now;
		
		config_t cfg;
			
		config_setting_t *root, *list, *group, *device, *device_attr, *setting, *module;

		config_init(&cfg);
	
		if ( access (CONFIG_FILE_NAME, F_OK) != -1)
		{
			// exists
			printf("cfg file exists\n");
			
			if(! config_read_file(&cfg, CONFIG_FILE_NAME))
			{
				fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
						config_error_line(&cfg), config_error_text(&cfg));
				config_destroy(&cfg);
				return -1;
			}
			
		  printf("init\n");
		  root = config_root_setting(&cfg);
			
		  group = config_setting_get_member(root, "bridge_config");
		  if(!group)
		  {
			config_destroy(&cfg);
			return -1;
		  }
		  
		  
		  setting = config_setting_get_member(group, "cycle_time");
		  if(!setting)
		  {
			config_destroy(&cfg);
			return -1;
		  }

		  cycle_time = config_setting_get_int(setting);	
		
			
		}
		else{
			// not exists
			printf("cfg file not exists\n");
			
			root = config_root_setting(&cfg);
			
			group = config_setting_add(root,"bridge_config", CONFIG_TYPE_GROUP);
			
			list = config_setting_add(group,"control_module", CONFIG_TYPE_LIST);
			
			setting = config_setting_add(group, "cycle_time", CONFIG_TYPE_INT);
		    config_setting_set_int(setting, DEFAULT_CYCLE_TIME);
			
			cycle_time = DEFAULT_CYCLE_TIME;
			
			if(! config_write_file(&cfg, CONFIG_FILE_NAME))
     		{
				printf("Error while writing file.\n");
				config_destroy(&cfg);
				return -1;
			}
			  
			printf("New configuration successfully written to: %s\n", CONFIG_FILE_NAME);
			
			
			
		}
		
		
		
		config_destroy(&cfg);
			
		return 0;
		
	
}

int update_config_file()
{
		time_t now;
		
		config_t cfg;
			
		config_setting_t *root, *list, *group, *device, *device_attr, *setting, *module;

		config_init(&cfg);

		int i,j;
			// config file exists in current dir
		printf("***************updating config file**********************\n");
			
		 if(! config_read_file(&cfg, CONFIG_FILE_NAME))
		  {
			fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
					config_error_line(&cfg), config_error_text(&cfg));
			config_destroy(&cfg);
			return -1;
		  }
		  
		  printf("init\n");
		  root = config_root_setting(&cfg);
			
		  group = config_setting_get_member(root, "bridge_config");
		  if(!setting)
		  {
			config_destroy(&cfg);
			return -1;
		  }
		  
		  setting = config_setting_get_member(group, "cycle_time");
		  if(!setting)
		  {
			config_destroy(&cfg);
			return -1;
		  }
		  	

		if (cycle_time != config_setting_get_int(setting))
		{
		  cycle_time = config_setting_get_int(setting);	
		  printf("***********cycle_time changed, reset start time********\n");
		  reset_time_stamp();
		}
		
									// calculate estimate read time
		int max_read_time = get_max_read_time() + 3 * num_simblee_control_module;

		if (cycle_time < max_read_time)
		{
			printf("warning, cycle_time smaller than read time, adjust to %d\n", max_read_time);
					
			cycle_time = max_read_time;
			config_setting_set_int(setting, cycle_time);
			printf("***********cycle_time smaller than max read time, reset start time********\n");
			reset_time_stamp();
		}
		printf("***************checking cycle_time, max_read_time=%d****************\n",max_read_time);
		printf("***************cycle_time=%d************************\n",cycle_time);
	
		  setting = config_setting_get_member(group, "control_module");
		  if(!setting)
		  {
			config_destroy(&cfg);
			return -1;
		  }
		  
		  printf("get control module\n");
		  list = config_lookup(&cfg, "bridge_config.control_module");
		  if(list != NULL)
		  {
			 int count = config_setting_length(list);
			 
			 for(i = 0; i < num_simblee_control_module;i++)
			 {
				 int module_exist_in_config = 0;
				 for(j = 0; j < count;j++)
				 {
					 module = config_setting_get_elem(list, j);
					 const char *addr;
					 
					 printf("start check i=%d,j=%d\n",i,j);
					 
					 if (!(config_setting_lookup_string(module, "address", &addr))){
						continue;
					 }
					 else{
						 
						if (strcmp(addr, user_control_module[i].addr) == 0)
						{
							module_exist_in_config = 1;
							break;
						}
					}
					 
				 }
				 
				 if (module_exist_in_config){
					 
					 printf("updating\n");
					 
					device_attr = config_setting_get_member(module, "serial");
					config_setting_set_string(device_attr, user_control_module[i].serial_num);
							
					device_attr = config_setting_get_member(module, "rssi");
					config_setting_set_int(device_attr, user_control_module[i].rssi);
						
					device_attr = config_setting_get_member(module, "num_temp_sensors");
					config_setting_set_int(device_attr, user_control_module[i].num_temp_sensors);
							
					device_attr = config_setting_get_member(module, "vortex_read_time");
					config_setting_set_int(device_attr, user_control_module[i].vortex_read_time);
					
					device_attr = config_setting_get_member(module, "vortex_read_time_config");
					user_control_module[i].vortex_read_time_config = config_setting_get_int(device_attr);
					
					time(&now);
				
					config_time_stamp = localtime(&now);
				
					strftime(time_chars,sizeof(time_chars),"%Y_%m_%d_%H_%M_%S",config_time_stamp);
				
					device_attr = config_setting_get_member(module, "last_update_time_stamp");
					config_setting_set_string(device_attr, time_chars);
							
					printf("parameters updated for addr = %s\n",user_control_module[i].addr);
				 }
				 else{
					
					printf("not on the list, add to list\n");
					// add to list
					device = config_setting_add(list, NULL, CONFIG_TYPE_GROUP);
				
					setting = config_setting_add(device, "serial", CONFIG_TYPE_STRING);
					config_setting_set_string(setting, user_control_module[i].serial_num);
							
					setting = config_setting_add(device, "address",CONFIG_TYPE_STRING);
					config_setting_set_string(setting, user_control_module[i].addr);
							
					setting = config_setting_add(device, "rssi",CONFIG_TYPE_INT);
					config_setting_set_int(setting, user_control_module[i].rssi);
							
					setting = config_setting_add(device, "num_temp_sensors",CONFIG_TYPE_INT);
					config_setting_set_int(setting, user_control_module[i].num_temp_sensors);
							
					setting = config_setting_add(device, "vortex_read_time",CONFIG_TYPE_INT);
					config_setting_set_int(setting, user_control_module[i].vortex_read_time);
					
					setting = config_setting_add(device, "vortex_read_time_config", CONFIG_TYPE_INT);
					config_setting_set_int(setting, DEFAULT_VORTEX_READ_TIME);
					//printf("Module %d: Addr=%s, SN=%s\n",i,user_control_module[i].addr,user_control_module[i].serial_num);
				
					time(&now);
				
					config_time_stamp = localtime(&now);
				
					strftime(time_chars,sizeof(time_chars),"%Y_%m_%d_%H_%M_%S",config_time_stamp);
				
					setting = config_setting_add(device, "last_update_time_stamp", CONFIG_TYPE_STRING);
					config_setting_set_string(setting, time_chars);
				 
				 }
				 
				printf("checking %d\n",i);
			 }
		    
			  if(! config_write_file(&cfg, CONFIG_FILE_NAME))
			  {
				fprintf(stderr, "Error while writing file.\n");
				config_destroy(&cfg);
				return -1;
			  }
				

		  
			  return 0;
		  
		  }
		  else{
			  printf("warning:list error\n");
			  config_destroy(&cfg);
			  return -1;
		  }
		  

		  
}

void parse_ble_scan_data(unsigned char *buf)
{
	evt_le_meta_event *meta;
	le_advertising_info *info;
	custome_ad_data *ad_data;
	char addr[18];

	ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
	
	meta = (void *) ptr;
	
	if (meta->subevent == 0x02)
	{
		info = (le_advertising_info *) (meta->data + 1);
		
		ad_data = (custome_ad_data *) (info->data + 19);
		
		if (ad_data->com_id1 == SIM_AD_COMPANY_ID1 
		 && ad_data->com_id2 == SIM_AD_COMPANY_ID2)
		 {	
			ba2str(&info->bdaddr, addr);
			//printf("company ID verified\n");
			if (check_module_exist(addr, &cur_ble_device_index) == 1)
			{
				printf("already registered\n");
			}
			else{
				printf("add new module, address=%s\n",addr);
				if (num_simblee_control_module >= INITIAL_USER_MODULE_NUM)
				{
					user_control_module = realloc(user_control_module, sizeof(simblee_control_module) * (num_simblee_control_module + 1));
				}
				

				
				cur_ble_device_index = num_simblee_control_module;
				
				num_simblee_control_module++;	
			}
			
			
			memcpy(user_control_module[cur_ble_device_index].addr, addr, sizeof(addr));
			memcpy(user_control_module[cur_ble_device_index].serial_num, info->data + 2, SIM_AD_SERIAL_NUM_SIZE);
			user_control_module[cur_ble_device_index].rssi = (int)(ad_data->rssi - 256);
			user_control_module[cur_ble_device_index].num_temp_sensors = (int)ad_data->num_temp_sensors;
			user_control_module[cur_ble_device_index].vortex_read_time = (int)ad_data->vortex_read_time;
			
			printf("vortex_read_time=%02x\n",ad_data->vortex_read_time);
			printf("vortex_read_time=%d\n",user_control_module[cur_ble_device_index].vortex_read_time);
			
			user_control_module[cur_ble_device_index].status = (int)ad_data->status;
			
			update_config_file();
			
		     switch(read_state)
			 {
				case SCAN:
					//printf("ad_data status = %02x\n",ad_data->status);
					switch(ad_data->status)
					{	
						case SIM_AD_STATUS_REDAY:
							read_state = SET_SLEEP;
							break;
						case SIM_AD_STATUS_DATA_REDAY:
							read_state = READ_DATA;
							break;
					}
					break;
			 }
		 }
	}
}

void scan()
{
	int len,i;
	len = read(scan_sock, buf, sizeof(buf));
	
	if (len == SIM_AD_TOTAL_LENGTH)
	{
		parse_ble_scan_data(buf);
		printf("AD data start:\n");
		for (i = 0; i < len; i++)
		{
			printf("%02x ",buf[i]);
		}
		printf("\n");
		printf("AD data end\n");
	}
	
	usleep(100000);
}

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

int send_LE_data(char *addr, char *data, size_t data_len)
{
	int i;
	struct sockaddr_l2 addr_des, addr_src;
	int sock, status;
	bdaddr_t sba;
	uint16_t cid = ATT_CID;
	uint8_t  dst_type = BDADDR_LE_RANDOM;
	uint8_t  src_type = BDADDR_LE_PUBLIC;
	int try = RETRY_TIMES;
	int sec_level = BT_SECURITY_LOW;
//	char config_packet[5] = {ATT_OP_WRITE_CMD, SIM_WRITE_HND, 0x00, 0x02, 0x00};
	
	printf("send_LE_data addr = %s\n",addr);
	
	printf("size of data=%d\n",data_len);
	
	for (i = 0; i < data_len;i++)
	{
		printf("%02x ",data[i]);
	}
	
	printf("\n");
	
	bacpy(&sba, BDADDR_ANY);
			
	memset(&addr_src, 0, sizeof(addr_src));
	addr_src.l2_family = AF_BLUETOOTH;
	bacpy(&addr_src.l2_bdaddr, &sba);
	addr_src.l2_cid = htobs(cid);
	addr_src.l2_bdaddr_type = src_type;
	// Setup BLE source address
		
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
	while(try > 0 && write(sock, data, data_len) < 0)
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
	close(sock);
	return 0;
FAIL:
	close(sock);
	return -1;
}

int send_LE_sleep_time(char *addr)
{
	int i;
	struct sockaddr_l2 addr_des, addr_src;
	int sock, status;
	bdaddr_t sba;
	uint16_t cid = ATT_CID;
	uint8_t  dst_type = BDADDR_LE_RANDOM;
	uint8_t  src_type = BDADDR_LE_PUBLIC;
	int try = RETRY_TIMES;
	int sec_level = BT_SECURITY_LOW;
//	char config_packet[5] = {ATT_OP_WRITE_CMD, SIM_WRITE_HND, 0x00, 0x02, 0x00};
	
	bacpy(&sba, BDADDR_ANY);
			
	memset(&addr_src, 0, sizeof(addr_src));
	addr_src.l2_family = AF_BLUETOOTH;
	bacpy(&addr_src.l2_bdaddr, &sba);
	addr_src.l2_cid = htobs(cid);
	addr_src.l2_bdaddr_type = src_type;
	// Setup BLE source address
		
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

	clock_gettime(CLOCK_MONOTONIC, &ts_cycle_cur_time);
	long sleep_time =  (ts_cycle_time.tv_sec - ts_cycle_cur_time.tv_sec) * 1e3 + (ts_cycle_time.tv_nsec - ts_cycle_cur_time.tv_nsec) / 1e6;
	
	printf("sleep time = %d\n",sleep_time);
	
	
	
	if (sleep_time < 0)
	{
		printf("something wrong\n");
	}
	else{
		
		char send_packet[12];
		
		bzero(send_packet, sizeof(send_packet));
		send_packet[0] = ATT_OP_WRITE_CMD;
		send_packet[1] = SIM_WRITE_HND;
		send_packet[2] = 0x00;
		send_packet[3] = 0x01;
		
		memcpy(send_packet + 4, &user_control_module[cur_ble_device_index].vortex_read_time_config, sizeof(uint8_t));
		memcpy(send_packet + 5, &sleep_time, sizeof(long));
		
		try = RETRY_TIMES;
		while(try > 0 && write(sock, send_packet, sizeof(send_packet)) < 0)
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
			goto SUCCESS;
		}
	}
	

	
SUCCESS:
	close(sock);
	printf("close sock\n");
	return 0;
FAIL:
	close(sock);
	return -1;
}

int configure_control_module()
{
	char config_packet[5] = {ATT_OP_WRITE_CMD, SIM_WRITE_HND, 0x00, 0x02, 0x14};
	
	int i, j;
	
	for (i = 0; i < num_simblee_control_module;i++)
	{	

		if (user_control_module[i].vortex_read_time != user_control_module[i].vortex_read_time_config)
		{			
			
			memcpy(config_packet + 4, &user_control_module[i].vortex_read_time_config, sizeof(uint8_t));
			
			send_LE_data(user_control_module[i].addr, config_packet, sizeof(config_packet));
			

			
		}
	}
	
	return 0;
}

int get_max_read_time()
{
	int i;
	int max = 0;
	
	for (i = 0; i < num_simblee_control_module;i++)
	{	
		user_control_module[i].total_read_time = (double)user_control_module[i].vortex_read_time_config +
												 (double)user_control_module[i].num_temp_sensors * 0.2;
												 
		if ((int)user_control_module[i].total_read_time > max)
		{
			max = (int)user_control_module[i].total_read_time;
		}
	}
	
	return max;
}

double get_elapsed_time()
{
	double diff;
	
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
		
	diff = 1.0e9 * (cur_time.tv_sec - start_time.tv_sec) + cur_time.tv_nsec - start_time.tv_nsec;
		
	diff = diff / 1.0e9;
	
	return diff;
}

void display_time_stamp(struct timespec input_cur_time, char *text_desc)
{
	double diff;
		
	diff = input_cur_time.tv_sec + input_cur_time.tv_nsec / 1.0e9;

	printf("[%s Time Stamp=%f]\n",text_desc,diff);
}

void parse_simblee_data(double *flow_rate, double *temp_reading, char *temp_address, 
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

int read_LE_data(char *addr)
{
	struct sockaddr_l2 addr_des, addr_src;
    int sock, status;
	char notification_packet[5] = {ATT_OP_WRITE_REQ, SIM_NOTIF_HND, 0x00, 0x01, 0x00};
	char rcv_confirm_packet[5] = {ATT_OP_WRITE_CMD, SIM_WRITE_HND, 0x00, 0x01, 0x00};
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
	int rc1,rc2;
	pthread_t thread1,thread2;
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
	while(try > 0 && write(sock, notification_packet, sizeof(notification_packet)) < 0)
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
					parse_simblee_data(&flow_rate,temp_reading,temp_address,&num_temp_readings,&expect_packet_size,current_data);
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
							parse_simblee_data(&flow_rate,temp_reading,temp_address,&num_temp_readings,&expect_packet_size,current_data);
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
							parse_simblee_data(&flow_rate,temp_reading,temp_address,&num_temp_readings,&expect_packet_size,current_data);
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
					clock_gettime(CLOCK_MONOTONIC, &ts_cycle_cur_time);
					long sleep_time =  (ts_cycle_time.tv_sec - ts_cycle_cur_time.tv_sec) * 1e3 + (ts_cycle_time.tv_nsec - ts_cycle_cur_time.tv_nsec) / 1e6;
					
					printf("sleep time = %d\n",sleep_time);
					
					
					
					if (sleep_time < 0)
					{
						printf("something wrong\n");
						goto FAIL;
					}
					else{
						
						char send_packet[12];
						
						bzero(send_packet, sizeof(send_packet));
						send_packet[0] = ATT_OP_WRITE_CMD;
						send_packet[1] = SIM_WRITE_HND;
						send_packet[2] = 0x00;
						send_packet[3] = 0x01;
						
						memcpy(send_packet + 4, &user_control_module[cur_ble_device_index].vortex_read_time_config, sizeof(uint8_t));
						memcpy(send_packet + 5, &sleep_time, sizeof(long));
						
						try = RETRY_TIMES;
						while(try > 0 && write(sock, send_packet, sizeof(send_packet)) < 0)
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
					}
					
					char BLE_log_date[15];
					char BLE_log_time[10];
					bzero(BLE_log_date,sizeof(BLE_log_date));
					bzero(BLE_log_time,sizeof(BLE_log_time));
					update_time_stamp();
					strftime(time_chars,sizeof(time_chars),"%Y_%m_%d_%H_%M_%S",tm_info);
					printf("TEST read time = %s\n", time_chars);
					strftime(BLE_log_date,sizeof(BLE_log_date),"%m/%d/%Y",tm_info);
					strftime(BLE_log_time,sizeof(BLE_log_time),"%H:%M:%S",tm_info);
					
					hx_data *cur_hx_bulk_data_for_sql, *cur_hx_bulk_data_for_dynamoDB;
					
					cur_hx_bulk_data_for_sql = malloc(sizeof(hx_data) * (num_temp_readings + 1));
					cur_hx_bulk_data_for_dynamoDB = malloc(sizeof(hx_data) * (num_temp_readings + 1));
					//printf("flow_rate=%f\n",flow_rate);
					printf("%s,%s,Flow Rate,%f,-,%s,%s\n",BLE_log_date,BLE_log_time,flow_rate,user_control_module[cur_ble_device_index].serial_num,user_control_module[cur_ble_device_index].addr);
					
					cur_hx_bulk_data_for_sql[0].time_stamp = cur_read_time;
					cur_hx_bulk_data_for_sql[0].data = flow_rate;
					cur_hx_bulk_data_for_sql[0].data_address = 0x00;
					strcpy(cur_hx_bulk_data_for_sql[0].data_unit,"RAW");
					strcpy(cur_hx_bulk_data_for_sql[0].data_type,"FLOW_RATE");
					strcpy(cur_hx_bulk_data_for_sql[0].serial_num,user_control_module[cur_ble_device_index].serial_num);
					strcpy(cur_hx_bulk_data_for_sql[0].ble_address,user_control_module[cur_ble_device_index].addr);
					
					for(i = 0;i< num_temp_readings;i++)
					{
					//	printf("num_temp=%d,temp_reading=%f,address=%02x\n",i,temp_reading[i],temp_address[i]);
						printf("%s,%s,Temperature,%f,%02x,%s,%s\n",BLE_log_date,BLE_log_time,temp_reading[i],temp_address[i],user_control_module[cur_ble_device_index].serial_num,user_control_module[cur_ble_device_index].addr);
						
						cur_hx_bulk_data_for_sql[i + 1].time_stamp = cur_read_time;
						cur_hx_bulk_data_for_sql[i + 1].data = temp_reading[i];
						cur_hx_bulk_data_for_sql[i + 1].data_address = (uint8_t)temp_address[i];
						strcpy(cur_hx_bulk_data_for_sql[i + 1].data_unit,"F");
						strcpy(cur_hx_bulk_data_for_sql[i + 1].data_type,"TEMPERATURE");
						strcpy(cur_hx_bulk_data_for_sql[i + 1].serial_num,user_control_module[cur_ble_device_index].serial_num);
						strcpy(cur_hx_bulk_data_for_sql[i + 1].ble_address,user_control_module[cur_ble_device_index].addr);
						
					}
					
					memcpy(cur_hx_bulk_data_for_dynamoDB, cur_hx_bulk_data_for_sql, sizeof(hx_data) * (num_temp_readings + 1));
					
					thread_arg hx_data_arguments_sql, hx_data_arguments_dynamoDB;
					
					hx_data_arguments_sql.user_data = cur_hx_bulk_data_for_sql;
					hx_data_arguments_sql.size_hx_data = num_temp_readings + 1;
					
					hx_data_arguments_dynamoDB.user_data = cur_hx_bulk_data_for_dynamoDB;
					hx_data_arguments_dynamoDB.size_hx_data = num_temp_readings + 1;
					
					if( (rc1 = pthread_create( &thread1, NULL, &write_sqlite_hx_data_wrapper, (void *)&hx_data_arguments_sql)))
					{
						fprintf(stderr,"Thread creation failed: %d\n", rc1);
					}

					if (rc1 == 0)
					{
						rc1 = pthread_detach(thread1);
					}
					
					
					if( (rc2 = pthread_create( &thread2, NULL, &send_data_to_cloud_wrapper, (void *)&hx_data_arguments_dynamoDB)))
					{
						fprintf(stderr,"Thread creation failed: %d\n", rc1);
					}

					if (rc2 == 0)
					{
						rc1 = pthread_detach(thread2);
					}
					
					
				//	write_sqlite_hx_data(cur_hx_bulk_data, num_temp_readings + 1);
				//	pthread_join(thread1,NULL);
				//	free(cur_hx_bulk_data_for_sql);
				//	free(cur_hx_bulk_data_for_dynamoDB);
				//	fflush(fp);
					printf("successfully finish reading\n");
					goto DONE;
					
				}
			}
			

			packet_count++;
		}
		else if (status < 0)
		{
			printf("cannot read\n");
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

void update_time_stamp()
{
	next_read_time = read_start_time + num_cycle * cycle_time;
	cur_read_time = read_start_time + (num_cycle - 1) * cycle_time;
	ts_cycle_time.tv_sec = ts_cycle_init_time.tv_sec + num_cycle * cycle_time;
	display_time_stamp(ts_cycle_time, "ts_cycle_time");
	tm_info = localtime(&cur_read_time);
	strftime(time_chars,sizeof(time_chars),"%Y_%m_%d_%H_%M_%S",tm_info);
	printf("Next read time = %s\n", time_chars);
}

void reset_time_stamp()
{
	num_cycle = 1;
	printf("Cycle time = %d\n", cycle_time);
	time(&read_start_time);
	clock_gettime(CLOCK_MONOTONIC, &ts_cycle_init_time);
	display_time_stamp(ts_cycle_init_time, "ts_cycle_init_time");
	ts_cycle_time.tv_sec = ts_cycle_init_time.tv_sec + num_cycle * cycle_time;
	
}

int main(){
	
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);
	
	
	user_control_module = malloc(sizeof(simblee_control_module) * INITIAL_USER_MODULE_NUM);
	
	init_config_file();
	
	read_state = INIT;
	
	printf("Cycle time = %d\n", cycle_time);
	time(&read_start_time);
	clock_gettime(CLOCK_MONOTONIC, &ts_cycle_init_time);
	display_time_stamp(ts_cycle_init_time, "ts_cycle_init_time");
	ts_cycle_time = ts_cycle_init_time;
	
	while(!usr_interrupt)
	{

		switch(read_state)
		{
			case INIT:
				printf("INIT\n");
				update_time_stamp();
				read_state = INIT_SCAN;
				break;
			case INIT_SCAN:
				printf("init scan\n");
				init_scan();
				read_state = SCAN;
				break;
			case SCAN:
				clock_gettime(CLOCK_MONOTONIC, &ts_cycle_cur_time);
				if (ts_cycle_cur_time.tv_sec > ts_cycle_time.tv_sec)
				{
					num_cycle++;
					update_time_stamp();
					printf("update time stamp\n");
				}
				scan();
				break;
			case CLOSE_SCAN:
				close_scan();
				read_state = INIT;
				break;
			case READ_DATA:
				printf("READ_DATA\n");
				if (ble_scan_status == 1)
				{
					close_scan();
				}
				int status_le_read_data;
				status_le_read_data = read_LE_data(user_control_module[cur_ble_device_index].addr);
				read_state = INIT;				
				break;
			case SET_SLEEP:
				printf("SET_SLEEP\n");
				if (ble_scan_status == 1)
				{
					close_scan();
				}
				send_LE_sleep_time(user_control_module[cur_ble_device_index].addr);
				read_state = INIT;
				break;
		}

		
	}
			
//	printf("*****Config Finish, start reading cycle******[ %f s]\n", get_elapsed_time());
	if (ble_scan_status == 1)
	{
		close_scan();
	}
	
	free(user_control_module);
//	fclose(fp);
	exit(EXIT_SUCCESS);
	
}

