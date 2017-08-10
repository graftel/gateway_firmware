
#include <core_module_command_util.h>
#include <string.h>
#include <stdio.h>

uint8_t cal_checksum(uint8_t *value, uint8_t len)
{
	uint8_t i = 0;
	int checksum = 0;

	for(i = 0; i < len; i++)
	{
		checksum += value[i];
	}

	checksum &= 0xff;

	return (uint8_t)checksum;
}

int get_ble_cmd(bridge *bg, char *ble_cmd, int *len)
{
	// First Get temp Connected to the core module, get cmd based on proto
	if (bg->cm[bg->index_cm].size_sen != 0)
	{
		switch(bg->cm[bg->index_cm].sen[bg->cm[bg->index_cm].index_sen].proto)
		{
			case RS485:
				(*len) = READ_TEMP_LEN;
				ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
				ble_cmd[BLE_DATA_LEN_POS] = READ_TEMP_LEN - 2;
				ble_cmd[BLE_DATA_CMD_POS] = READ_TEMP_HEX;
				memcpy(ble_cmd + BLE_DATA_ADDR_POS, bg->cm[bg->index_cm].sen[bg->cm[bg->index_cm].index_sen].addr, MAX_RS485_ADDR_LEN);
				ble_cmd[READ_TEMP_LEN - 1] = cal_checksum(ble_cmd, READ_TEMP_LEN - 1);
				return 0;
			case I2C:
				break;
		}
	}
		
	return -1;
}


/*


int get_ble_cmd(bridge *bg, uint8_t *ble_cmd, uint8_t *len)
{
	
}

int get_next_ble_cmd(bridge *bg, uint8_t *ble_cmd, uint8_t *len)
{

	if (bg->cm[index_cm].size_t0 != 0 && bg->cm[index_cm].index_t0 < bg->cm[index_cm].size_t0) // start with RS485 Sensor
	{
		get_ble_temp0_cmd(bg->cm[index_cm].t0[index_t0].cmd_steps, bg->cm[index_cm].addr, ble_cmd, len);
	}
	else if (bg->cm[index_cm].size_t1 != 0 && bg->cm[index_cm].index_t1 < bg->cm[index_cm].size_t1) // read i2c sensor
	{
		
	}
	else
	{
		return 1; //finished
	}
}
*/

/*
int get_ble_temp0_cmd(cmd_step_enum steps, char *addr, uint8_t *ble_cmd, uint8_t *len){
	switch(steps)
	{
		case STEP1:	// READ TEMP
			(*len) = READ_TEMP_LEN;
			ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
			ble_cmd[BLE_DATA_LEN_POS] = READ_TEMP_LEN - 2;
			ble_cmd[BLE_DATA_CMD_POS] = READ_TEMP_HEX;
			memcpy(ble_cmd + BLE_DATA_ADDR_POS, addr, MAX_RS485_ADDR_LEN);
			ble_cmd[READ_TEMP_LEN - 1] = cal_checksum(ble_cmd, READ_TEMP_LEN - 1);
			break;
		case STEP2: // READ COEF0
			(*len) = READ_COEF_LEN;
			ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
			ble_cmd[BLE_DATA_LEN_POS] = READ_COEF_LEN - 2;
			ble_cmd[BLE_DATA_CMD_POS] = READ_COEF_HEX;
			memcpy(ble_cmd + BLE_DATA_ADDR_POS, addr, MAX_RS485_ADDR_LEN);
			ble_cmd[BLE_DATA_ADDR_POS + MAX_RS485_ADDR_LEN] = READ_COEF0 - 1;
			ble_cmd[READ_COEF_LEN - 1] = cal_checksum(ble_cmd, READ_COEF_LEN - 1);
			break;
		case STEP3:  // READ COEF1
			(*len) = READ_COEF_LEN;
			ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
			ble_cmd[BLE_DATA_LEN_POS] = READ_COEF_LEN - 2;
			ble_cmd[BLE_DATA_CMD_POS] = READ_COEF_HEX;
			memcpy(ble_cmd + BLE_DATA_ADDR_POS, addr, MAX_RS485_ADDR_LEN);
			ble_cmd[BLE_DATA_ADDR_POS + MAX_RS485_ADDR_LEN] = READ_COEF1 - 1;
			ble_cmd[READ_COEF_LEN - 1] = cal_checksum(ble_cmd, READ_COEF_LEN - 1);
			break;
		case STEP4:  // READ COEF2
			(*len) = READ_COEF_LEN;
			ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
			ble_cmd[BLE_DATA_LEN_POS] = READ_COEF_LEN - 2;
			ble_cmd[BLE_DATA_CMD_POS] = READ_COEF_HEX;
			memcpy(ble_cmd + BLE_DATA_ADDR_POS, addr, MAX_RS485_ADDR_LEN);
			ble_cmd[BLE_DATA_ADDR_POS + MAX_RS485_ADDR_LEN] = READ_COEF2 - 1;
			ble_cmd[READ_COEF_LEN - 1] = cal_checksum(ble_cmd, READ_COEF_LEN - 1);
			break;
		case STEP5: // READ COEF3
			(*len) = READ_COEF_LEN;
			ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
			ble_cmd[BLE_DATA_LEN_POS] = READ_COEF_LEN - 2;
			ble_cmd[BLE_DATA_CMD_POS] = READ_COEF_HEX;
			memcpy(ble_cmd + BLE_DATA_ADDR_POS, addr, MAX_RS485_ADDR_LEN);
			ble_cmd[BLE_DATA_ADDR_POS + MAX_RS485_ADDR_LEN] = READ_COEF3 - 1;
			ble_cmd[READ_COEF_LEN - 1] = cal_checksum(ble_cmd, READ_COEF_LEN - 1);
			break;
		case STEP6: // READ COEF4
			(*len) = READ_COEF_LEN;
			ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
			ble_cmd[BLE_DATA_LEN_POS] = READ_COEF_LEN - 2;
			ble_cmd[BLE_DATA_CMD_POS] = READ_COEF_HEX;
			memcpy(ble_cmd + BLE_DATA_ADDR_POS, addr, MAX_RS485_ADDR_LEN);
			ble_cmd[BLE_DATA_ADDR_POS + MAX_RS485_ADDR_LEN] = READ_COEF4 - 1;
			ble_cmd[READ_COEF_LEN - 1] = cal_checksum(ble_cmd, READ_COEF_LEN - 1);
			break;
		case STEP7: // READ COEF4
			(*len) = READ_COEF_LEN;
			ble_cmd[BLE_DATA_CH_POS] = RS485_CHANNEL_HEX;
			ble_cmd[BLE_DATA_LEN_POS] = READ_COEF_LEN - 2;
			ble_cmd[BLE_DATA_CMD_POS] = READ_COEF_HEX;
			memcpy(ble_cmd + BLE_DATA_ADDR_POS, addr, MAX_RS485_ADDR_LEN);
			ble_cmd[BLE_DATA_ADDR_POS + MAX_RS485_ADDR_LEN] = READ_COEF4 - 1;
			ble_cmd[READ_COEF_LEN - 1] = cal_checksum(ble_cmd, READ_COEF_LEN - 1);
			break;
	}
	
	return 0;
}

int parse_ble_temp0_cmd(bridge *bg, uint8_t *ble_cmd, uint8_t len){
	switch(bg->cm[index_cm].t0[index_t0].cmd_steps)
	{
		case STEP1:	// READ TEMP
			if (get_ble_temp0_cmd(bg->cm[index_cm].t0[index_t0].getCoef == 1 && 
				get_ble_temp0_cmd(bg->cm[index_cm].t0[index_t0].getCal == 1)
			{
					// Todo: Calculate ble value
			}
			else if (get_ble_temp0_cmd(bg->cm[index_cm].t0[index_t0].getCoef == 0)
			{
				bg->cm[index_cm].t0[index_t0].cmd_steps = STEP2;
			}
			
			break;
		case STEP2:
			break;
		case STEP3:
		
	}
}
*/