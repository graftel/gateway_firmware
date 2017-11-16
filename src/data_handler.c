
#include <data_handler.h>
#include <iot_handler.h>

bridge *bridge_data;

void *db_data_handler(void *args)
{
		bridge_data = (bridge *)args;
		data_set_def *data_set;
	//	int ret = 0;
	//	int no_internet_count = 0;
		while(1)
		{

				data_set = g_async_queue_pop(bridge_data->data_queue);
				if (data_set != NULL)
				{

					printf("pop data: %d , ", data_set->timestamp);
					printf("%s , ", data_set->id);
					printf("%f , ", data_set->data);
					printf("%d\n", data_set->data_code);

					if (write_sqlite_hx_data(data_set) != 0)
					{
						fprintf(stderr, "write to local db error\n");
					}

					if (publish_data_to_iot_hub(data_set) != 0)
					{
						fprintf(stderr, "publish to iot hub error\n");
					}

          /*
					if (no_internet_count == 0)
					{
							ret = sync_hx_data();
							if (ret != 0)
							{
								no_internet_count++;
							}
					}
					else
					{
						no_internet_count++;

						if (no_internet_count > NO_INTERNET_CHECK_FREQUENCY)
						{
							no_internet_count = 0;
						}
					}*/
				}
				g_free(data_set);
		}


}
