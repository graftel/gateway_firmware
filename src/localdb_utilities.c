// Functions to Save data to local SQLite DB
#include <localdb_utilities.h>

char sql_query[500];
char buffer[50];
int main_table_exist = 0;
//int write_values(hx_data *user_hx_data)
int callback_table_exist(void *NotUsed, int argc, char **argv,
                    char **azColName) {

    NotUsed = 0;
    int i;
    for (i = 0; i < argc; i++) {

    		if (strcmp(argv[i],"1") == 0)
    		{
    			main_table_exist = 1;
    			return 0;
    		}
    }

    return 0;
}

void *write_sqlite_hx_data_wrapper(void *data)
{
  bridge *user_data = (bridge *)data;

  return (void *)write_sqlite_hx_data(user_data);
}

int write_sqlite_hx_data(bridge *user_data)
{
   sqlite3 *db;
   char *err_msg = 0;
   int rc,i,j;

   struct stat st = {0};
   char db_path[100];

  strcpy(db_path, HX_SQLITE_DATA_FOLDER_NAME);
  strcat(db_path, "/");
  strcat(db_path, HX_SQLITE_FILE_NAME);

  if (stat(HX_SQLITE_DATA_FOLDER_NAME, &st) == -1) {
      mkdir(HX_SQLITE_DATA_FOLDER_NAME, 0755);
  }

   rc = sqlite3_open(db_path, &db);

   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return 1;
   }

   memset(sql_query,0,sizeof(sql_query));

   strcpy (sql_query, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='");
   strcat (sql_query, HX_MAIN_TABLE_NAME);
   strcat (sql_query, "';");

//   printf("sql query=%s\n",sql_query);

   rc = sqlite3_exec(db, sql_query, callback_table_exist, 0, &err_msg);

   if (rc != SQLITE_OK) {
	    fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);

        return 1;
   }

   if (main_table_exist == 0){
	   main_table_exist = 1;
	   memset(sql_query,0,sizeof(sql_query));
	   strcpy (sql_query, "CREATE TABLE ");
	   strcat (sql_query, HX_MAIN_TABLE_NAME);
	   strcat (sql_query, "(\
					    id 				integer    primary key autoincrement\
					   ,time_stamp  	integer(4) not null default (strftime('%s','now'))\
					   ,data_time_stamp integer(4) \
             ,deviceID        text \
					   ,data 			      real  	   not null \
             ,data_quality       integer          \
             ,sync_status        integer);");

		rc = sqlite3_exec(db, sql_query, 0, 0, &err_msg);

//		printf("query =%s\n",sql_query);

	    if (rc != SQLITE_OK) {
			fprintf(stderr, "Failed to create table\n");
			fprintf(stderr, "SQL error: %s\n", err_msg);

			sqlite3_free(err_msg);
			sqlite3_close(db);

			return 1;
		}
   }

   for (i = 0; i < user_data->size_cm; i++)
   {
       for (j = 0; j < user_data->cm[i].size_sen; j++)
       {

           memset(sql_query,0,sizeof(sql_query));
           strcpy (sql_query, "INSERT INTO ");
           strcat (sql_query, HX_MAIN_TABLE_NAME);
           strcat (sql_query, " (data_time_stamp,deviceID,data,data_quality,sync_status) VALUES(");

           // Time stamp
           memset(buffer,0,sizeof(buffer));
           sprintf(buffer,"%d",user_data->current_timestamp);
           strcat (sql_query, buffer);
           strcat (sql_query, ",'");
           // Device ID
           strcat (sql_query, user_data->cm[i].sen[j].addr);
           strcat (sql_query, "',");
           // Data
           memset(buffer,0,sizeof(buffer));
           sprintf(buffer,"%f",user_data->cm[i].sen[j].data);
           strcat (sql_query, buffer);
           strcat (sql_query, ",");

           // Data code && sync status
           memset(buffer,0,sizeof(buffer));
           sprintf(buffer,"%d",(int)user_data->cm[i].sen[j].data_code);
           strcat (sql_query, buffer);
           strcat (sql_query, ",0);");

           DEBUG_PRINT3("insert query:%s\n",sql_query);

           rc = sqlite3_exec(db, sql_query, 0, 0, &err_msg);

           if (rc != SQLITE_OK) {
             fprintf(stderr, "Failed to insert value\n");
             fprintf(stderr, "SQL error: %s\n", err_msg);

             sqlite3_free(err_msg);
             sqlite3_close(db);

             return 1;
          }


       }
   }

	DEBUG_PRINT3("*********write to SQL finish************\n");

  sqlite3_close(db);

  return 0;
}
