#include <mhash.h>
#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <time.h>

#define     AWS_ACCESS_KEY         "AKIAJ5YW27C7WNKRXB4Q"                         // Put your AWS access key here.  
                                                                                 // Don't put the read-only user credentials here, instead use your AWS account credentials
                                                                                 // or the credentials of an account with write access to your DynamoDB table here.
                                                                                 
#define     AWS_SECRET_ACCESS_KEY  "KZQw/w182YfzPhInrBj0oG15I+79YFgZgVnsLAIt"                  // Put your AWS secret access key here.
                                                                                 // Don't put the read-only user credentials here, instead use your AWS account credentials
                                                                                 // or the credentials of an account with write access to your DynamoDB table here.

#define     AWS_REGION             "us-east-1"                                   // The region where your dynamo DB table lives.
                                                                                 // Copy the _exact_ region value from this table: http://docs.aws.amazon.com/general/latest/gr/rande.html#ddb_region 

#define     AWS_HOST               "dynamodb.us-west-2.amazonaws.com"            // The endpoint host for where your dynamo DB table lives.
                                                                                 // Copy the _exact_ endpoint host from this table: http://docs.aws.amazon.com/general/latest/gr/rande.html#ddb_region 

// Other sketch configuration
#define     READING_DELAY_MINS     1      // Number of minutes to wait between readings.
#define     TIMEOUT_MS             15000  // How long to wait (in milliseconds) for a server connection to respond (for both AWS and NTP calls).

// Don't modify the below constants unless you want to play with calling other DynamoDB APIs
#define     AWS_TARGET             "DynamoDB_20120810.GetItem"
#define     AWS_SERVICE            "dynamodb"
#define     AWS_SIG_START          "AWS4" AWS_SECRET_ACCESS_KEY
#define     SHA256_HASH_LENGTH     32
#define 	AWS_4_REGUEST		   "aws4_request"
//#define     DATE_TIME			   "20160824T152700Z"
//#define     DATE                   "20160824"

 int main()
 {

        char password[] = "";
        int keylen = 0;
        char date[] = "20160824";
        int datalen = 8;
        MHASH td;
        unsigned char *mac;
        int j;
		char signingkey[SHA256_HASH_LENGTH];
		
		time_t time_now;
		
		struct tm *tmp;
		char time_data[20];
		char time_date1[9];
		char time_full[17];
		time_now = time(NULL);
		tmp = gmtime(&time_now);
		
		if (tmp == NULL)
		{
			fprintf(stderr,"Cannot get UTC time\n");
			return 1;
		}
		
		strftime(time_full,sizeof(time_full),"%Y%m%dT%H%M%SZ",tmp);
		
		memset(time_date1,0,sizeof(time_date1));
		
		memcpy(time_date1, time_full, 8);
		
		td = mhash_hmac_init(MHASH_SHA256, (uint8_t*)AWS_SIG_START, strlen(AWS_SIG_START), mhash_get_hash_pblock(MHASH_SHA256));
        mhash(td, time_date1, strlen(time_date1));
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
		
		char payload[] = "{\"TableName\":\"Graftel_Data\",\"Key\":{\"Data_ID\":{\"N\":\"1\"},\"Date_Type\":{\"S\":\"Flow_Rate\"}}}";
		char payloadlength[] = "86";

		
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
		
		char canonical_request[1024];
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
		strcat(canonical_request, AWS_TARGET);
		strcat(canonical_request, can_str5);
		strcat(canonical_request, payloadhash);
		
		canonical_size = strlen(can_str1) 
					   + strlen(payloadlength) 
					   + strlen(can_str2) 
					   + strlen(AWS_HOST) 
					   + strlen(can_str3)
					   + strlen(AWS_TARGET)
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
		
		char final_str[1024];
		char fin_str1[] = "AWS4-HMAC-SHA256\n";
		
		
		strcpy(final_str, fin_str1);
		strcat(final_str, time_full);
		strcat(final_str, "\n");
		strcat(final_str, time_date1);
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
		
		char send_message[1024];
		
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
		strcat(send_message, time_date1);
		strcat(send_message, "/");
		strcat(send_message, AWS_REGION);
		strcat(send_message, "/");
		strcat(send_message, AWS_SERVICE);
		strcat(send_message, "/aws4_request, SignedHeaders=content-length;content-type;host;x-amz-date;x-amz-target, Signature=");
		strcat(send_message, signature);
		strcat(send_message, "\r\ncontent-type: application/x-amz-json-1.0\r\ncontent-length: ");
		strcat(send_message, payloadlength);
		strcat(send_message, "\r\nx-amz-target: ");
		strcat(send_message, AWS_TARGET);
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
		if (sockfd < 0) error("ERROR opening socket");

		/* lookup the ip address */
		server = gethostbyname(host);
		if (server == NULL) error("ERROR, no such host");

		/* fill in the structure */
		memset(&serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(portno);
		memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

		/* connect the socket */
		if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
			error("ERROR connecting");

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
			error("ERROR storing complete response from socket");

		/* close the socket */
		close(sockfd);

		/* process response */
		printf("Response:\n%s\n",response);

		return 0;
		
 }