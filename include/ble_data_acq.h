#ifndef _BLE_DATA_ACQ_H
#define _BLE_DATA_ACQ_H
#include <defines.h>
/* Len of signature in write signed packet */
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

#define GENERIC_AUDIO_UUID	"00001203-0000-1000-8000-00805f9b34fb"

#define HSP_HS_UUID		"00001108-0000-1000-8000-00805f9b34fb"
#define HSP_AG_UUID		"00001112-0000-1000-8000-00805f9b34fb"

#define HFP_HS_UUID		"0000111e-0000-1000-8000-00805f9b34fb"
#define HFP_AG_UUID		"0000111f-0000-1000-8000-00805f9b34fb"

#define ADVANCED_AUDIO_UUID	"0000110d-0000-1000-8000-00805f9b34fb"

#define A2DP_SOURCE_UUID	"0000110a-0000-1000-8000-00805f9b34fb"
#define A2DP_SINK_UUID		"0000110b-0000-1000-8000-00805f9b34fb"

#define AVRCP_REMOTE_UUID	"0000110e-0000-1000-8000-00805f9b34fb"
#define AVRCP_TARGET_UUID	"0000110c-0000-1000-8000-00805f9b34fb"

#define PANU_UUID		"00001115-0000-1000-8000-00805f9b34fb"
#define NAP_UUID		"00001116-0000-1000-8000-00805f9b34fb"
#define GN_UUID			"00001117-0000-1000-8000-00805f9b34fb"
#define BNEP_SVC_UUID		"0000000f-0000-1000-8000-00805f9b34fb"

#define PNPID_UUID		"00002a50-0000-1000-8000-00805f9b34fb"
#define DEVICE_INFORMATION_UUID	"0000180a-0000-1000-8000-00805f9b34fb"

#define GATT_UUID		"00001801-0000-1000-8000-00805f9b34fb"
#define IMMEDIATE_ALERT_UUID	"00001802-0000-1000-8000-00805f9b34fb"
#define LINK_LOSS_UUID		"00001803-0000-1000-8000-00805f9b34fb"
#define TX_POWER_UUID		"00001804-0000-1000-8000-00805f9b34fb"
#define BATTERY_UUID		"0000180f-0000-1000-8000-00805f9b34fb"
#define SCAN_PARAMETERS_UUID	"00001813-0000-1000-8000-00805f9b34fb"

#define SAP_UUID		"0000112D-0000-1000-8000-00805f9b34fb"

#define HEART_RATE_UUID			"0000180d-0000-1000-8000-00805f9b34fb"
#define HEART_RATE_MEASUREMENT_UUID	"00002a37-0000-1000-8000-00805f9b34fb"
#define BODY_SENSOR_LOCATION_UUID	"00002a38-0000-1000-8000-00805f9b34fb"
#define HEART_RATE_CONTROL_POINT_UUID	"00002a39-0000-1000-8000-00805f9b34fb"

#define HEALTH_THERMOMETER_UUID		"00001809-0000-1000-8000-00805f9b34fb"
#define TEMPERATURE_MEASUREMENT_UUID	"00002a1c-0000-1000-8000-00805f9b34fb"
#define TEMPERATURE_TYPE_UUID		"00002a1d-0000-1000-8000-00805f9b34fb"
#define INTERMEDIATE_TEMPERATURE_UUID	"00002a1e-0000-1000-8000-00805f9b34fb"
#define MEASUREMENT_INTERVAL_UUID	"00002a21-0000-1000-8000-00805f9b34fb"

#define CYCLING_SC_UUID		"00001816-0000-1000-8000-00805f9b34fb"
#define CSC_MEASUREMENT_UUID	"00002a5b-0000-1000-8000-00805f9b34fb"
#define CSC_FEATURE_UUID	"00002a5c-0000-1000-8000-00805f9b34fb"
#define SENSOR_LOCATION_UUID	"00002a5d-0000-1000-8000-00805f9b34fb"
#define SC_CONTROL_POINT_UUID	"00002a55-0000-1000-8000-00805f9b34fb"

#define RFCOMM_UUID_STR		"00000003-0000-1000-8000-00805f9b34fb"

#define HDP_UUID		"00001400-0000-1000-8000-00805f9b34fb"
#define HDP_SOURCE_UUID		"00001401-0000-1000-8000-00805f9b34fb"
#define HDP_SINK_UUID		"00001402-0000-1000-8000-00805f9b34fb"

#define HID_UUID		"00001124-0000-1000-8000-00805f9b34fb"

#define DUN_GW_UUID		"00001103-0000-1000-8000-00805f9b34fb"

#define GAP_UUID		"00001800-0000-1000-8000-00805f9b34fb"
#define PNP_UUID		"00001200-0000-1000-8000-00805f9b34fb"

#define SPP_UUID		"00001101-0000-1000-8000-00805f9b34fb"

#define OBEX_SYNC_UUID		"00001104-0000-1000-8000-00805f9b34fb"
#define OBEX_OPP_UUID		"00001105-0000-1000-8000-00805f9b34fb"
#define OBEX_FTP_UUID		"00001106-0000-1000-8000-00805f9b34fb"
#define OBEX_PCE_UUID		"0000112e-0000-1000-8000-00805f9b34fb"
#define OBEX_PSE_UUID		"0000112f-0000-1000-8000-00805f9b34fb"
#define OBEX_PBAP_UUID		"00001130-0000-1000-8000-00805f9b34fb"
#define OBEX_MAS_UUID		"00001132-0000-1000-8000-00805f9b34fb"
#define OBEX_MNS_UUID		"00001133-0000-1000-8000-00805f9b34fb"
#define OBEX_MAP_UUID		"00001134-0000-1000-8000-00805f9b34fb"

/* GATT UUIDs section */
#define GATT_PRIM_SVC_UUID				0x2800
#define GATT_SND_SVC_UUID				0x2801
#define GATT_INCLUDE_UUID				0x2802
#define GATT_CHARAC_UUID				0x2803

/* GATT Characteristic Types */
#define GATT_CHARAC_DEVICE_NAME				0x2A00
#define GATT_CHARAC_APPEARANCE				0x2A01
#define GATT_CHARAC_PERIPHERAL_PRIV_FLAG		0x2A02
#define GATT_CHARAC_RECONNECTION_ADDRESS		0x2A03
#define GATT_CHARAC_PERIPHERAL_PREF_CONN		0x2A04
#define GATT_CHARAC_SERVICE_CHANGED			0x2A05
#define GATT_CHARAC_SYSTEM_ID				0x2A23
#define GATT_CHARAC_MODEL_NUMBER_STRING			0x2A24
#define GATT_CHARAC_SERIAL_NUMBER_STRING		0x2A25
#define GATT_CHARAC_FIRMWARE_REVISION_STRING		0x2A26
#define GATT_CHARAC_HARDWARE_REVISION_STRING		0x2A27
#define GATT_CHARAC_SOFTWARE_REVISION_STRING		0x2A28
#define GATT_CHARAC_MANUFACTURER_NAME_STRING		0x2A29

/* GATT Characteristic Descriptors */
#define GATT_CHARAC_EXT_PROPER_UUID			0x2900
#define GATT_CHARAC_USER_DESC_UUID			0x2901
#define GATT_CLIENT_CHARAC_CFG_UUID			0x2902
#define GATT_SERVER_CHARAC_CFG_UUID			0x2903
#define GATT_CHARAC_FMT_UUID				0x2904
#define GATT_CHARAC_AGREG_FMT_UUID			0x2905
#define GATT_CHARAC_VALID_RANGE_UUID			0x2906
#define GATT_EXTERNAL_REPORT_REFERENCE			0x2907
#define GATT_REPORT_REFERENCE				0x2908


#define RETRY_TIMES 5
#define FAST_RETRY_TIMES 100
#define BLE_TIME_OUT	10
#define ERROR_DATA_VALUE 999.0



// ALREADY_DONE_FOR_YOU: Defining 16-bit characteristic UUID
//#define BLE_UUID_OUR_CHARACTERISTC_READ_UUID          {0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x55, 0x47, 0x00, 0x00} // Just a random, but recognizable value
//#define BLE_UUID_OUR_CHARACTERISTC_WRITE_UUID          {0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x56, 0x47, 0x00, 0x00} // Just a random, but recognizable value
//#define BLE_UUID_OUR_CHARACTERISTC_READ_UUID 0x5678
//#define BLE_UUID_OUR_CHARACTERISTC_WRITE_UUID 0x1234
#define BLE_RX_MAX_LENGTH (GATT_MTU_SIZE_DEFAULT - 3)
#define BLE_PACKET_MAX_LENGTH (GATT_MTU_SIZE_DEFAULT - 3)

#define MAX_UUID_LEN 16

int ble_data_acq(core_module *cm);

#endif
