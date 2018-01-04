#ifndef _TEMP_PROBE_12CH_10K_H
#define _TEMP_PROBE_12CH_10K_H

#include <defines.h>
#define UART_12_CH_CMD_LENGTH 16


int get_ble_cmd_12_ch(sensor *sen, char *ble_cmd, int *len);
int get_uart_cmd_12_ch(sensor *sen, char *cmd, int *len, char *cm_addr);

data_code_def process_ble_resp_12_ch(sensor *sen, char *ble_resp, int len);
data_code_def process_uart_resp_12_ch(sensor *sen, char *uart_resp, int len);

#endif
