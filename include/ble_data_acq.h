#ifndef _BLE_DATA_ACQ_H
#define _BLE_DATA_ACQ_H
#include <defines.h>
#define ATT_CID					4
#define ATT_PSM					31
#define READ_HANDLE				0x0D
#define WRITE_HANDLE			0x10

#define RETRY_TIMES 5
#define FAST_RETRY_TIMES 100
#define BLE_TIME_OUT	10
#define ERROR_DATA_VALUE 999.0

int ble_data_acq(core_module *cm);

#endif
