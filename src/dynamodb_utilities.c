
#define CHUNK_SIZE 512

#include <dynamodb_utilities.h>
#include <utilities.h>
#include <errno.h>
#include <localdb_utilities.h>

char sql_query[500];
bridge *bridge_data;

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

int sync_single_raw_data(int timestamp, char *deviceID, double data, int data_quality)
{
	char payload[MAX_HTTP_REQUEST_SIZE];
	char http_request[MAX_HTTP_REQUEST_SIZE];
	char http_response[MAX_HTTP_RESPONSE_SIZE];
	char tmp_char[50];
	int http_request_len;
	int http_response_len;

	json_object *jobj_root = json_object_new_object();
	json_object *jtable_name = json_object_new_string("Hx.RawData");
	json_object_object_add(jobj_root,"TableName",jtable_name);

	json_object *jitem = json_object_new_object();

	json_object *jdevice_id = json_object_new_string(deviceID);
	json_object *jvalue = json_object_new_object();
	json_object_object_add(jvalue,"S",jdevice_id);
	json_object_object_add(jitem,"DeviceID",jvalue);

	memset(tmp_char, 0, sizeof(tmp_char));
	sprintf(tmp_char, "%d", timestamp);
	json_object *jtimestamp = json_object_new_string(tmp_char);
	json_object *jvalue1 = json_object_new_object();
	json_object_object_add(jvalue1,"N",jtimestamp);
	json_object_object_add(jitem,"EpochTimeStamp",jvalue1);

	memset(tmp_char, 0, sizeof(tmp_char));
	sprintf(tmp_char, "%f", data);
	json_object *jdata = json_object_new_string(tmp_char);
	json_object *jvalue2 = json_object_new_object();
	json_object_object_add(jvalue2,"N",jdata);
	json_object_object_add(jitem,"Value",jvalue2);

	memset(tmp_char, 0, sizeof(tmp_char));
	sprintf(tmp_char, "%d", data_quality);
	json_object *jdata_quality = json_object_new_string(tmp_char);
	json_object *jvalue3 = json_object_new_object();
	json_object_object_add(jvalue3,"N",jdata_quality);
	json_object_object_add(jitem,"Data Quality",jvalue3);

	json_object_object_add(jobj_root,"Item",jitem);

	sprintf(payload, "%s",json_object_to_json_string_ext(jobj_root, JSON_C_TO_STRING_PLAIN));

	json_object_put(jobj_root);

	int ipayloadlength = strlen(payload);

	DEBUG_PRINT("%s\n",payload);

	if (put_dynamodb_http_request(payload, ipayloadlength, http_request, &http_request_len, AWSDB_PUT_ITEM) != 0)
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

	return 0;

}


int sync_hx_data()
{
			if (check_internet() != 0)
			{
					fprintf(stderr, "no internet, cannot send data to remote\n");
					return 2;
			}
			char db_path[100];

			strcpy(db_path, HX_SQLITE_DATA_FOLDER_NAME);
			strcat(db_path, "/");
			strcat(db_path, HX_SQLITE_FILE_NAME);
			sqlite3 *db;
	//		char *zErrMsg = 0;
			sqlite3_stmt *stmt;
			int rc;

			rc = sqlite3_open(db_path, &db);

			if (rc != SQLITE_OK) {

	        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
	        sqlite3_close(db);
					return 1;
	    }


			memset(sql_query,0,sizeof(sql_query));

			strcpy (sql_query, "select * from ");
			strcat (sql_query, HX_MAIN_TABLE_NAME);
			strcat (sql_query, " where sync_status = 0;");

			sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
			printf("Got results:\n");
			while(sqlite3_step(stmt) != SQLITE_DONE) {
					char str_colID[20];
					char *err_msg = 0;
					int colID = sqlite3_column_int(stmt,0);
					int cur_timestamp = sqlite3_column_int(stmt,2);
				  char *deviceID = (char*)sqlite3_column_text(stmt,3);
					double data = sqlite3_column_double(stmt,4);
					int data_quality = sqlite3_column_int(stmt,5);
					int sync_status = 0;
					char str_sync_status[10];
					printf("timestamp=%d\n", cur_timestamp );
					printf("deviceID= %s\n", deviceID);

					if (deviceID == NULL || colID == 0 || cur_timestamp ==0)
					{
						  DEBUG_PRINT3("no data to sync\n");
							return 0;
					}

					if (sync_single_raw_data(cur_timestamp, deviceID, data, data_quality) == 0)
					{
							sync_status = 1;
					}
					else
					{
						sync_status = 0;
					}
							// mark data as updated
					memset(sql_query,0,sizeof(sql_query));
					memset(str_colID, 0, sizeof(str_colID));
					sprintf(str_colID, "%d", colID);
					strcpy (sql_query, "update ");
					strcat (sql_query, HX_MAIN_TABLE_NAME);
					strcat (sql_query, " set sync_status = ");
					memset(str_sync_status, 0, sizeof(str_sync_status));
					sprintf(str_sync_status, "%d", sync_status);
					strcat (sql_query, str_sync_status);
					strcat (sql_query, " where id=");
					strcat (sql_query, str_colID);
					strcat (sql_query, ";");

					printf("update_query=%s\n", sql_query);
					rc = sqlite3_exec(db, sql_query, 0, 0, &err_msg);


			    if (rc != SQLITE_OK) {
					fprintf(stderr, "Failed to update table\n");
					fprintf(stderr, "SQL error: %s\n", err_msg);

					sqlite3_free(err_msg);
					return 1;
				}
			}

			sqlite3_finalize(stmt);

			sqlite3_close(db);

			return 0;

}

void *db_data_handler(void *args)
{
		bridge_data = (bridge *)args;
		data_set_def *data_set;
		int ret = 0;
		int no_internet_count = 0;
		while(1)
		{

				data_set = g_async_queue_pop(bridge_data->data_queue);
				if (data_set != NULL)
				{

					printf("pop data: %d , ", data_set->timestamp);
					printf("%s , ", data_set->id);
					printf("%f , ", data_set->data);
					printf("%d\n", data_set->data_code);

					if (write_sqlite_hx_data(data_set) != 0)
					{
						fprintf(stderr, "write to local db error\n");
					}

					if (no_internet_count == 0)
					{
							ret = sync_hx_data();
							if (ret != 0)
							{
								no_internet_count++;
							}
					}
					else
					{
						no_internet_count++;

						if (no_internet_count > NO_INTERNET_CHECK_FREQUENCY)
						{
							no_internet_count = 0;
						}
					}



					g_free(data_set);
				}
		}


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
			DEBUG_PRINT("chunk_size=%d\n",size_recv);
			memcpy(resp + total_size, chunk, size_recv);
            total_size += size_recv;
           // DEBUG_PRINT("%s" , chunk);
            //reset beginning time
            gettimeofday(&begin , NULL);
        }
    }

    return total_size;
}

int send_http_request_to_dynamodb(char *http_request, int http_request_len, char *http_response, int *http_response_len){

//	int portno =        AWS_PORT_NUM;
	char *host =        AWS_HOST;

	struct addrinfo	serv_addrinfo;
	int sockfd, bytes, sent, total;
//	char response[MAX_HTTP_RESPONSE_SIZE];

	DEBUG_PRINT("Request:\n%s\n",http_request);


	/* lookup the ip address */
//	server = gethostbyname(host);
//	if (server == NULL) {
//		perror("perror, no such host");
//		return 1;
//	}

	if (resolve_ip(host, &serv_addrinfo) != 0)
	{
		perror("error, no such host");
		return 1;
	}
	/* create the socket */
	sockfd = socket(serv_addrinfo.ai_family, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
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
		perror("ERROR connecting");
		return 1;
	}


	DEBUG_PRINT("connected\n");
	/* send the request */
	total = http_request_len;
	sent = 0;
	DEBUG_PRINT("sending messageing\n");
	do {
		bytes = write(sockfd,http_request + sent,total - sent);
		if (bytes < 0)
			perror("ERROR writing send_message to socket");
		if (bytes == 0)
			break;
		sent+=bytes;
		DEBUG_PRINT("sent=%d\n",sent);
	} while (sent < total);

	/* receive the response */

	int total_recv = recv_timeout(sockfd, 1, http_response);
	/* close the socket */
	close(sockfd);

	/* process response */
	DEBUG_PRINT("Response:\n%s\n",http_response);

	(*http_response_len) = total_recv;

//	memcpy(http_response, response, (*http_response_len));


	return 0;
}

int put_dynamodb_http_request(char *payload, int payload_len, char *http_request, int *http_request_len, aws_request_num num)
{
	int j;
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

	DEBUG_PRINT("time_date=%s\n",time_date);

	DEBUG_PRINT("time_full=%s\n",time_full);

	char SIG_FULL[100];
	strcpy(SIG_FULL, AWS_SIG_START);
	strcat(SIG_FULL, bridge_data->aws_secret_access_key);

	DEBUG_PRINT("SIG_FULL = %s\n", SIG_FULL);

	td = mhash_hmac_init(MHASH_SHA256, (uint8_t*)SIG_FULL, strlen(SIG_FULL), mhash_get_hash_pblock(MHASH_SHA256));
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
//	int canonical_size;
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
/*
	canonical_size = strlen(can_str1)
				   + strlen(payloadlength)
				   + strlen(can_str2)
				   + strlen(AWS_HOST)
				   + strlen(can_str3)
				   + strlen(get_aws_request_str(num))
				   + strlen(can_str4)
				   + strlen(payloadhash);
*/
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

	DEBUG_PRINT("signature=%s\n",signature);

	char send_message[MAX_HTTP_REQUEST_SIZE];

	strcpy(send_message, "POST / HTTP/1.1\r\n");
	strcat(send_message, "Accept: */*\r\n");
	strcat(send_message, "host: ");
	strcat(send_message, AWS_HOST);
	strcat(send_message, ";");
	strcat(send_message, "\r\nx-amz-date: ");
	strcat(send_message, time_full);
	strcat(send_message, "\r\nAuthorization: AWS4-HMAC-SHA256 Credential=");
	strcat(send_message, bridge_data->aws_access_key);
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
	return 0;
}
