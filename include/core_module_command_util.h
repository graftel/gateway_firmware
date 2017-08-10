#ifndef _CORE_MODULE_COMMAND_UTIL_H
#define _CORE_MODULE_COMMAND_UTIL_H

#include <defines.h>

 #define MAX_RS485_ADDR_LEN 6

#define BLE_DATA_CH_POS 0
#define BLE_DATA_LEN_POS 1
#define BLE_DATA_CMD_POS 2
#define BLE_DATA_ADDR_POS 3

#define RS485_CHANNEL_HEX 0x03


#define READ_TEMP_HEX 0x01
#define READ_COEF_HEX 0x02
#define READ_CAL_HEX 0x03
#define WRITE_CAL_HEX 0x04
#define WRITE_ENABLE_HEX 0x05

#define READ_TEMP_LEN 10
#define READ_COEF_LEN 11
#define READ_CAL_LEN  11
#define WRITE_CAL_LEN 15
#define WRITE_ENABLE_LEN 10
/*
typedef enum{
	READ_TEMP = 0,
	READ_COEF0,
	READ_COEF1,
	READ_COEF2,
	READ_COEF3,
	READ_COEF4,
	READ_CAL_A,
	READ_CAL_B,
	READ_CAL_C,
	WRITE_CAL_A,
	WRITE_CAL_B,
	WRITE_CAL_C,
	WRITE_ENABLE,
} tsys01_cmd_type;

typedef struct
{
	int status;
	
} cm_ble_cmd;

int get_next_ble_cmd(sensor_def *sensor_def_data);
int parse_ble_cmd();

int get_ble_cmd(cmd_type type, char *addr, uint8_t *ble_cmd, uint8_t *len);
int parse_ble_cmd(uint8_t *ble_cmd, uint8_t *result, uint8_t *len); */
int get_ble_cmd(bridge *bg, char *ble_cmd, int *len);

#endif