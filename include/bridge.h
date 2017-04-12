#ifndef _BRIDGE_H
#define _BRIDGE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <libconfig.h>
#include <pthread.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>


#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define ATT_SIGNATURE_LEN		12

/* Attribute Protocol Opcodes */
#define ATT_OP_ERROR			0x01
#define ATT_OP_MTU_REQ			0x02
#define ATT_OP_MTU_RESP			0x03
#define ATT_OP_FIND_INFO_REQ		0x04
#define ATT_OP_FIND_INFO_RESP		0x05
#define ATT_OP_FIND_BY_TYPE_REQ		0x06
#define ATT_OP_FIND_BY_TYPE_RESP	0x07
#define ATT_OP_READ_BY_TYPE_REQ		0x08
#define ATT_OP_READ_BY_TYPE_RESP	0x09
#define ATT_OP_READ_REQ			0x0A
#define ATT_OP_READ_RESP		0x0B
#define ATT_OP_READ_BLOB_REQ		0x0C
#define ATT_OP_READ_BLOB_RESP		0x0D
#define ATT_OP_READ_MULTI_REQ		0x0E
#define ATT_OP_READ_MULTI_RESP		0x0F
#define ATT_OP_READ_BY_GROUP_REQ	0x10
#define ATT_OP_READ_BY_GROUP_RESP	0x11
#define ATT_OP_WRITE_REQ		0x12
#define ATT_OP_WRITE_RESP		0x13
#define ATT_OP_WRITE_CMD		0x52
#define ATT_OP_PREP_WRITE_REQ		0x16
#define ATT_OP_PREP_WRITE_RESP		0x17
#define ATT_OP_EXEC_WRITE_REQ		0x18
#define ATT_OP_EXEC_WRITE_RESP		0x19
#define ATT_OP_HANDLE_NOTIFY		0x1B
#define ATT_OP_HANDLE_IND		0x1D
#define ATT_OP_HANDLE_CNF		0x1E
#define ATT_OP_SIGNED_WRITE_CMD		0xD2

/* Error codes for Error response PDU */
#define ATT_ECODE_INVALID_HANDLE		0x01
#define ATT_ECODE_READ_NOT_PERM			0x02
#define ATT_ECODE_WRITE_NOT_PERM		0x03
#define ATT_ECODE_INVALID_PDU			0x04
#define ATT_ECODE_AUTHENTICATION		0x05
#define ATT_ECODE_REQ_NOT_SUPP			0x06
#define ATT_ECODE_INVALID_OFFSET		0x07
#define ATT_ECODE_AUTHORIZATION			0x08
#define ATT_ECODE_PREP_QUEUE_FULL		0x09
#define ATT_ECODE_ATTR_NOT_FOUND		0x0A
#define ATT_ECODE_ATTR_NOT_LONG			0x0B
#define ATT_ECODE_INSUFF_ENCR_KEY_SIZE		0x0C
#define ATT_ECODE_INVAL_ATTR_VALUE_LEN		0x0D
#define ATT_ECODE_UNLIKELY			0x0E
#define ATT_ECODE_INSUFF_ENC			0x0F
#define ATT_ECODE_UNSUPP_GRP_TYPE		0x10
#define ATT_ECODE_INSUFF_RESOURCES		0x11
/* Application error */
#define ATT_ECODE_IO				0x80
#define ATT_ECODE_TIMEOUT			0x81
#define ATT_ECODE_ABORTED			0x82

#define ATT_MAX_VALUE_LEN			512
#define ATT_DEFAULT_L2CAP_MTU			48
#define ATT_DEFAULT_LE_MTU			23

#define ATT_CID					4
#define ATT_PSM					31

/* Flags for Execute Write Request Operation */
#define ATT_CANCEL_ALL_PREP_WRITES		0x00
#define ATT_WRITE_ALL_PREP_WRITES		0x01

/* Find Information Response Formats */
#define ATT_FIND_INFO_RESP_FMT_16BIT		0x01
#define ATT_FIND_INFO_RESP_FMT_128BIT		0x02

#define SIM_STATE_SCAN  	0x01
#define SIM_STATE_READ  	0x02
#define SIM_STATE_STOP  	0x03

#define SIM_DATA_SUMMARY 	0x20
#define SIM_DATA_FLOWRATE   0x21
#define SIM_DATA_TEMP 		0x22

#define SIM_AD_COMPANY_ID1  0x47
#define SIM_AD_COMPANY_ID2  0x54
#define SIM_AD_TOTAL_LENGTH 46
#define SIM_AD_SERIAL_NUM_SIZE 7
#define SIM_AD_STATUS_REDAY 0x01
#define SIM_AD_STATUS_DATA_REDAY 0x02

#define SIM_WRITE_HND       0x14
#define SIM_NOTIF_HND       0x12
#define SIM_READ_HND        0x11

#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */
#define EIR_TX_POWER                0x0A  /* transmit power level */
#define EIR_DEVICE_ID               0x10  /* device ID */

#define INITIAL_USER_MODULE_NUM     50
#define DEFAULT_VORTEX_READ_TIME    0   // seconds
#define DEFAULT_CYCLE_TIME          60  // seconds

#define CONFIG_FILE_NAME  "BLE_CONFIG.cfg"

typedef struct 
{
	char addr[18];
	char serial_num[10];
	int rssi;
	int num_temp_sensors;  // max 127
	int vortex_read_time; //in seconds
	int	vortex_read_time_config;   // 0 - 255
	double total_read_time; // in seconds
	int status;
} simblee_control_module;

typedef struct 
{
  uint8_t  length;
  uint8_t  manu_code;
  uint8_t  com_id1;
  uint8_t  com_id2;
  uint8_t  num_temp_sensors;
  uint8_t  vortex_read_time;
  uint8_t  status;
  uint8_t  reserved1;
  uint8_t  reserved2;
  uint8_t  reserved3;
  uint8_t  reserved4;
  uint8_t  reserved5;
  uint8_t  rssi;
} custome_ad_data;

enum read_state_def{
	CONFIG,
	INIT,
	INIT_SCAN,
	SCAN,
	CLOSE_SCAN,
	READ_DATA,
	SET_SLEEP
};

typedef struct 
{
   time_t time_stamp;  
   double data;
   uint8_t data_address;
   char data_unit[20];
   char data_type[20];
   char serial_num[20];
   char ble_address[18];
} hx_data;

typedef struct{
  hx_data *user_data;  
  int size_hx_data;
} thread_arg;


#endif