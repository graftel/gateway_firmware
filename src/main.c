// Main Program

#include <i2c_tsys01.h>
#include <string.h>
#include <stdio.h>
#include <sqlfunc.h>
#include <defines.h>

struct tm *tm_info;
time_t cur_read_time;
int rc1 = 0;
pthread_t thread1;

int main()
{
	int status;
	TSYS01_Sensor s1,s2,s3,s4;
	
	status = TSYS01_init(&s1, 0x66);
	
	if (status != 0)
	{
		printf("init error\n");
		return -1;
	}
		
	while(1)
	{
		status = TSYS01_GetTemp(&s1);
		
		if (status != 0)
		{
			printf("read error\n");
		}
		tm_info = localtime(&cur_read_time);
		
		sensor_data s_data1;
		
		s_data1.time_stamp = cur_read_time;
		s_data1.data = s1.temp_reading;
		strcpy(s_data1.data_tag,"I2C:Temp");
		strcpy(s_data1.dev_address,"0x66");
		strcpy(s_data1.data_unit,"C");
		strcpy(s_data1.data_type,"Temperature");
		strcpy(s_data1.serial_num,"123456");
		strcpy(s_data1.ble_address,"11:22:33:44:55:66");
		strcpy(s_data1.protocol,"I2C");
		strcpy(s_data1.note,"Special Note");
		
		bulk_data sensor_bulk_data;
		sensor_bulk_data.data = &s_data1;
		sensor_bulk_data.size_data = 1;
		
		if( (rc1 = pthread_create( &thread1, NULL, &write_sqlite_sensor_data_wrapper, (void *)&sensor_bulk_data)))
		{
			fprintf(stderr,"Thread creation failed: %d\n", rc1);
		}
		
		if (rc1 == 0)
		{
			rc1 = pthread_detach(thread1);
		}
		
//		printf("Temp1=%f\n",s1.temp_reading);
		
		sleep(10);
	}
	
	return 0;
	
}