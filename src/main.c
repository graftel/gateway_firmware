
#include <defines.h>
#include <utilities.h>
#include <ble_data_acq.h>
#include <dynamodb_utilities.h>
#include <localdb_utilities.h>

int start_time,current_time; // test
int interval = 10; // seconds
int status = 0;
int i = 0;
static bridge bridge_data;
//static int running_cycle = 0;
GThread *thread_localdb, *sync_db;

int rc;
GError  *err1 = NULL;
GError  *err2 = NULL;
gboolean timeout_callback(gpointer data)
{

		current_time = time(NULL);
   //MCP79410_Read_Epoch_Time(&current_time);

    if ((current_time - start_time) % interval == 0)
    {
				//start collecting data based on config file
				bridge_data.current_timestamp = current_time;
				for (i = 0; i < bridge_data.size_cm; i++)
				{
					core_module *cur_cm = &bridge_data.cm[i];

					if (cur_cm->protocol == CM_BLE)
					{
            ble_data_acq(cur_cm);
					}
					else if (cur_cm->protocol == CM_RS485)
					{

					}

				}
				thread_localdb = g_thread_try_new("LOCAL_DB_TH", (GThreadFunc)write_sqlite_hx_data_wrapper, (gpointer)&bridge_data, &err1);
				// save all data to local db thread
				if (err1 != NULL)
				{
					g_printerr("%s!\n",err1->message);
				}
/*
		if (check_internet() != 0)
		{
			g_print("time(%d):no internet, waiting\n",time(NULL));	// only perform local DAQ
		}
		else
		{
			if (running_cycle == 0)
			{
				get_def_from_cloud(&bridge_data);
				running_cycle++;
			}

			if (bridge_data.size_cm != 0)
			{
				g_print("time(%d):get def from cloud, acquire ble data\n",time(NULL));
				ble_data_acq(&bridge_data);

				g_print("time(%d):SEND DATA TO CLOUD THREAD\n",time(NULL));
				// SEND DATA TO CLOUD THREAD

				bridge_data.current_timestamp = current_time;

				thread_send_data = g_thread_try_new("SEND_TO_CLOUD_TH", (GThreadFunc)send_data_to_cloud_wrapper, (gpointer)&bridge_data, &err2);

				if (err2 != NULL)
				{
					g_printerr("%s!\n",err2->message);
				}

				g_thread_join(thread_send_data);

			}



		}
		*/

    }

    return TRUE;
}

int main()
{
/* 	int curSec = 00;    // 0-59
	int curMin = 29;    // 0-59
	int curHour = 12;    // 0-23 in 24-hour format
	bool is12 = false;   // 12 = true, 24 = false
	int curDay = 16;    // 1-31
	int curMonth = 6;   // 1-12
	int curYear = 2017; // 2000 - 2099
	int curDayofweek = 5;  // 1-7 Monday = 1, Tuesday = 2.....

	MCP79410_Setup_Date(curSec,curMin,curHour,curDayofweek,is12,curDay,curMonth,curYear); */
    GMainLoop *loop;
	if (load_config(&bridge_data) == 1)
	{
		g_print("config load failed\n");
	}
	// Start Getting Sensor Definitions
	//get_sensor_defs(&bridge_data);

	start_time = time(NULL);

//	MCP79410_Read_Epoch_Time(&start_time);

	DEBUG_PRINT("start_time=%d\n",start_time);
	sync_db = g_thread_try_new("SYNC_DB_TH", (GThreadFunc)sync_data_with_cloud_wrapper, (gpointer)&bridge_data, &err1);

  loop = g_main_loop_new ( NULL , FALSE );
  // add source to default context
// add interface source

  g_timeout_add (1000 , timeout_callback , loop);
  g_main_loop_run (loop);
  g_main_loop_unref(loop);


	g_print("free everything");
	free_defs(&bridge_data);
  return 0;
}
