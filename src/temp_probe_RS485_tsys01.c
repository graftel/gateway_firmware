#include <string.h>
#include <stdio.h>
#include <temp_probe_RS485_tsys01.h>
#include <utilities.h>



uint8_t api_id = 0;

int get_ble_cmd_temp_probe_RS485_tsys01(sensor *sen, char *ble_cmd, int *len)
{
	// First Get temp Connected to the core module, get cmd based on proto
	(*len) = READ_TEMP_LEN;
	ble_cmd[BLE_DATA_CH_POS] = BLE_TSYS01_HEADER;
	ble_cmd[BLE_DATA_LEN_POS] = READ_TEMP_LEN - 2;
	ble_cmd[BLE_DATA_API_POS] = api_id;
	ble_cmd[BLE_DATA_CMD_POS] = READ_TEMP_HEX;
	memcpy(ble_cmd + BLE_DATA_ADDR_POS, sen->addr, ADDR_LEN_I2C_PROBE);
	ble_cmd[READ_TEMP_LEN - 1] = cal_checksum((uint8_t*)ble_cmd, READ_TEMP_LEN - 1);

	return 0;
}

int get_uart_cmd_temp_probe_RS485_tsys01(sensor *sen, char *cmd, int *len, char *cm_addr)
{
	int pos = 0;
	(*len) = UART_I2C_PROBE_CMD_LENGTH;
	cmd[pos] = '*';
	pos++;
	memcpy(cmd + pos, cm_addr, ADDR_LEM_CORE_MODULE);
	pos += ADDR_LEM_CORE_MODULE;
	cmd[pos] = 'A';
	pos++;
	memcpy(cmd + pos, sen->addr, ADDR_LEN_I2C_PROBE);
	pos += ADDR_LEN_I2C_PROBE;
	cmd[pos] = 0x0d;

	return 0;
}

data_code_def process_ble_resp_temp_probe_RS485_tsys01(sensor *sen, char *ble_resp, int len)
{
    float temp_value;

    if (ble_resp[BLE_DATA_CMD_POS] == 0x41 && ble_resp[BLE_DATA_API_POS] == api_id)
    {
        DEBUG_PRINT("read ok\n");
        memcpy(&temp_value, ble_resp + BLE_DATA_ADDR_POS + ADDR_LEN_I2C_PROBE, sizeof(float));
        DEBUG_PRINT3("temp_reading=%f\n",temp_value);

        sen->data = temp_value;
    }
    else{
          DEBUG_PRINT("read error, no response read sensor in RS485\n");
          return SENSOR_TSYS01_DATA_PARSE_ERR;
    }

		api_id++;

		if (api_id >= 0xff)
		{
			api_id = 0;
		}

    return OK;
}

data_code_def process_uart_resp_temp_probe_RS485_tsys01(sensor *sen, char *uart_resp, int len)
{
    float temp_value;
		char float_tmp[MAX_FLOAT_VALUE_LIMIT];
		char *chr_pos = strchr((char*)uart_resp,'=');
		int equal_pos = 0;

		if (chr_pos == NULL)
		{
			DEBUG_PRINT("resp error\n");
			return PROTOCOL_UART_PARSE_ERR;
		}
		else{
			equal_pos = (int)(chr_pos - (char *)uart_resp);

			if (strncmp(uart_resp + equal_pos + 1, ERROR_CODE_STR, strlen(ERROR_CODE_STR)) == 0)
			{
				DEBUG_PRINT("Error happened in core module\n");
				memset(float_tmp,0,sizeof(float_tmp));
				memcpy(float_tmp,uart_resp + equal_pos + 1 + strlen(ERROR_CODE_STR), 1);
				int error_code = atoi(float_tmp);
				DEBUG_PRINT("error_code=%d\n", error_code);

				return SENSOR_TSYS01_START_ERR + error_code;
			}
			else
			{
				memset(float_tmp,0,sizeof(float_tmp));
				memcpy(float_tmp,uart_resp + equal_pos + 1, len - equal_pos - 1);
				temp_value = atof(float_tmp);
				DEBUG_PRINT("temp_reading=%f\n",temp_value);

				sen->data = temp_value;
			}
		}

    return OK;
}
