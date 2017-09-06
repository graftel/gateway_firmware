// Functions to upload data to dynamo DB

#include <dynamoDBfunc.h>
#include <json/json.h>
#include <glib.h>
#define CHUNK_SIZE 512
int get_sensors_def_from_clouud(bridge *data);
int get_sensors_def_from_single_core_module(bridge *data, int index);
int get_core_module_def_from_cloud(bridge *data);

char *get_aws_request_str(aws_request_num num)
{
	switch(num)
	{
		case AWSDB_SCAN: return "DynamoDB_20120810.Scan";
		case AWSDB_QUERY: return "DynamoDB_20120810.Query";
		case AWSDB_GET_ITEM: return "DynamoDB_20120810.GetItem";
		case AWSDB_PUT_ITEM: return "DynamoDB_20120810.PutItem";
		case AWSDB_BATCH_WRITE_ITEM: return "DynamoDB_20120810.BatchWriteItem";
		case AWSDB_UPDATE_ITEM: return "DynamoDB_20120810.UpdateItem";
		default: return "UNKNOWN";
	}
}

void *send_data_to_cloud_wrapper(void *args)
{
	bridge *arguments = args;
	
	return (void *)send_sensor_data_to_cloud(arguments);
}

int recv_timeout(int s , int timeout, char *resp)
{
    int size_recv , total_size= 0;
    struct timeval begin , now;
    char chunk[CHUNK_SIZE];
    double timediff;
     
    //make socket non blocking
    fcntl(s, F_SETFL, O_NONBLOCK);
     
    //beginning time
    gettimeofday(&begin , NULL);
     
    while(1)
    {
        gettimeofday(&now , NULL);
         
        //time elapsed in seconds
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
         
        //if you got some data, then break after timeout
        if( total_size > 0 && timediff > timeout )
        {
            break;
        }
         
        //if you got no data at all, wait a little longer, twice the timeout
        else if( timediff > timeout*2)
        {
            break;
        }
         
        memset(chunk ,0 , CHUNK_SIZE);  //clear the variable
        if((size_recv =  read(s , chunk , CHUNK_SIZE) ) < 0)
        {
            //if nothing was received then we want to wait a little before trying again, 0.1 seconds
            usleep(10000);
        }
        else
        {
			printf("chunk_size=%d\n",size_recv);
			memcpy(resp + total_size, chunk, size_recv);
            total_size += size_recv;
           // printf("%s" , chunk);
            //reset beginning time
            gettimeofday(&begin , NULL);
        }
    }
     
    return total_size;
}

int send_http_request_to_dynamodb(char *http_request, int http_request_len, char *http_response, int *http_response_len){
		
	int portno =        AWS_PORT_NUM;
	char *host =        AWS_HOST;

	struct hostent *server;
	struct sockaddr_in serv_addr;
	struct addrinfo	serv_addrinfo;
	int sockfd, bytes, sent, received, total;
//	char response[MAX_HTTP_RESPONSE_SIZE];

	printf("Request:\n%s\n",http_request);


	/* lookup the ip address */
//	server = gethostbyname(host);
//	if (server == NULL) {
//		error("ERROR, no such host");
//		return 1;
//	}

	if (resolve_ip(host, &serv_addrinfo) != 0)
	{
		error("ERROR, no such host");
		return 1;
	}
	/* create the socket */
	sockfd = socket(serv_addrinfo.ai_family, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
		return 1;
	}

	/* fill in the structure */
//	memset(&serv_addr,0,sizeof(serv_addr));
//	serv_addr.sin_family = AF_INET;
//	serv_addr.sin_port = htons(portno);
//	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
//	memcpy(&serv_addr.sin_addr.s_addr,data->local_db_addr,MAX_IP_ADDR);

	/* connect the socket */
//	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	if (connect(sockfd, serv_addrinfo.ai_addr, serv_addrinfo.ai_addrlen))
	{
		error("ERROR connecting");
		return 1;
	}
		

	printf("connected\n");
	/* send the request */
	total = http_request_len;
	sent = 0;
	printf("sending messageing\n");
	do {
		bytes = write(sockfd,http_request + sent,total - sent);
		if (bytes < 0)
			error("ERROR writing send_message to socket");
		if (bytes == 0)
			break;
		sent+=bytes;
		printf("sent=%d\n",sent);
	} while (sent < total);

	/* receive the response */
	
	int total_recv = recv_timeout(sockfd, 1, http_response);
	/* close the socket */
	close(sockfd);

	/* process response */
	printf("Response:\n%s\n",http_response);
	
	(*http_response_len) = total_recv;
	
//	memcpy(http_response, response, (*http_response_len));
	
	
	return 0;
}

int put_dynamodb_http_request(char *payload, int payload_len, char *http_request, int *http_request_len, aws_request_num num)
{
	int i,j;
	MHASH td;
	unsigned char *mac;
	
	char signingkey[SHA256_HASH_LENGTH];
	
	time_t time_now;
	
	struct tm *tmp;
	char time_date[20];
	char time_full[20];
	
	time_now = time(NULL);
	tmp = gmtime(&time_now);
	
	if (tmp == NULL)
	{
		fprintf(stderr,"Cannot get UTC time\n");
		return -1;
	}
	
	strftime(time_full,sizeof(time_full),"%Y%m%dT%H%M%SZ",tmp);
	
	strftime(time_date,sizeof(time_date),"%Y%m%d",tmp);
	
	printf("time_date=%s\n",time_date);
	
	printf("time_full=%s\n",time_full);
	
	td = mhash_hmac_init(MHASH_SHA256, (uint8_t*)AWS_SIG_START, strlen(AWS_SIG_START), mhash_get_hash_pblock(MHASH_SHA256));
	mhash(td, time_date, strlen(time_date));
	mac = mhash_hmac_end(td);
	
	td = mhash_hmac_init(MHASH_SHA256, mac, SHA256_HASH_LENGTH, mhash_get_hash_pblock(MHASH_SHA256));
	mhash(td, AWS_REGION, strlen(AWS_REGION));
	mac = mhash_hmac_end(td);
	
	td = mhash_hmac_init(MHASH_SHA256, mac, SHA256_HASH_LENGTH, mhash_get_hash_pblock(MHASH_SHA256));
	mhash(td, AWS_SERVICE, strlen(AWS_SERVICE));
	mac = mhash_hmac_end(td);
	
	td = mhash_hmac_init(MHASH_SHA256, mac, SHA256_HASH_LENGTH, mhash_get_hash_pblock(MHASH_SHA256));
	mhash(td, AWS_4_REGUEST, strlen(AWS_4_REGUEST));
	mac = mhash_hmac_end(td);
	
	memcpy(signingkey, mac, SHA256_HASH_LENGTH);
	

        // generate
	
	int ipayloadlength = payload_len;
	
	char payloadlength[10];
	
//	char payload[] = "{\"TableName\":\"Graftel_Data\",\"Key\":{\"Data_ID\":{\"N\":\"1\"},\"Date_Type\":{\"S\":\"Flow_Rate\"}}}";
	
	sprintf(payloadlength,"%d",ipayloadlength);

	
	td = mhash_init(MHASH_SHA256);
	mhash(td, payload, payload_len);
	mac = mhash_end(td);
	
	char payloadhash[2*SHA256_HASH_LENGTH+1];
	char* buf_ptr = payloadhash;
	memset(payloadhash, 0, 2*SHA256_HASH_LENGTH+1);
	for (j = 0; j < SHA256_HASH_LENGTH; j++)
	{
		buf_ptr += sprintf(buf_ptr,"%02x",mac[j]);
	}
	
	char canonical_request[MAX_HTTP_REQUEST_SIZE];
	int canonical_size;
	char can_str1[] = "POST\n/\n\ncontent-length:";
	char can_str2[] = "\ncontent-type:application/x-amz-json-1.0\nhost:";
	char can_str3[] = ";\nx-amz-date:";
	char can_str4[] = "\nx-amz-target:";
	char can_str5[] = "\n\ncontent-length;content-type;host;x-amz-date;x-amz-target\n";
	
	strcpy(canonical_request, can_str1);
	strcat(canonical_request, payloadlength);
	strcat(canonical_request, can_str2);
	strcat(canonical_request, AWS_HOST);
	strcat(canonical_request, can_str3);
	strcat(canonical_request, time_full);
	strcat(canonical_request, can_str4);
	strcat(canonical_request, get_aws_request_str(num));
	strcat(canonical_request, can_str5);
	strcat(canonical_request, payloadhash);
	
	canonical_size = strlen(can_str1) 
				   + strlen(payloadlength) 
				   + strlen(can_str2) 
				   + strlen(AWS_HOST) 
				   + strlen(can_str3)
				   + strlen(get_aws_request_str(num))
				   + strlen(can_str4)
				   + strlen(payloadhash);
	
	td = mhash_init(MHASH_SHA256);
	mhash(td, canonical_request, strlen(canonical_request));
	mac = mhash_end(td);
	
	char canonicalhash[2*SHA256_HASH_LENGTH+1];
	char* canonicalhash_ptr = canonicalhash;
	memset(canonicalhash, 0, 2*SHA256_HASH_LENGTH+1);
	for (j = 0; j < SHA256_HASH_LENGTH; j++)
	{
		canonicalhash_ptr += sprintf(canonicalhash_ptr,"%02x",mac[j]);
	}
	
	char final_str[MAX_HTTP_REQUEST_SIZE];
	char fin_str1[] = "AWS4-HMAC-SHA256\n";
	
	
	strcpy(final_str, fin_str1);
	strcat(final_str, time_full);
	strcat(final_str, "\n");
	strcat(final_str, time_date);
	strcat(final_str, "/");
	strcat(final_str, AWS_REGION);
	strcat(final_str, "/");
	strcat(final_str, AWS_SERVICE);
	strcat(final_str, "/aws4_request\n");
	strcat(final_str, canonicalhash);
	
	td = mhash_hmac_init(MHASH_SHA256, (uint8_t*)signingkey, SHA256_HASH_LENGTH, mhash_get_hash_pblock(MHASH_SHA256));
	mhash(td, final_str, strlen(final_str));
	mac = mhash_hmac_end(td);
	
	char signature[2*SHA256_HASH_LENGTH+1];
	char* signature_ptr = signature;
	memset(signature, 0, 2*SHA256_HASH_LENGTH+1);
	for (j = 0; j < SHA256_HASH_LENGTH; j++)
	{
		signature_ptr += sprintf(signature_ptr,"%02x",mac[j]);
	}

	printf("signature=%s\n",signature);
	
	char send_message[MAX_HTTP_REQUEST_SIZE];
	
	strcpy(send_message, "POST / HTTP/1.1\r\n");
	strcat(send_message, "Accept: */*\r\n");
	strcat(send_message, "host: ");
	strcat(send_message, AWS_HOST);
	strcat(send_message, ";");
	strcat(send_message, "\r\nx-amz-date: ");
	strcat(send_message, time_full);
	strcat(send_message, "\r\nAuthorization: AWS4-HMAC-SHA256 Credential=");
	strcat(send_message, AWS_ACCESS_KEY);
	strcat(send_message, "/");
	strcat(send_message, time_date);
	strcat(send_message, "/");
	strcat(send_message, AWS_REGION);
	strcat(send_message, "/");
	strcat(send_message, AWS_SERVICE);
	strcat(send_message, "/aws4_request, SignedHeaders=content-length;content-type;host;x-amz-date;x-amz-target, Signature=");
	strcat(send_message, signature);
	strcat(send_message, "\r\ncontent-type: application/x-amz-json-1.0\r\ncontent-length: ");
	strcat(send_message, payloadlength);
	strcat(send_message, "\r\nx-amz-target: ");
	strcat(send_message, get_aws_request_str(num));
	strcat(send_message, "\r\n\r\n");
	strcat(send_message, payload);
	
	(*http_request_len) = strlen(send_message); 
	
	memcpy(http_request, send_message, (*http_request_len));
	
	return 0;
	
}

int send_payload_to_cloud(char *payload, int payload_len, char *response, int *response_len)
{
	
}

int get_def_from_cloud(bridge *data)
{
	int res = 0;
	
	res = get_core_module_def_from_cloud(data);
	
	res = get_sensors_def_from_clouud(data);
	
	return res;
}

int get_sensors_def_from_clouud(bridge *data)
{
	
	int res = 0;
	
	if (data->size_cm == 0)
	{
		g_print("no core module to read\n");
		return 1;
	}
	
	int i = 0;
	
	for (i = 0; i < data->size_cm; i++)
	{
		//reading 
		g_print("reading def: core module #%d\n",i);
		
		if (get_sensors_def_from_single_core_module(data, i) != 0)
		{
			return -1;
		}
	}
	
	return 0;
	
	
}
int free_defs(bridge *data)
{
	if (data->size_sen != 0)
	{
		g_free(data->sen);
	}

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
int get_sensors_def_from_single_core_module(bridge *data, int index)
{
	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;
	int i;
	json_object *jobj_root = json_object_new_object();
	json_object *jtable_name = json_object_new_string("Hx.DeviceConfiguration");
	
	json_object *jfilterexp = json_object_new_string("LinkDeviceID = :val");
	
	json_object *jcapacity = json_object_new_string("TOTAL");
	
	json_object *jdevice_id = json_object_new_string(data->cm[index].addr);
	json_object *jvalue = json_object_new_object();
	json_object *jexpvalue = json_object_new_object();
	
	json_object_object_add(jvalue,"S",jdevice_id);
	json_object_object_add(jexpvalue,":val",jvalue);
	
	json_object_object_add(jobj_root,"TableName",jtable_name);
	json_object_object_add(jobj_root,"FilterExpression",jfilterexp);
	json_object_object_add(jobj_root,"ExpressionAttributeValues",jexpvalue);
	json_object_object_add(jobj_root,"ReturnConsumedCapacity",jcapacity);
	
	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));
	
	json_object_put(jobj_root);
	
	int ipayloadlength = strlen(payload);
	
//	printf("%s\n",payload);
	
	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_SCAN) != 0)
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
	

	json_object * jobj = json_tokener_parse(json_start);     
	int num_core = 0;
	json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/
		if (strcmp(key, "Items") == 0)
		{
			printf("found Array\n");
			json_object * jitems = val;
			json_object * jitem;
			int arraylen = json_object_array_length(jitems);
			
			data->cm[index].index_sen = 0;
			data->cm[index].size_sen= arraylen;
			data->cm[index].sen = g_try_new0(sensor, arraylen);
			
			printf("numbers of sensors=%d\n", arraylen);
			for (i = 0; i < arraylen; i++)
			{
				jitem =  json_object_array_get_idx(jitems, i);
				
				json_object *jtmp, *jtmp1;
				
				if (json_object_object_get_ex(jitem,"DeviceID", &jtmp))
				{
					if (json_object_object_get_ex(jtmp,"S", &jtmp1))
					{
						data->cm[index].sen[i].size_err = 0;
						g_stpcpy (data->cm[index].sen[i].addr,json_object_get_string(jtmp1));
					}
				}
				
				if (json_object_object_get_ex(jitem,"TechDetail", &jtmp))
				{
					if (json_object_object_get_ex(jtmp,"S", &jtmp1))
					{
						if (strstr(json_object_get_string(jtmp1), "RS485_TSYS01") != NULL)
						{
							g_print("sensor #%d, is RS485\n", i);
							data->cm[index].sen[i].proto = RS485;
						}
						else if (strstr(json_object_get_string(jtmp1), "ANALOG") != NULL)
						{
							g_print("sensor #%d, is ANALOG\n", i);
							data->cm[index].sen[i].proto = ANALOG;
						}
						
					}
				}
			}
			
		}
    }

	json_object_put(jobj);
	return 0;
	
}

char *get_alert_msg(err_code err)
{
	
	switch(err)
	{
		case BLE_CONN_TIMEOUT:
			return "BLE Connection Timeout";
		case BLE_READ_ERR:
			return "BLE Read Error";
		case BLE_WRITE_ERR:
			return "BLE Write Error";
		case RS485_TSYS01_NO_RESPONSE:
			return "RS485 Temperature Sensor No response";
		case RS485_TSYS01_READ_ERR:
			return "RS485 Temperature Sensor Read Error";
		case RS485_TSYS01_WRITE_ERR:
			return "RS485 Temperature Sensor Write Error";
		case RS485_TSYS01_ADDRESS_VERIFY_ERR:
			return "RS485 Temperature Sensor Address Error";
	}
}

int update_device_status(bridge *data)
{
	int i,j,k;
	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;
	int payload_size;
	char buffer[50];
	
	
	
	
	json_object *jobj_root = json_object_new_object();
	
	json_object *jobj_table = json_object_new_string("Hx.DeviceConfiguration");
	
	
	
	json_object *jobj_key = json_object_new_object();
	
	json_object *jobj_deviceid = json_object_new_object();
	json_object *jobj_deviceid_str = json_object_new_string("02A001");
	
	json_object *jobj_type = json_object_new_object();
	json_object *jobj_type_str = json_object_new_string("Temperature");
	
	json_object *jobj_update_exp = json_object_new_string("set DeviceStatus = :val1");
	
	json_object *jobj_exp_attr = json_object_new_object();
	
	json_object *jobj_exp_attr_item = json_object_new_object();
	
	json_object *jobj_exp_attr_item_str = json_object_new_string("OK");
	
	json_object *jobj_return_value =  json_object_new_string("ALL_NEW");
	
	json_object_object_add(jobj_root,"TableName",jobj_table);
	json_object_object_add(jobj_deviceid,"S",jobj_deviceid_str);
	json_object_object_add(jobj_type,"S",jobj_type_str);
	json_object_object_add(jobj_key,"DeviceID",jobj_deviceid);
	json_object_object_add(jobj_key,"Type",jobj_type);
	
	json_object_object_add(jobj_root,"Key",jobj_key);
	
	json_object_object_add(jobj_root,"UpdateExpression",jobj_update_exp);
	
	json_object_object_add(jobj_exp_attr_item,"S",jobj_exp_attr_item_str);
	json_object_object_add(jobj_exp_attr,":val1",jobj_exp_attr_item);
	
	json_object_object_add(jobj_root,"ExpressionAttributeValues",jobj_exp_attr);
	
	json_object_object_add(jobj_root,"ReturnValues",jobj_return_value);
	
	
	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));
	
	int ipayloadlength = strlen(payload);

	g_print("**************************************************\n");
	g_print("*********************UPDATE ITEM **************************\n");
	printf("%s\n",payload);
	g_print("**************************************************\n");
	g_print("*********************UPDATE ITEM ************************\n");
	json_object_put(jobj_root);
	memset(http_request,0,sizeof(http_request));
	http_request_len = 0;
	
	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_UPDATE_ITEM) != 0)
	{
		return -1;	
	}
	
	memset(http_response,0,sizeof(http_response));
	http_response_len = 0;
	
	if (send_http_request_to_dynamodb(http_request, http_request_len, http_response, &http_response_len) != 0)
	{
		return -1;
	}
	
	
	
	return 0;
	
}

int send_alert_to_cloud(bridge *data)
{
	int i,j,k;
	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;
	int payload_size;
	char buffer[50];
	
	json_object *jobj_root = json_object_new_object();
	json_object *jobj_table = json_object_new_object();
	json_object *jdataarray = json_object_new_array();
	
	for (i = 0; i < data->size_cm; i++)
	{
		for (j = 0; j < data->cm[i].size_err;j++)
		{
			json_object *jone_set_request = json_object_new_object();
			json_object *jsingle_put_request = json_object_new_object();
			json_object *jsingle_item = json_object_new_object();
			
			json_object *jdevice_id = json_object_new_object();
			
			json_object *jdevice_id_str = json_object_new_string(data->cm[i].addr);
			json_object_object_add(jdevice_id,"S",jdevice_id_str);
			
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"%d",data->current_timestamp);
			
			json_object *jtime = json_object_new_object();
			
			json_object *jtime_str = json_object_new_string(buffer);
			json_object_object_add(jtime,"N",jtime_str);

			
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"%s",get_alert_msg(data->cm[i].err_list[j]));
			
			json_object *jvalue = json_object_new_object();
			
			json_object *jvalue_str = json_object_new_string(buffer);
			json_object_object_add(jvalue,"S",jvalue_str);
			
			
			
			json_object_object_add(jsingle_item,"DeviceID",jdevice_id);
			json_object_object_add(jsingle_item,"EpochTimeStamp",jtime);
			json_object_object_add(jsingle_item,"ErrorMessage",jvalue);
			
			json_object_object_add(jsingle_put_request,"Item",jsingle_item);
			json_object_object_add(jone_set_request,"PutRequest",jsingle_put_request);
			
			json_object_array_add(jdataarray, jone_set_request);
		
		}
	}
	

	for (i = 0; i < data->size_cm; i++)
	{
		for (j = 0; j < data->cm[i].size_sen; j++)
		{
			for (k = 0; k < data->cm[i].sen[j].size_err;k++)
			{
				json_object *jone_set_request = json_object_new_object();
				json_object *jsingle_put_request = json_object_new_object();
				json_object *jsingle_item = json_object_new_object();
				
				json_object *jdevice_id = json_object_new_object();
				
				json_object *jdevice_id_str = json_object_new_string(data->cm[i].sen[j].addr);
				json_object_object_add(jdevice_id,"S",jdevice_id_str);
				
				memset(buffer,0,sizeof(buffer));
				sprintf(buffer,"%d",data->current_timestamp);
				
				json_object *jtime = json_object_new_object();
				
				json_object *jtime_str = json_object_new_string(buffer);
				json_object_object_add(jtime,"N",jtime_str);

				
				memset(buffer,0,sizeof(buffer));
				sprintf(buffer,"%s",get_alert_msg(data->cm[i].sen[j].err_list[k]));
				
				json_object *jvalue = json_object_new_object();
				
				json_object *jvalue_str = json_object_new_string(buffer);
				json_object_object_add(jvalue,"S",jvalue_str);
				
				
				
				json_object_object_add(jsingle_item,"DeviceID",jdevice_id);
				json_object_object_add(jsingle_item,"EpochTimeStamp",jtime);
				json_object_object_add(jsingle_item,"ErrorMessage",jvalue);
				
				json_object_object_add(jsingle_put_request,"Item",jsingle_item);
				json_object_object_add(jone_set_request,"PutRequest",jsingle_put_request);
				
				json_object_array_add(jdataarray, jone_set_request);
				
			}

				
		}
	//	uuid_generate_random(uuid);

        // unparse (to string)
        //char uuid_str[37];      // ex. "1b4e28ba-2fa1-11d2-883f-0016d3cca427" + "\0"
        //uuid_unparse_lower(uuid, uuid_str);
       // printf("generate uuid=%s\n", uuid_str);
		

	}
	
	json_object_object_add(jobj_table,"Hx.Alerts", jdataarray);
	json_object_object_add(jobj_root,"RequestItems", jobj_table);
	
	json_object *jcapacity = json_object_new_string("TOTAL");
	json_object_object_add(jobj_root,"ReturnConsumedCapacity", jcapacity);
	
	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));
	
	int ipayloadlength = strlen(payload);

	g_print("**************************************************\n");
	g_print("*********************ALERT **************************\n");
	printf("%s\n",payload);
	g_print("**************************************************\n");
	g_print("*********************ALERT ************************\n");
	json_object_put(jobj_root);
	memset(http_request,0,sizeof(http_request));
	http_request_len = 0;
	
	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_BATCH_WRITE_ITEM) != 0)
	{
		return -1;
	}
	
	memset(http_response,0,sizeof(http_response));
	http_response_len = 0;
	
	if (send_http_request_to_dynamodb(http_request, http_request_len, http_response, &http_response_len) != 0)
	{
		return -1;
	}
	
	return 0;
}

int get_core_module_def_from_cloud(bridge *data)
{
	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;
	char ble_buffer[MAX_BLE_ADDR], addr_buffer[MAX_SENSOR_ADDR];
	int i;
	json_object *jobj_root = json_object_new_object();
	json_object *jtable_name = json_object_new_string("Hx.DeviceConfiguration");
	
	json_object *jfilterexp = json_object_new_string("LinkDeviceID = :val");
	
	json_object *jcapacity = json_object_new_string("TOTAL");
	
	json_object *jdevice_id = json_object_new_string(BRIDGE_SERIAL_NUM);
	json_object *jvalue = json_object_new_object();
	json_object *jexpvalue = json_object_new_object();
	
	json_object_object_add(jvalue,"S",jdevice_id);
	json_object_object_add(jexpvalue,":val",jvalue);
	
	json_object_object_add(jobj_root,"TableName",jtable_name);
	json_object_object_add(jobj_root,"FilterExpression",jfilterexp);
	json_object_object_add(jobj_root,"ExpressionAttributeValues",jexpvalue);
	json_object_object_add(jobj_root,"ReturnConsumedCapacity",jcapacity);
	
	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));

	json_object_put(jobj_root);	
	
	int ipayloadlength = strlen(payload);
	
//	printf("%s\n",payload);
	
	put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_SCAN);
	
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
	
	json_object * jobj = json_tokener_parse(json_start);     
	int num_core = 0;
	json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/
		if (strcmp(key, "Items") == 0)
		{
			printf("found Array\n");
			json_object * jitems = val;
			json_object * jitem;
			int arraylen = json_object_array_length(jitems);
			

			data->cm = g_try_new0(core_module, arraylen);
			data->index_cm = 0;
			data->size_cm = arraylen;
			
			
			
			printf("numbers of core_module=%d\n", arraylen);
			for (i = 0; i < arraylen; i++)
			{
				jitem =  json_object_array_get_idx(jitems, i);
				
				json_object *jble_addr_root, *jble_addr_value, *jaddr_root, *jaddr_value;
				
				if (json_object_object_get_ex(jitem,"Bluetooth_Address", &jble_addr_root))
				{
					if (json_object_object_get_ex(jble_addr_root,"S", &jble_addr_value))
					{
						memset(ble_buffer,0,sizeof(ble_buffer));
						g_stpcpy(ble_buffer, json_object_get_string(jble_addr_value));

					}
				}
				
				if (json_object_object_get_ex(jitem,"DeviceID", &jaddr_root))
				{
					if (json_object_object_get_ex(jaddr_root,"S", &jaddr_value))
					{
						memset(addr_buffer,0,sizeof(addr_buffer));
						g_stpcpy(addr_buffer, json_object_get_string(jaddr_value));
					}
				}
				data->cm[i].size_err = 0;
			//	g_print("addr_buffer=%s\n",addr_buffer);
				
				memcpy(data->cm[i].addr, addr_buffer, sizeof(addr_buffer));
				//g_stpcpy (data->cm[i].addr,addr_buffer);
				
			//	g_print("ble_buffer=%s\n",ble_buffer);
				
				memcpy(data->cm[i].ble_addr, ble_buffer, sizeof(ble_buffer));
				//g_stpcpy (data->cm[i].ble_addr,ble_buffer);
				
			}
			
		}
    }
	json_object_put(jobj);	
	g_print("*******START LIST OF CORE_MODULE*********\n");
	for(i = 0; i < data->size_cm; i++)
	{
		
		g_print("Core Moudle #%d \n", i);
		g_print("DeviceID=%s\n",data->cm[i].addr);
		g_print("BLE_ADDR=%s\n",data->cm[i].ble_addr);
		
	}
	g_print("*******END LIST OF CORE_MODULE*********\n");
	
	
	return 0;
}

int send_sensor_data_to_cloud(bridge *data)
{
        // generate
	int i,j;
	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	int http_request_len;
	int http_response_len;
	int payload_size;
	char buffer[50];
	
	json_object *jobj_root = json_object_new_object();
	json_object *jobj_table = json_object_new_object();
	json_object *jdataarray = json_object_new_array();
	
	

	for (i = 0; i < data->size_cm; i++)
	{
		for (j = 0; j < data->cm[i].size_sen; j++)
		{
			json_object *jone_set_request = json_object_new_object();
			json_object *jsingle_put_request = json_object_new_object();
			json_object *jsingle_item = json_object_new_object();
			
			json_object *jdevice_id = json_object_new_object();
			
			json_object *jdevice_id_str = json_object_new_string(data->cm[i].sen[j].addr);
			json_object_object_add(jdevice_id,"S",jdevice_id_str);
			
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"%d",data->current_timestamp);
			
			json_object *jtime = json_object_new_object();
			
			json_object *jtime_str = json_object_new_string(buffer);
			json_object_object_add(jtime,"N",jtime_str);

			
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"%f",data->cm[i].sen[j].data);
			
			json_object *jvalue = json_object_new_object();
			
			json_object *jvalue_str = json_object_new_string(buffer);
			json_object_object_add(jvalue,"N",jvalue_str);
			
			
			
			json_object_object_add(jsingle_item,"DeviceID",jdevice_id);
			json_object_object_add(jsingle_item,"EpochTimeStamp",jtime);
			json_object_object_add(jsingle_item,"Value",jvalue);
			
			json_object_object_add(jsingle_put_request,"Item",jsingle_item);
			json_object_object_add(jone_set_request,"PutRequest",jsingle_put_request);
			
			json_object_array_add(jdataarray, jone_set_request);
				
		}
	//	uuid_generate_random(uuid);

        // unparse (to string)
        //char uuid_str[37];      // ex. "1b4e28ba-2fa1-11d2-883f-0016d3cca427" + "\0"
        //uuid_unparse_lower(uuid, uuid_str);
       // printf("generate uuid=%s\n", uuid_str);
		

	}
	
	json_object_object_add(jobj_table,"Hx.RawData", jdataarray);
	json_object_object_add(jobj_root,"RequestItems", jobj_table);
	
	json_object *jcapacity = json_object_new_string("TOTAL");
	json_object_object_add(jobj_root,"ReturnConsumedCapacity", jcapacity);
	
	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));
	
	int ipayloadlength = strlen(payload);
	
//	printf("%s\n",payload);
	json_object_put(jobj_root);
	
	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_BATCH_WRITE_ITEM) != 0)
	{
		return -1;
	}
	
	if (send_http_request_to_dynamodb(http_request, http_request_len, http_response, &http_response_len) != 0)
	{
		return -1;
	}
	
	
	json_object *jobj_root_calData = json_object_new_object();
	json_object *jobj_table_calData = json_object_new_string("Hx.PendingCalculation");
	
	json_object *jput_item = json_object_new_object();
	
	json_object *jasset_id = json_object_new_object();
	
	json_object *jasset_id_str = json_object_new_string(ASSETID);
	
	json_object_object_add(jasset_id,"S",jasset_id_str);
	
	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"%d",data->current_timestamp);

	json_object *j_cur_time = json_object_new_object();
	
	json_object *j_cur_time_str = json_object_new_string(buffer);
	json_object_object_add(j_cur_time,"N",j_cur_time_str);
	
	json_object *jpriority = json_object_new_object();
	
	json_object *jpriority_str = json_object_new_string("1");
	
	json_object_object_add(jpriority,"N",jpriority_str);
	
	json_object_object_add(jput_item,"AssetID", jasset_id);
	json_object_object_add(jput_item,"EpochTimeStamp", j_cur_time);
	json_object_object_add(jput_item,"Priority", jpriority);
	
	json_object_object_add(jobj_root_calData,"TableName", jobj_table_calData);
	json_object_object_add(jobj_root_calData,"Item", jput_item);
	
	memset(payload,0,sizeof(payload));
	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root_calData, JSON_C_TO_STRING_PLAIN));
	
	ipayloadlength = strlen(payload);
	
//	printf("%s\n",payload);
	json_object_put(jobj_root_calData);
	memset(http_request,0,sizeof(http_request));
	
	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_PUT_ITEM) != 0)
	{
		return -1;
	}
	
	memset(http_response,0,sizeof(http_response));
	if (send_http_request_to_dynamodb(http_request, http_request_len, http_response, &http_response_len) != 0)
	{
		return -1;
	}
	
	
	
//	send_alert_to_cloud(data);
	
//	update_device_status(data);
	
	return 0;
}
