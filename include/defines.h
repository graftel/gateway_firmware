#ifndef _DEFINES_H
#define _DEFINES_H

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>

#define MAX_BLE_PACKET_LEN 40
#define BRIDGE_SERIAL_NUM "04A001"
#define ASSETID			  "GT_HX_170101"
#define MAX_SENSOR_ADDR 6
#define MAX_BLE_ADDR    20
#define MAX_TSYS01_COEF_LEN  5
#define MAX_TSYS01_DATA_LEN  3
#define MAX_MAC_ADDR  6
#define MAX_IP_ADDR   4
#define MAX_ERROR_MSG 10
#define MAX_SECRET_LEN 200
#define MAX_LINE 500
#define INTERNET_CHECK_HOST_NAME "www.google.com"
#define MAX_PROTOCOL_LEN      20
#define MAX_HTTP_REQUEST_SIZE 65535
#define MAX_HTTP_RESPONSE_SIZE 65535


#define MAX_RS485_ADDR_LEN 6

#define BLE_DATA_CH_POS 0
#define BLE_DATA_LEN_POS 1
#define BLE_DATA_API_POS 2
#define BLE_DATA_CMD_POS 3
#define BLE_DATA_ADDR_POS 4

#define RS485_CHANNEL_HEX 0x02


#define READ_TEMP_HEX 0x01
#define READ_COEF_HEX 0x02
#define READ_CAL_HEX 0x03
#define WRITE_CAL_HEX 0x04
#define WRITE_ENABLE_HEX 0x05

#define READ_TEMP_LEN 11
#define READ_COEF_LEN 11
#define READ_CAL_LEN  11
#define WRITE_CAL_LEN 15
#define WRITE_ENABLE_LEN 10

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
//#define     AWS_SIG_START          "AWS4" AWS_SECRET_ACCESS_KEY
#define     AWS_SIG_START          "AWS4"
#define     SHA256_HASH_LENGTH     32
#define 	AWS_4_REGUEST		   "aws4_request"
//#define     DATE_TIME			   "20160824T152700Z"
//#define     DATE                   "20160824"
#define  	INTERNET_CHECK_HOST    "www.google.com"

#define DEBUG 2
#define DEFAULT_ERR_VALUE 999.0F

#if defined(DEBUG) && DEBUG == 1
 #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
 #define DEBUG_PRINT3(fmt, args...) fprintf(stderr, fmt, ## args)
 #define DEBUG_PRINT4(fmt, args...) fprintf(stderr, fmt, ## args)
#elif defined(DEBUG) && DEBUG == 2
	#define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ## args)
  #define DEBUG_PRINT3(fmt, args...) fprintf(stderr, fmt, ## args)
  #define DEBUG_PRINT4(fmt, args...) fprintf(stderr, fmt, ## args)
#elif defined(DEBUG) && DEBUG == 3
  #define DEBUG_PRINT3(fmt, args...) fprintf(stderr, fmt, ## args)
  #define DEBUG_PRINT(fmt, args...)
  #define DEBUG_PRINT4(fmt, args...)
#elif defined(DEBUG) && DEBUG == 4
  #define DEBUG_PRINT4(fmt, args...) fprintf(stderr, fmt, ## args)
  #define DEBUG_PRINT(fmt, args...)
  #define DEBUG_PRINT3(fmt, args...)
#else
 #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif



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
	OK = 0,
  BLE_CONN_TIMEOUT,
	BLE_READ_ERR,
	BLE_WRITE_ERR,
  BLE_INTERNAL_CMD_NO_RESPONSE,
  BLE_INTERNAL_ADDR_ERROR,
	RS485_TSYS01_NO_RESPONSE,
	RS485_TSYS01_READ_ERR,
	RS485_TSYS01_WRITE_ERR,
	RS485_TSYS01_ADDRESS_VERIFY_ERR,
  UNKNOWN_PROBE_DEFINITION,
} data_code_def;

typedef enum{
	RS485 = 0,
	I2C,
	BLE,
	TCP,
	ANALOG,
	MODBUS,
} comm_protocol;

typedef enum {
	CM_RS485 = 0,
	CM_BLE,
}	core_module_protocol;

typedef struct
{
  char            id[MAX_SENSOR_ADDR];
  double          data;
  data_code_def   data_code;
  int             timestamp;
} data_set_def;

typedef struct
{
	char 			      addr[MAX_SENSOR_ADDR];
	double		      data;
  data_code_def   data_code;
	comm_protocol   proto;
} sensor;

typedef struct
{
	char 			addr[MAX_SENSOR_ADDR];
	double			battery_percentage;
	double			status;
	char			ble_addr[MAX_BLE_ADDR];
  char      protocol[MAX_PROTOCOL_LEN];
	sensor 			*sen;
	int				size_sen;
	int				index_sen;

  uint16_t   read_handle;
  uint16_t   write_handle;
  int       discovered;
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

	char			aws_access_key[MAX_SECRET_LEN];
	char		  aws_secret_access_key[MAX_SECRET_LEN];
	//Sensors:
	core_module  	*cm;
  GAsyncQueue    *data_queue;
	int				size_cm;
	int				index_cm;

	int				err_list[MAX_ERROR_MSG];
	int				size_err;

  double    config_version;
} bridge;




#endif
