#ifndef _DEFINES_H
#define _DEFINES_H

#define BRIDGE_SERIAL_NUM "04A001"
#define ASSETID			  "GT_HX_170101"
#define MAX_SENSOR_ADDR 6
#define MAX_BLE_ADDR    20
#define MAX_TSYS01_COEF_LEN  5
#define MAX_TSYS01_DATA_LEN  3
#define MAX_MAC_ADDR  6
#define MAX_IP_ADDR   4
#define MAX_ERROR_MSG 10
#define MAX_LINE 500
#define INTERNET_CHECK_HOST_NAME "www.google.com"

#define MAX_HTTP_REQUEST_SIZE 65535
#define MAX_HTTP_RESPONSE_SIZE 65535

#define     AWS_ACCESS_KEY         "AKIAJ5YW27C7WNKRXB4Q"                         // Put your AWS access key here.  
                                                                                 // Don't put the read-only user credentials here, instead use your AWS account credentials
                                                                                 // or the credentials of an account with write access to your DynamoDB table here.
                                                                                 
#define     AWS_SECRET_ACCESS_KEY  "KZQw/w182YfzPhInrBj0oG15I+79YFgZgVnsLAIt"                  // Put your AWS secret access key here.
                                                                                 // Don't put the read-only user credentials here, instead use your AWS account credentials
                                                                                 // or the credentials of an account with write access to your DynamoDB table here.

#define     AWS_REGION             "us-east-1"                                   // The region where your dynamo DB table lives.
                                                                                 // Copy the _exact_ region value from this table: http://docs.aws.amazon.com/general/latest/gr/rande.html#ddb_region 

#define     AWS_HOST               "dynamodb.us-east-1.amazonaws.com"            // The endpoint host for where your dynamo DB table lives.
                                                                                 // Copy the _exact_ endpoint host from this table: http://docs.aws.amazon.com/general/latest/gr/rande.html#ddb_region 
#define		AWS_PORT_NUM			80
																				 
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

#define DEBUG 1
#define DEFAULT_ERR_VALUE 999.0F

#include <time.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
	AWSDB_SCAN = 0,
	AWSDB_QUERY,
	AWSDB_GET_ITEM,
	AWSDB_PUT_ITEM,
	AWSDB_BATCH_WRITE_ITEM,
	AWSDB_UPDATE_ITEM,
} aws_request_num;

typedef enum {
	RS485_TSYS01 = 0,
	RS485_FLOW_METER,
	I2C_TSYS01,
	I2C_FLOW_METER,
	ANALOG_METER0,
	ANALOG_METER1,
	ANALOG_METER2,
	ANALOG_METER3,
	CORE_MODULE,
	BRIDGE,
} sensor_type;

typedef enum {
	STEP1 = 0,
	STEP2,
	STEP3,
	STEP4,
	STEP5,
	STEP6,
	STEP7,
	STEP8,
	STEP9,
	STEP10,
	STEP11,
	STEP12,
	STEP13,
} cmd_step_enum;

typedef enum {
	BLE_CONN_TIMEOUT = 0,
	BLE_READ_ERR,
	BLE_WRITE_ERR,
	RS485_TSYS01_NO_RESPONSE,
	RS485_TSYS01_READ_ERR,
	RS485_TSYS01_WRITE_ERR,
	RS485_TSYS01_ADDRESS_VERIFY_ERR,
} err_code;

typedef enum{
	RS485 = 0,
	I2C,
	BLE,
	TCP,
	ANALOG,
	MODBUS,
} comm_protocol;

typedef struct
{
	char 			addr[MAX_SENSOR_ADDR];
	double		    data;	
	comm_protocol   proto;
	int				err_list[MAX_ERROR_MSG];
	int				size_err;
} sensor;

typedef struct
{
	char 			addr[MAX_SENSOR_ADDR];
	double			battery_percentage;
	double			status;
	char			ble_addr[MAX_BLE_ADDR];
	
	sensor 			*sen;
	int				size_sen;
	int				index_sen;
	
	int				err_list[MAX_ERROR_MSG];
	int				size_err;
} core_module;

typedef struct
{
	char 			addr[MAX_SENSOR_ADDR];
	double			value;
	uint8_t			wifi_addr[MAX_MAC_ADDR];
	uint8_t			ethernet_addr[MAX_MAC_ADDR];
	
	char			local_db_addr[MAX_IP_ADDR];
	int				local_db_port;
	int 			current_timestamp;
	
	
	//Sensors:
	core_module  	*cm;
	int				size_cm;
	int				index_cm;
	sensor 			*sen;
	int				size_sen;
	int				index_sen;
	
	int				err_list[MAX_ERROR_MSG];
	int				size_err;
} bridge;




#endif
