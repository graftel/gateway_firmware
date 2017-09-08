#include <string.h>
#include <stdio.h>
#include <temp_probe_RS485_tsys01.h>
#include <utilities.h>

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

int get_ble_cmd_temp_probe_RS485_tsys01(sensor *sen, char *ble_cmd, int *len)
{
	// First Get temp Connected to the core module, get cmd based on proto
	(*len) = READ_TEMP_LEN;
	ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
	ble_cmd[BLE_DATA_LEN_POS] = READ_TEMP_LEN - 2;
	ble_cmd[BLE_DATA_CMD_POS] = READ_TEMP_HEX;
	memcpy(ble_cmd + BLE_DATA_ADDR_POS, sen->addr, MAX_RS485_ADDR_LEN);
	ble_cmd[READ_TEMP_LEN - 1] = cal_checksum((uint8_t*)ble_cmd, READ_TEMP_LEN - 1);

	return 0;
}

data_code_def process_ble_resp_temp_probe_RS485_tsys01(sensor *sen, char *ble_resp, int len)
{
    float temp_value;

    if (ble_resp[2] == 0x41)
    {
        DEBUG_PRINT("read ok\n");
        memcpy(&temp_value, ble_resp + 9, sizeof(float));
        DEBUG_PRINT("temp_reading=%f\n",temp_value);

        sen->data = temp_value;
    }
    else{
          DEBUG_PRINT("read error, no response read sensor in RS485\n");
          return RS485_TSYS01_NO_RESPONSE;
    }

    return OK;
}
