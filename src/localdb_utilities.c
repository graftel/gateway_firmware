// Functions to Save data to local SQLite DB
#include <localdb_utilities.h>

char sql_query[500];
char buffer[50];
//int write_values(hx_data *user_hx_data)

int write_sqlite_hx_data(data_set_def *data_set)
{
   // set external lock 
   sqlite3 *db;
   char *err_msg = 0;
   int rc;

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
      fprintf(stderr, "local db: Can't open database: %s\n", sqlite3_errmsg(db));
      return 1;
   }

	   memset(sql_query,0,sizeof(sql_query));
	   strcpy (sql_query, "CREATE TABLE if not exists ");
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


     memset(sql_query,0,sizeof(sql_query));
     strcpy (sql_query, "INSERT INTO ");
     strcat (sql_query, HX_MAIN_TABLE_NAME);
     strcat (sql_query, " (data_time_stamp,deviceID,data,data_quality,sync_status) VALUES(");

     // Time stamp
     memset(buffer,0,sizeof(buffer));
     sprintf(buffer,"%d",data_set->timestamp);
     strcat (sql_query, buffer);
     strcat (sql_query, ",'");
     // Device ID
     strcat (sql_query, data_set->id);
     strcat (sql_query, "',");
     // Data
     memset(buffer,0,sizeof(buffer));
     sprintf(buffer,"%f",data_set->data);
     strcat (sql_query, buffer);
     strcat (sql_query, ",");

     // Data code && sync status
     memset(buffer,0,sizeof(buffer));
     sprintf(buffer,"%d",(int)data_set->data_code);
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

  sqlite3_free(err_msg);
  sqlite3_close(db);

  return 0;
}
