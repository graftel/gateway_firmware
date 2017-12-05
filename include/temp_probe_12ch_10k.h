#ifndef _TEMP_PROBE_12CH_10K_H
#define _TEMP_PROBE_12CH_10K_H

#include <defines.h>

int get_ble_cmd_12_ch(sensor *sen, char *ble_cmd, int *len);

data_code_def process_ble_resp_12_ch(sensor *sen, char *ble_resp, int len);

#endif
