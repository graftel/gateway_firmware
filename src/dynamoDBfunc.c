// Functions to upload data to dynamo DB

#include <dynamoDBfunc.h>
#include <bridge.h>

void *send_data_to_cloud_wrapper(void *args)
{
	thread_arg *arguments = args;
	
	return (void *)send_data_to_cloud(arguments->user_data,arguments->size_hx_data);
}

int send_data_to_cloud(hx_data *user_data, int size_hx_data)
{
//	char password[] = "";
//	int keylen = 0;
//	int datalen = 8;
	MHASH td;
	unsigned char *mac;
	int i,j;
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
		return 1;
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
	
	uuid_t uuid;

        // generate

	
	char payload[65535];
	int payload_size;
	char buffer[50];
	strcpy(payload, "{\"RequestItems\": {\"Graftel_Cloud_Data\":[");
	
	for (i = 0; i < size_hx_data; i++)
	{
		uuid_generate_random(uuid);

        // unparse (to string)
        char uuid_str[37];      // ex. "1b4e28ba-2fa1-11d2-883f-0016d3cca427" + "\0"
        uuid_unparse_lower(uuid, uuid_str);
        printf("generate uuid=%s\n", uuid_str);
		
		strcat(payload, "{\"PutRequest\":{\"Item\":{\"UUID\":{\"S\":\"");
		strcat(payload, uuid_str);
		strcat(payload, "\"},\"DATA_TYPE\":{\"S\":\"");
		strcat(payload, user_data[i].data_type);
		
		strcat(payload, "\"},\"TIME_STAMP\":{\"N\":\"");
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"%d",(long)user_data[i].time_stamp);
		strcat (payload, buffer);
		
		strcat(payload, "\"},\"DATA\":{\"N\":\"");
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"%.15f",user_data[i].data);
		strcat (payload, buffer);
		
		strcat(payload, "\"},\"UNIT\":{\"S\":\"");
		strcat(payload, user_data[i].data_unit);
		
		strcat(payload, "\"},\"DATA_ADDRESS\":{\"S\":\"");
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"%02X",user_data[i].data_address);
		strcat (payload, buffer);
		
		strcat(payload, "\"},\"SERIAL_NUM\":{\"S\":\"");
		strcat(payload, user_data[i].serial_num);
		
		strcat(payload, "\"},\"BLE_ADDRESS\":{\"S\":\"");
		strcat(payload, user_data[i].ble_address);
		
		strcat(payload, "\"}}}},");
	}
	
	payload[strlen(payload) - 1] = ']';
	
	strcat(payload,"},\"ReturnConsumedCapacity\":\"TOTAL\"}");
	
	int ipayloadlength = strlen(payload);
	
	char payloadlength[10];
	
//	char payload[] = "{\"TableName\":\"Graftel_Data\",\"Key\":{\"Data_ID\":{\"N\":\"1\"},\"Date_Type\":{\"S\":\"Flow_Rate\"}}}";
	
	sprintf(payloadlength,"%d",ipayloadlength);

	
	td = mhash_init(MHASH_SHA256);
	mhash(td, payload, strlen(payload));
	mac = mhash_end(td);
	
	char payloadhash[2*SHA256_HASH_LENGTH+1];
	char* buf_ptr = payloadhash;
	memset(payloadhash, 0, 2*SHA256_HASH_LENGTH+1);
	for (j = 0; j < SHA256_HASH_LENGTH; j++)
	{
		buf_ptr += sprintf(buf_ptr,"%02x",mac[j]);
	}
	
	char canonical_request[65535];
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
	strcat(canonical_request, AWS_BATCH_WRITE_ITEM);
	strcat(canonical_request, can_str5);
	strcat(canonical_request, payloadhash);
	
	canonical_size = strlen(can_str1) 
				   + strlen(payloadlength) 
				   + strlen(can_str2) 
				   + strlen(AWS_HOST) 
				   + strlen(can_str3)
				   + strlen(AWS_BATCH_WRITE_ITEM)
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
	
	char final_str[65535];
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
	
	char send_message[65535];
	
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
	strcat(send_message, AWS_BATCH_WRITE_ITEM);
	strcat(send_message, "\r\n\r\n");
	strcat(send_message, payload);
	
	int portno =        80;
	char *host =        AWS_HOST;

	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd, bytes, sent, received, total;
	char response[4096];

	printf("Request:\n%s\n",send_message);

	/* create the socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
		return 1;
	}
	/* lookup the ip address */
	server = gethostbyname(host);
	if (server == NULL) {
		error("ERROR, no such host");
		return 1;
	}
	/* fill in the structure */
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

	/* connect the socket */
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR connecting");
		return 1;
	}
		

	printf("connected\n");
	/* send the request */
	total = strlen(send_message);
	sent = 0;
	printf("sending messageing\n");
	do {
		bytes = write(sockfd,send_message+sent,total-sent);
		if (bytes < 0)
			error("ERROR writing send_message to socket");
		if (bytes == 0)
			break;
		sent+=bytes;
		printf("sent=%d\n",sent);
	} while (sent < total);

	/* receive the response */
	memset(response,0,sizeof(response));
	total = sizeof(response)-1;
	received = 0;
	printf("wanting for response\n");
	do {
		bytes = read(sockfd,response+received,total-received);
		printf("bytes=%d\n",bytes);
		if (bytes < 0)
			error("ERROR reading response from socket");
		received+=bytes;
		printf("received=%d\n",received);
	} while (received < 100);

	if (received == total)
	{
		error("ERROR storing complete response from socket");
		return 1;
	}	

	/* close the socket */
	close(sockfd);

	/* process response */
	printf("Response:\n%s\n",response);

	free(user_data);
	
	return 0;
}