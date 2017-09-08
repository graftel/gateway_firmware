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
	uint8_t i = 0;
	int checksum = 0;

	for(i = 0; i < len; i++)
	{
		checksum += value[i];
	}

	checksum &= 0xff;

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

	printf("host_name=%s\n",host_name);
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
			printf("debug 1\n");
		   memcpy(addr, rp, sizeof(struct addrinfo));
		   printf("debug 2\n");
		   break;                  /* Success */
	   }
   }
	printf("debug 3\n");
   if (rp == NULL) {               /* No address succeeded */
	   fprintf(stderr, "Could not connect\n");
	   close(sfd);
	   return -1;
   }
	printf("debug 4\n");
   freeaddrinfo(result);           /* No longer needed */
   printf("debug 5\n");
   close(sfd);
   printf("debug 6\n");
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


int load_config(bridge *bridge_data)
{
	int i,j;
	char *config_path = "/etc/hxgateway/config.json";

	if (file_exists(config_path) == 0)
	{
		printf("Error, cannot locate config file\n");
		return 1;
	}

	FILE *fp;
	long lSize;
	char *buffer;

	fp = fopen (config_path , "rb" );
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
					if (json_object_object_get_ex(jitem,"BLE_Address", &jble_addr_value))
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
						if (strcmp(json_object_get_string(jaddr_value), "Bluetooth") == 0)
						{
							bridge_data->cm[i].protocol = CM_BLE;
						}
						else if (strcmp(json_object_get_string(jaddr_value), "RS485") == 0)
						{
							bridge_data->cm[i].protocol = CM_RS485;
						}
					}

					if (json_object_object_get_ex(jitem,"Sensors", &jsensors))
					{
						json_object * jsensor = NULL;
						bridge_data->cm[i].size_sen = json_object_array_length(jsensors);
						bridge_data->cm[i].index_sen = 0;
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
