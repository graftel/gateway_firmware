#ifndef _SQLFUNC_H
#define _SQLFUNC_H

#define HX_SQLITE_FILE_NAME "sensor_data.db"
#define HX_MAIN_TABLE_NAME "RAW_DATA"
#include <defines.h>

int write_sqlite_sensor_data(sensor_data *data, int size_data);
void *write_sqlite_sensor_data_wrapper(void *args);


#endif