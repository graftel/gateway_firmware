#ifndef _LOCALDB_UTILITIES_H
#define _LOCALDB_UTILITIES_H

#include <defines.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define HX_SQLITE_DATA_FOLDER_NAME "/var/lib/hxmonitor"
#define HX_SQLITE_FILE_NAME "hx_user_data.db"
#define HX_MAIN_TABLE_NAME "HX_RAW_DATA"

int write_sqlite_hx_data(bridge *user_data);
void *write_sqlite_hx_data_wrapper(void *data);

#endif
