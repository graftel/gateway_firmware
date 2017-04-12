// Functions to Save data to local SQLite DB
#include <bridge.h>
#include <sqlfunc.h>
#include <sqlite3.h>

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
		
 //       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    
    return 0;
}

void *write_sqlite_hx_data_wrapper(void *args)
{
	thread_arg *arguments = args;
	
	return (void *)write_sqlite_hx_data(arguments->user_data,arguments->size_hx_data);
}

int write_sqlite_hx_data(hx_data *user_data, int size_hx_data)
{
   sqlite3 *db;
   char *err_msg = 0;
   int rc,i;
   
   rc = sqlite3_open(HX_SQLITE_FILE_NAME, &db);

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
					   ,data 			real  	   not null\
					   ,data_unit 		text\
					   ,data_type 		text\
					   ,data_address 	text\
					   ,serial_num 		text\
					   ,ble_address 	text);");
					   
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
   
   for (i = 0; i < size_hx_data; i++)
   {
	   memset(sql_query,0,sizeof(sql_query));
	   strcpy (sql_query, "INSERT INTO ");
	   strcat (sql_query, HX_MAIN_TABLE_NAME);
	   strcat (sql_query, " (data_time_stamp,data,data_unit,data_type,data_address,serial_num,ble_address) VALUES("); 
	   
	   // Time stamp
	   memset(buffer,0,sizeof(buffer));
	   sprintf(buffer,"%d",(long)user_data[i].time_stamp);
	   strcat (sql_query, buffer);
	   strcat (sql_query, ",");
	   // Data
	   memset(buffer,0,sizeof(buffer));
	   sprintf(buffer,"%.15f",user_data[i].data);
	   strcat (sql_query, buffer);
	   strcat (sql_query, ",'");
	   // Data unit
	   strcat (sql_query, user_data[i].data_unit);
	   strcat (sql_query, "','");
	   // Data type
	   strcat (sql_query, user_data[i].data_type);
	   strcat (sql_query, "','");
	   // Data address
	   memset(buffer,0,sizeof(buffer));
	   sprintf(buffer,"%02X",user_data[i].data_address);
	   strcat (sql_query, buffer);
	   strcat (sql_query, "','");
	   // serial_num
	   strcat (sql_query, user_data[i].serial_num);
	   strcat (sql_query, "','");
	   // ble_address
	   strcat (sql_query, user_data[i].ble_address);
	   strcat (sql_query, "');");
	   
	 //  printf("insert query:%s\n",sql_query);
	   
	   rc = sqlite3_exec(db, sql_query, 0, 0, &err_msg);
			
	   if (rc != SQLITE_OK) {
		   fprintf(stderr, "Failed to insert value\n");
		   fprintf(stderr, "SQL error: %s\n", err_msg);

		   sqlite3_free(err_msg);
		   sqlite3_close(db);
			
		   return 1;
		}
    }
	
	printf("*********write to SQL finish************\n");
	
	free(user_data);
	
   sqlite3_close(db);
   
   return 0;
}