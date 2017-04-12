#include <mhash.h>
//#include <stdio.h> /* printf, sprintf */
//#include <stdlib.h> /* exit */
//#include <unistd.h> /* read, write, close */
//#include <string.h> /* memcpy, memset */
//#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <uuid/uuid.h>
#include <bridge.h>

#define     AWS_ACCESS_KEY         "AKIAIW46AH4LLSITXPKQ"                         // Put your AWS access key here.  
                                                                                 // Don't put the read-only user credentials here, instead use your AWS account credentials
                                                                                 // or the credentials of an account with write access to your DynamoDB table here.
                                                                                 
#define     AWS_SECRET_ACCESS_KEY  "MXmJceGZa4caCN7bzH9Ju+62OpjnflBZgdWBPKqX"                  // Put your AWS secret access key here.
                                                                                 // Don't put the read-only user credentials here, instead use your AWS account credentials
                                                                                 // or the credentials of an account with write access to your DynamoDB table here.

#define     AWS_REGION             "us-west-2"                                   // The region where your dynamo DB table lives.
                                                                                 // Copy the _exact_ region value from this table: http://docs.aws.amazon.com/general/latest/gr/rande.html#ddb_region 

#define     AWS_HOST               "dynamodb.us-west-2.amazonaws.com"            // The endpoint host for where your dynamo DB table lives.
                                                                                 // Copy the _exact_ endpoint host from this table: http://docs.aws.amazon.com/general/latest/gr/rande.html#ddb_region 

// Other sketch configuration
#define     READING_DELAY_MINS     1      // Number of minutes to wait between readings.
#define     TIMEOUT_MS             15000  // How long to wait (in milliseconds) for a server connection to respond (for both AWS and NTP calls).

// Don't modify the below constants unless you want to play with calling other DynamoDB APIs
#define     AWS_TARGET             "DynamoDB_20120810.GetItem"
#define		AWS_BATCH_WRITE_ITEM   "DynamoDB_20120810.BatchWriteItem"
#define     AWS_SERVICE            "dynamodb"
#define     AWS_SIG_START          "AWS4" AWS_SECRET_ACCESS_KEY
#define     SHA256_HASH_LENGTH     32
#define 	AWS_4_REGUEST		   "aws4_request"
//#define     DATE_TIME			   "20160824T152700Z"
//#define     DATE                   "20160824"
#define  	INTERNET_CHECK_HOST    "www.google.com"

void *send_data_to_cloud_wrapper(void *args);

int send_data_to_cloud(hx_data *user_data, int size_hx_data);