#ifndef _DP_SENSOR_H
#define _DP_SENSOR_H

#include <defines.h>

#define UART_DP_SENSOR_CMD_LENGTH 15

int get_uart_cmd_dp_sensor(sensor *sen, char *cmd, int *len, char *cm_addr);
int get_ble_cmd_dp_sensor(sensor *sen, char *ble_cmd, int *len);

data_code_def process_uart_resp_dp_sensor(sensor *sen, char *uart_resp, int len);
data_code_def process_ble_resp_dp_sensor(sensor *sen, char *ble_resp, int len);

#endif
