#ifndef _TEMP_PROBE_RS485_TSYS01_H
#define _TEMP_PROBE_RS485_TSYS01_H


#include <defines.h>
#define UART_I2C_PROBE_CMD_LENGTH 15

int get_ble_cmd_temp_probe_RS485_tsys01(sensor *sen, char *ble_cmd, int *len);
int get_uart_cmd_temp_probe_RS485_tsys01(sensor *sen, char *cmd, int *len, char *cm_addr);

data_code_def process_ble_resp_temp_probe_RS485_tsys01(sensor *sen, char *ble_resp, int len);
data_code_def process_uart_resp_temp_probe_RS485_tsys01(sensor *sen, char *uart_resp, int len);

#endif
