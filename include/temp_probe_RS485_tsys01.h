#ifndef _TEMP_PROBE_RS485_TSYS01_H
#define _TEMP_PROBE_RS485_TSYS01_H

#include <defines.h>

int get_ble_cmd_temp_probe_RS485_tsys01(sensor *sen, char *ble_cmd, int *len);

data_code_def process_ble_resp_temp_probe_RS485_tsys01(sensor *sen, char *ble_resp, int len);

#endif
