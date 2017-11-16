// global general utilies functions

#include <utilities.h>

uint8_t singlechar2hex(char din)
{
	if (din >= 0x30 && din <= 0x39)
	{
		din -= 0x30;
	}
	else if (din >= 41 && din <= 0x46)
	{
		din -= 0x37;
	}

	return din;
}

uint16_t char2hex16(uint8_t *data, uint8_t start_pos)
{

    uint16_t out = 0x00;

    out += (singlechar2hex(data[start_pos]) << 12);
    out += (singlechar2hex(data[start_pos + 1]) << 8);
    out += (singlechar2hex(data[start_pos + 2]) << 4);
    out += singlechar2hex(data[start_pos + 3]);
    return out;

}

uint8_t char2hex(uint8_t *data, uint8_t start_pos)
{
	uint8_t out = 0x00;

	out = (singlechar2hex(data[start_pos]) << 4) + singlechar2hex(data[start_pos + 1]);

	return out;

}
char nibbleToHexCharacter(uint8_t nibble) {
                                   /* Converts 4 bits into hexadecimal */
  if (nibble < 10) {
    return ('0' + nibble);
  }
  else {
    return ('A' + nibble - 10);
  }
}
uint8_t hex2char(uint8_t data_in, char *out)
{
	out[0] = nibbleToHexCharacter((data_in & 0b11110000) >> 4);
	out[1] = nibbleToHexCharacter(data_in & 0b00001111);

	return 0;
}

uint8_t cal_checksum(uint8_t *value, uint8_t len)
{
	uint8_t i;
	int checksum = 0;

	for(i = BLE_DATA_API_POS; i < len; i++)
	{
		checksum += value[i];
	}

	checksum &= 0xff;

	checksum = 0xff - checksum;

	return (uint8_t)checksum;
}

int resolve_ip(char *host_name, struct addrinfo *addr)
{
	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd = 0, s;

	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(host_name, "http", &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

   /* getaddrinfo() returns a list of address structures.
	  Try each address until we successfully connect(2).
	  If socket(2) (or connect(2)) fails, we (close the socket
	  and) try the next address. */

   for (rp = result; rp != NULL; rp = rp->ai_next) {
	   sfd = socket(rp->ai_family, rp->ai_socktype,
					rp->ai_protocol);
	   if (sfd == -1)
		   continue;

	   if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
		{
		   memcpy(addr, rp, sizeof(struct addrinfo));
		   break;                  /* Success */
	   }
   }
   if (rp == NULL) {               /* No address succeeded */
	   fprintf(stderr, "Could not connect\n");
	   close(sfd);
	   return -1;
   }
   freeaddrinfo(result);           /* No longer needed */
   close(sfd);
   return 0;
}

int check_internet()
{
    int sockfd;
    char buffer[MAX_LINE];
	struct addrinfo re_addr;

    sockfd = -1;

	if (resolve_ip(INTERNET_CHECK_HOST, &re_addr) == 0)
	{
        if((sockfd = socket(re_addr.ai_family,SOCK_STREAM,IPPROTO_TCP)) != -1)
        {
			if( connect(sockfd,re_addr.ai_addr,re_addr.ai_addrlen) == 0)
			{
				if(write(sockfd,"GET /index.html HTTP/1.1\r\n\r\n", 29) >= 28)
				{
					shutdown(sockfd, SHUT_WR);
					if(read(sockfd, buffer, MAX_LINE) != -1) // all right!
					{
						close(sockfd);
						return 0;
					}
				}
			}
        }
    }

    if(sockfd!=-1)
        close(sockfd);
    return -1; // no internet
}

int file_exists(const char *fname)
{
	if( access( fname, F_OK ) != -1 ) {
		return 1;
	} else {
		return 0;
	}
}

int get_eth0_mac_addr(char *mac_addr)
{
	int fd;
	struct ifreq ifr;
	char *iface = "eth0";
	unsigned char *mac;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	ioctl(fd, SIOCGIFHWADDR, &ifr);

	close(fd);

	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	//display mac address
	sprintf(mac_addr, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	return 0;
}
/*
int update_config_file(bridge *bridge_data){
	int i = 0, j = 0;
	char payload[MAX_HTTP_REQUEST_SIZE];

	json_object *jobj_root = json_object_new_object();
	json_object *jvalue1 = json_object_new_string(bridge_data->aws_access_key);
	json_object_object_add(jobj_root,"AWS_ACCESS_KEY",jvalue1);

	json_object *jvalue2 = json_object_new_string(bridge_data->aws_secret_access_key);
	json_object_object_add(jobj_root,"AWS_SECRET_ACCESS_KEY",jvalue2);

	json_object *jvalue3 = json_object_new_string(bridge_data->addr);
	json_object_object_add(jobj_root,"DeviceID",jvalue3);

	json_object *jarr1 = json_object_new_array();

	for (i = 0; i < bridge_data->size_cm; i++)
	{
			json_object *jcm = json_object_new_object();

			json_object *jcm_value1 = json_object_new_string(bridge_data->cm[i].addr);
			json_object_object_add(jcm,"DeviceID",jcm_value1);

			json_object *jcm_value2 = json_object_new_string(bridge_data->cm[i].ble_addr);
			json_object_object_add(jcm,"Bluetooth_Address",jcm_value2);

			json_object *jcm_value3 = json_object_new_string(bridge_data->cm[i].protocol);
			json_object_object_add(jcm,"Protocol",jcm_value3);

			json_object *jsen_arr = json_object_new_array();

			for (j = 0; j < bridge_data->cm[i].size_sen; j++)
			{
					json_object *jsen_addr = json_object_new_string(bridge_data->cm[i].sen[j].addr);
					json_object_array_add(jsen_arr, jsen_addr);
			}
			json_object_object_add(jcm,"Sensors",jsen_arr);

			json_object_array_add(jarr1, jcm);
	}

	json_object_object_add(jobj_root,"CoreModules",jarr1);

	json_object *jvalue4 = json_object_new_double(bridge_data->config_version);
	json_object_object_add(jobj_root,"ConfigVersion",jvalue4);

	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PRETTY));

	DEBUG_PRINT("output_data:%s\n",payload);

	json_object_put(jobj_root);

	struct stat st = {0};

	if (stat(CONFIG_FOLDER_PATH, &st) == -1) {
			mkdir(CONFIG_FOLDER_PATH, 0755);
	}

	FILE *file = fopen(CONFIG_FILE_PATH, "r+");
	if (file == NULL) {
		perror("could not open file");
	}

	if (ftruncate(fileno(file),0) != 0)
	{
		perror("truncate failed");
	}

	if (fputs(payload, file) == EOF)
	{
		perror("Cannot write to file");
	}

	fclose(file);
	return 0;
}

int update_sensors(int cm_index, bridge *bridge_data)
{
	int i;

	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;

	json_object *jobj_root = json_object_new_object();
	json_object *jtable_name = json_object_new_string("Hx.DeviceConfiguration");
	json_object_object_add(jobj_root,"TableName",jtable_name);

	json_object *jfilterexp = json_object_new_string("LinkDeviceID = :val");
	json_object_object_add(jobj_root,"FilterExpression",jfilterexp);

	json_object *jmacaddr0 = json_object_new_string(bridge_data->cm[cm_index].addr);
	json_object *jmacaddr1 = json_object_new_object();
	json_object *jmacaddr2 = json_object_new_object();
	json_object_object_add(jmacaddr1,"S",jmacaddr0);
	json_object_object_add(jmacaddr2,":val",jmacaddr1);
	json_object_object_add(jobj_root,"ExpressionAttributeValues",jmacaddr2);

	json_object *jcap = json_object_new_string("TOTAL");
	json_object_object_add(jobj_root,"ReturnConsumedCapacity",jcap);

	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));

	json_object_put(jobj_root);

	int ipayloadlength = strlen(payload);

	DEBUG_PRINT("%s\n",payload);

	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_SCAN, bridge_data->aws_access_key, bridge_data->aws_secret_access_key) != 0)
	{
		return -1;
	}

	if (send_http_request_to_dynamodb(http_request, http_request_len, http_response, &http_response_len) != 0)
	{
		return -1;
	}

	char *date_pos,*http_pos,*json_start;

	http_pos = strstr(http_response, "HTTP/1.1 200 OK");
	date_pos = strstr(http_response, "Date:");
	json_start = strstr(http_response, "{");
	if (http_pos == NULL)
	{
		return -1;
	}

	if (date_pos == NULL)
	{
		return -1;
	}

	if (json_start == NULL)
	{
		return -1;
	}

	json_object * jparse_obj = json_tokener_parse(json_start);

	json_object_object_foreach(jparse_obj, key, val) {
			if (strcmp(key, "Count") == 0)
			{
				json_object * jcount = val;
				int count = json_object_get_int(jcount);
				if (count == 0)
				{
					break;
				}
			}
			else if (strcmp(key, "Items") == 0)
			{
					printf("found sensors Array\n");
					json_object * jitems = val;
					json_object * jitem;
					int arraylen = json_object_array_length(jitems);


					bridge_data->cm[cm_index].sen = g_try_new0(sensor, arraylen);
					bridge_data->cm[cm_index].size_sen = arraylen;



					printf("numbers of sensors=%d\n", arraylen);
					for (i = 0; i < arraylen; i++)
					{
						jitem =  json_object_array_get_idx(jitems, i);

						json_object *jtmp0, *jtmp1;

						if (json_object_object_get_ex(jitem,"DeviceID", &jtmp0))
						{
							if (json_object_object_get_ex(jtmp0,"S", &jtmp1))
							{
									strcpy(bridge_data->cm[cm_index].sen[i].addr, json_object_get_string(jtmp1));
							}
						}

					}
	    }
	}
	DEBUG_PRINT("exit sensors parse\n");
	json_object_put(jparse_obj);

	return 0;
}


int update_core_module(const char *gateway_addr, bridge *bridge_data)
{
	// free all resources
	int i;
	if (bridge_data->size_cm != 0)
	{
		uint8_t i = 0;
		for(i = 0; i < bridge_data->size_cm; i++)
		{
			bridge_data->cm[i].size_sen = 0;
			g_free(bridge_data->cm[i].sen);
		}

		bridge_data->size_cm = 0;
		g_free(bridge_data->cm);
	}

	// first get core_module linked to gateway
	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;

	json_object *jobj_root = json_object_new_object();
	json_object *jtable_name = json_object_new_string("Hx.DeviceConfiguration");
	json_object_object_add(jobj_root,"TableName",jtable_name);

	json_object *jfilterexp = json_object_new_string("LinkDeviceID = :val");
	json_object_object_add(jobj_root,"FilterExpression",jfilterexp);

	json_object *jmacaddr0 = json_object_new_string(gateway_addr);
	json_object *jmacaddr1 = json_object_new_object();
	json_object *jmacaddr2 = json_object_new_object();
	json_object_object_add(jmacaddr1,"S",jmacaddr0);
	json_object_object_add(jmacaddr2,":val",jmacaddr1);
	json_object_object_add(jobj_root,"ExpressionAttributeValues",jmacaddr2);

	json_object *jcap = json_object_new_string("TOTAL");
	json_object_object_add(jobj_root,"ReturnConsumedCapacity",jcap);

	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));

	json_object_put(jobj_root);

	int ipayloadlength = strlen(payload);

	DEBUG_PRINT("%s\n",payload);

	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_SCAN, bridge_data->aws_access_key, bridge_data->aws_secret_access_key) != 0)
	{
		return -1;
	}

	if (send_http_request_to_dynamodb(http_request, http_request_len, http_response, &http_response_len) != 0)
	{
		return -1;
	}

	char *date_pos,*http_pos,*json_start;

	http_pos = strstr(http_response, "HTTP/1.1 200 OK");
	date_pos = strstr(http_response, "Date:");
	json_start = strstr(http_response, "{");
	if (http_pos == NULL)
	{
		return -1;
	}

	if (date_pos == NULL)
	{
		return -1;
	}

	if (json_start == NULL)
	{
		return -1;
	}

	json_object * jparse_obj = json_tokener_parse(json_start);

	json_object_object_foreach(jparse_obj, key, val) {
			if (strcmp(key, "Count") == 0)
			{
				json_object * jcount = val;
				int count = json_object_get_int(jcount);
				if (count == 0)
				{
					break;
				}
			}
			else if (strcmp(key, "Items") == 0)
			{
					printf("found core module Array\n");
					json_object * jitems = val;
					json_object * jitem;
					int arraylen = json_object_array_length(jitems);


					bridge_data->cm = g_try_new0(core_module, arraylen);
					bridge_data->size_cm = arraylen;



					printf("numbers of core_module=%d\n", arraylen);
					for (i = 0; i < arraylen; i++)
					{
						jitem =  json_object_array_get_idx(jitems, i);

						json_object *jtmp0, *jtmp1;

						if (json_object_object_get_ex(jitem,"Bluetooth_Address", &jtmp0))
						{
							if (json_object_object_get_ex(jtmp0,"S", &jtmp1))
							{
								strcpy(bridge_data->cm[i].ble_addr, json_object_get_string(jtmp1));
							}
						}

						if (json_object_object_get_ex(jitem,"DeviceID", &jtmp0))
						{
							if (json_object_object_get_ex(jtmp0,"S", &jtmp1))
							{
									strcpy(bridge_data->cm[i].addr, json_object_get_string(jtmp1));
									if (update_sensors(i, bridge_data) != 0){
										fprintf(stderr, "sensors update failed\n");
									}


							}
						}

						if (json_object_object_get_ex(jitem,"Protocol", &jtmp0))
						{
							if (json_object_object_get_ex(jtmp0,"S", &jtmp1)){
									strcpy(bridge_data->cm[i].protocol, json_object_get_string(jtmp1));
							}
						}


					}
	    }
	}

	DEBUG_PRINT("exit cm parse\n");
	json_object_put(jparse_obj);

	if (update_config_file(bridge_data) != 0)
	{
		fprintf(stderr, "config file update failed\n");
	}

	return 0;

}

int load_remote_config(bridge *bridge_data)
{
	char mac_addr[17];

	if (get_eth0_mac_addr(mac_addr) != 0)
	{
		fprintf(stderr, "error: cannot get mac address\n");
		return -1;
	}
	// start search mac address on dynamodb

	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;

	json_object *jobj_root = json_object_new_object();
	json_object *jtable_name = json_object_new_string("Hx.DeviceConfiguration");
	json_object_object_add(jobj_root,"TableName",jtable_name);

	json_object *jfilterexp = json_object_new_string("Ethernet_MAC_Address = :val");
	json_object_object_add(jobj_root,"FilterExpression",jfilterexp);

	json_object *jmacaddr0 = json_object_new_string(mac_addr);
	json_object *jmacaddr1 = json_object_new_object();
	json_object *jmacaddr2 = json_object_new_object();
	json_object_object_add(jmacaddr1,"S",jmacaddr0);
	json_object_object_add(jmacaddr2,":val",jmacaddr1);
	json_object_object_add(jobj_root,"ExpressionAttributeValues",jmacaddr2);

	json_object *jcap = json_object_new_string("TOTAL");
	json_object_object_add(jobj_root,"ReturnConsumedCapacity",jcap);

	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));

	json_object_put(jobj_root);

	int ipayloadlength = strlen(payload);

	DEBUG_PRINT("%s\n",payload);

	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_SCAN, bridge_data->aws_access_key, bridge_data->aws_secret_access_key) != 0)
	{
		return -1;
	}

	if (send_http_request_to_dynamodb(http_request, http_request_len, http_response, &http_response_len) != 0)
	{
		return -1;
	}

	char *date_pos,*http_pos,*json_start;

	http_pos = strstr(http_response, "HTTP/1.1 200 OK");
	date_pos = strstr(http_response, "Date:");
	json_start = strstr(http_response, "{");
	if (http_pos == NULL)
	{
		return -1;
	}

	if (date_pos == NULL)
	{
		return -1;
	}

	if (json_start == NULL)
	{
		return -1;
	}

	json_object * jparse_obj = json_tokener_parse(json_start);

	json_object_object_foreach(jparse_obj, key, val) {
			if (strcmp(key, "Count") == 0)
			{
				json_object * jcount = val;
				int count = json_object_get_int(jcount);
				if (count != 1)
				{
					DEBUG_PRINT("count wrong\n");
					break;
				}
			}
			else if (strcmp(key, "Items") == 0)
			{
				DEBUG_PRINT("found module \n");
				json_object *jitem = json_object_array_get_idx(val, 0);
				json_object *jtmp0, *jtmp1;

				if (json_object_object_get_ex(jitem,"ConfigVersion", &jtmp0))
				{
					if (json_object_object_get_ex(jtmp0,"N", &jtmp1))
					{
							const char *version = json_object_get_string(jtmp1);
							double ver = atof(version);

							if (ver <= bridge_data->config_version)
							{
								DEBUG_PRINT("no need to update\n");
								break;
							}
							else
							{
								bridge_data->config_version = ver;
								DEBUG_PRINT("config update needed\n");
							}
					}
				}

				if (json_object_object_get_ex(jitem,"DeviceID", &jtmp0))
				{
					if (json_object_object_get_ex(jtmp0,"S", &jtmp1))
					{
							DEBUG_PRINT("get DeviceID\n");
							const char *gateway_addr = json_object_get_string(jtmp1);
							memset(bridge_data->addr,0,sizeof(bridge_data->addr));
							strcpy(bridge_data->addr,gateway_addr);
							if (update_core_module(gateway_addr, bridge_data) != 0)
							{
								fprintf(stderr, "update failed\n");
							}
					}
				}

	    }
	}
	DEBUG_PRINT("exit gateway parse\n");
	json_object_put(jparse_obj);

	return 0;
}
*/
int load_local_config(bridge *bridge_data)
{
	int i,j;
	FILE *fp;
	long lSize;
	char *buffer;

	fp = fopen (CONFIG_FILE_PATH , "rb" );
	if( !fp ) return 1;

	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );

	/* allocate memory for entire content */
	buffer = calloc( 1, lSize+1 );
	if(!buffer )
	{
		fclose(fp);
		return 1;
	}

	/* copy the file into the buffer */
	if( 1!=fread( buffer , lSize, 1 , fp) )
	{
	  fclose(fp);
	  free(buffer);
	  return 1;
	}
	/* do your work here, buffer is a string contains the whole text */

	json_object * jobj = json_tokener_parse(buffer);

	json_object_object_foreach(jobj, key, val) {
		if (strcmp(key, "DeviceID") == 0)
		{
			memset(bridge_data->addr,0,sizeof(bridge_data->addr));
			g_stpcpy(bridge_data->addr, json_object_get_string(val));
		}
		else if (strcmp(key, "CoreModules") == 0)
		{
			json_object * jitems = val;
			json_object * jitem = NULL;
			bridge_data->size_cm = json_object_array_length(jitems);
			bridge_data->index_cm = 0;
			bridge_data->cm = g_try_new0(core_module, bridge_data->size_cm);

			for(i = 0; i < bridge_data->size_cm; i++)
			{
					jitem =  json_object_array_get_idx(jitems, i);
					json_object *jble_addr_value, *jaddr_value, *jsensors;
					if (json_object_object_get_ex(jitem,"Bluetooth_Address", &jble_addr_value))
					{
						memset(bridge_data->cm[i].ble_addr,0,sizeof(bridge_data->cm[i].ble_addr));
						g_stpcpy(bridge_data->cm[i].ble_addr, json_object_get_string(jble_addr_value));
					}

					if (json_object_object_get_ex(jitem,"DeviceID", &jaddr_value))
					{
						memset(bridge_data->cm[i].addr,0,sizeof(bridge_data->cm[i].addr));
						g_stpcpy(bridge_data->cm[i].addr, json_object_get_string(jaddr_value));
					}

					if (json_object_object_get_ex(jitem,"Protocol", &jaddr_value))
					{
						strcpy(bridge_data->cm[i].protocol, json_object_get_string(jaddr_value));
					}

					if (json_object_object_get_ex(jitem,"Sensors", &jsensors))
					{
						json_object * jsensor = NULL;
						bridge_data->cm[i].size_sen = json_object_array_length(jsensors);
						bridge_data->cm[i].index_sen = 0;
						bridge_data->cm[i].discovered = 0;
						bridge_data->cm[i].sen = g_try_new0(sensor, bridge_data->cm[i].size_sen);

						for(j = 0; j < bridge_data->cm[i].size_sen; j++)
						{
							jsensor = json_object_array_get_idx(jsensors,j);
							memset(bridge_data->cm[i].sen[j].addr, 0, sizeof(bridge_data->cm[i].sen[j].addr));
							g_stpcpy(bridge_data->cm[i].sen[j].addr, json_object_get_string(jsensor));
						}
						json_object_put(jsensor);
					}

					json_object_put(jble_addr_value);
					json_object_put(jaddr_value);
					json_object_put(jsensors);
			}

			json_object_put(jitems);
			json_object_put(jitem);
		}
		else if (strcmp(key, "AWS_ACCESS_KEY") == 0)
		{
			memset(bridge_data->aws_access_key,0,sizeof(bridge_data->aws_access_key));
			g_stpcpy(bridge_data->aws_access_key, json_object_get_string(val));
		}
		else if (strcmp(key, "AWS_SECRET_ACCESS_KEY") == 0)
		{
			memset(bridge_data->aws_secret_access_key,0,sizeof(bridge_data->aws_secret_access_key));
			g_stpcpy(bridge_data->aws_secret_access_key, json_object_get_string(val));
		}
		else if (strcmp(key, "ConfigVersion") == 0)
		{
			bridge_data->config_version = json_object_get_double(val);
		}

	}


json_object_put(jobj);



DEBUG_PRINT("number of core_modules=%d\n", bridge_data->size_cm);

for(i = 0; i < bridge_data->size_cm; i++)
{

	DEBUG_PRINT("Core Moudle #%d \n", i);
	DEBUG_PRINT("    DeviceID=%s\n",bridge_data->cm[i].addr);
	DEBUG_PRINT("    BLE_ADDR=%s\n",bridge_data->cm[i].ble_addr);

	for(j = 0; j < bridge_data->cm[i].size_sen; j++)
	{
		DEBUG_PRINT("         Sen_addr=%s\n",bridge_data->cm[i].sen[j].addr);
	}

}

	fclose(fp);
	free(buffer);

	return 0;
}

int init_data(bridge *bridge_data)
{



//	char *config_path = "/etc/hxmonitor/config.json";

	bridge_data->data_queue = g_async_queue_new();


	if (file_exists(CONFIG_FILE_PATH) == 0)
	{
		printf("Error, cannot locate config file\n");
		return 1;
	}
	else
	{
		if (load_local_config(bridge_data) != 0){
			printf("cannot load local file\n");
			return 1;
		}
/*		else
		{
				if (check_internet() == 0)  // always check the latest configuration file first
				{
					if (load_remote_config(bridge_data) != 0){
						printf("cannot load remote config file\n");
					}
				}
		}*/
	}

	return 0;

}


int free_defs(bridge *data)
{
	g_async_queue_unref(data->data_queue);

	if (data->size_cm != 0)
	{
		uint8_t i = 0;
		for(i = 0; i < data->size_cm; i++)
		{
			g_free(data->cm[i].sen);
		}

		g_free(data->cm);
	}

	return 0;

}
