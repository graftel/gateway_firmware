#ifndef _SQLFUNC_H
#define _SQLFUNC_H

#define HX_SQLITE_FILE_NAME "hx_user_data.db"
#define HX_MAIN_TABLE_NAME "HX_RAW_DATA"
#include <bridge.h>

int write_sqlite_hx_data(hx_data *user_data, int size_hx_data);
void *write_sqlite_hx_data_wrapper(void *args);


#endif