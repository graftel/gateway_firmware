#include <string.h>
#include <stdio.h>
#include <temp_probe_12ch_10k.h>
#include <utilities.h>



uint8_t api_id_12_ch = 0;

int get_ble_cmd_12_ch(sensor *sen, char *ble_cmd, int *len)
{
	// First Get temp Connected to the core module, get cmd based on proto
	(*len) = READ_12_CH_TEMP_LEN;
	ble_cmd[BLE_DATA_CH_POS] = RS485_12_CH_CHANNEL_HEX;
	ble_cmd[BLE_DATA_LEN_POS] = READ_12_CH_TEMP_LEN - 2;
	ble_cmd[BLE_DATA_API_POS] = api_id_12_ch;
	ble_cmd[BLE_DATA_CMD_POS] = READ_TEMP_HEX;
	memcpy(ble_cmd + BLE_DATA_ADDR_POS, sen->addr, ADDR_LEN_12_CH);
	ble_cmd[READ_12_CH_TEMP_LEN - 1] = cal_checksum((uint8_t*)ble_cmd, READ_12_CH_TEMP_LEN - 1);

	return 0;
}

data_code_def process_ble_resp_12_ch(sensor *sen, char *ble_resp, int len)
{
    float temp_value;

    if (ble_resp[BLE_DATA_CMD_POS] == 0x41 && ble_resp[BLE_DATA_API_POS] == api_id_12_ch)
    {
        DEBUG_PRINT("read ok\n");
        memcpy(&temp_value, ble_resp + BLE_DATA_ADDR_POS + ADDR_LEN_12_CH, sizeof(float));
        DEBUG_PRINT3("temp_reading=%f\n",temp_value);

        sen->data = temp_value;
    }
    else{
          DEBUG_PRINT("read error, no response read sensor in RS485\n");
          return RS485_TSYS01_NO_RESPONSE;
    }

		api_id_12_ch++;

		if (api_id_12_ch >= 0xff)
		{
			api_id_12_ch = 0;
		}

    return OK;
}
