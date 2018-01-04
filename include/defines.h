#ifndef _DEFINES_H
#define _DEFINES_H

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>

#define MAX_BLE_PACKET_LEN 40
#define MAX_UART_PACKET_LEN 64
#define BRIDGE_SERIAL_NUM "04A001"
#define ASSETID			  "GT_HX_170101"
#define MAX_SENSOR_ADDR 7
#define ADDR_LEN_I2C_PROBE 6
#define ADDR_LEN_12_CH 7
#define ADDR_LEM_CORE_MODULE 6
#define ADDR_LEN_DP_SENSOR 6
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
#define MAX_FLOAT_VALUE_LIMIT 20

#define CONFIG_DEVICE_ID 		"DeviceID"
#define CONFIG_CORE_MODULES 	"CoreModules"
#define CONFIG_BLE_ADDRESS 		"Bluetooth_Address"
#define CONFIG_PROTOCOL	   		"Protocol"
#define CONFIG_SENSORS	   		"Sensors"
#define CONFIG_VERSION			"ConfigVersion"
#define CONFIG_DAQ_INTERVAL "DAQInterval"
#define MAX_RS485_ADDR_LEN 6

#define BLE_DATA_CH_POS 0
#define BLE_DATA_LEN_POS 1
#define BLE_DATA_API_POS 2
#define BLE_DATA_CMD_POS 3
#define BLE_DATA_ADDR_POS 4

#define RS485_I2C_CHANNEL_HEX 0x02
#define RS485_12_CH_CHANNEL_HEX 0x03
#define BLE_DP_HEADER 0x05
#define BLE_DP_READ_LEN 6
#define BLE_DP_READ_DP 0x01


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

#define READ_12_CH_TEMP_LEN 12
#define ERROR_CODE_STR "ERR"
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
  PROTOCOL_BLE_CONN_TIMEOUT,
	PROTOCOL_BLE_READ_ERR,
	PROTOCOL_BLE_WRITE_ERR,
  PROTOCOL_BLE_INTERNAL_CMD_NO_RESPONSE,
  PROTOCOL_BLE_INTERNAL_ADDR_ERROR,
  // --------------------------------------
  PROTOCOL_UART_START_ERR = 10,
  PROTOCOL_UART_NO_RESPONSE,
  PROTOCOL_UART_PARSE_ERR,
  PROTOCOL_UART_INTERNAL_ERR,
  // ----------------------------------------
  SENSOR_GENERAL_START_ERR = 20,
  SENSOR_GENERAL_UNKNOWN_DEFINITION,
  // ----------------------------------------
  SENSOR_TSYS01_START_ERR = 30,
	SENSOR_TSYS01_NO_RESPONSE,
  SENSOR_TSYS01_DATA_PARSE_ERR,
  SENSOR_TSYS01_CHECK_SUM_ERR,
  SENSOR_TSYS01_WRONG_ADDR,
  SENSOR_TSYS01_INTERNAL_ERR,
  // ----------------------------------------
  SENSOR_12_CH_START_ERR = 40,
  SENSOR_12_CH_NO_RESPONSE,
  SENSOR_12_CH_DATA_PARSE_ERR,
  SENSOR_12_CH_CHECK_SUM_ERR,
  SENSOR_12_CH_WRONG_ADDR,
  SENSOR_12_CH_INTERNAL_ERR,
  // ----------------------------------------
  SENSOR_DP_START_ERR = 50,
  SENSOR_DP_NO_RESPONSE,
  SENSOR_DP_DATA_PARSE_ERR,
  SENSOR_DP_CHECK_SUM_ERR,
  SENSOR_DP_WRONG_ADDR,
  SENSOR_DP_INTERNAL_ERR,
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
	//Sensors:
	core_module  	*cm;
  GAsyncQueue    *data_queue;
	int				size_cm;
	int				index_cm;

	int				err_list[MAX_ERROR_MSG];
	int				size_err;
  int     daq_interval;         // seconds
	int    config_version;
} bridge;




#endif
