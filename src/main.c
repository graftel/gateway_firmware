// Main Program

#include <i2c_tsys01.h>
#include <string.h>
#include <stdio.h>
#include <sqlfunc.h>
#include <defines.h>
#include <stdlib.h>
#include <i2c_rtc.h>
#include <alarm_trigger.h>
#include <signal.h>

struct tm *tm_info;
time_t cur_read_time;
int rc1 = 0;
pthread_t thread1;

void sigusr2(int signo, siginfo_t *info, void *extra) 
{
       void *ptr_val = info->si_value.sival_ptr;
       int int_val = info->si_value.sival_int;
       sensor_data *raw_data;
    	int size_sensor_data = 0;
		int rc1;
		
		DAQ_GetData(&raw_data, &size_sensor_data,int_val);
		
		int i = 0;
		printf("check12\n");
		printf("main:data.size_data=%d\n",size_sensor_data);
		for (i = 0; i < size_sensor_data;i++)
		{
			printf("addr=%s ",raw_data[i].dev_address);
			printf("data=%f\n",raw_data[i].data);
		}
		
		
		bulk_sensor_data sensor_bulk_data;
		sensor_bulk_data.data = raw_data;
		sensor_bulk_data.size_data = size_sensor_data;
		
		if( (rc1 = pthread_create( &thread1, NULL, &write_sqlite_sensor_data_wrapper, (void *)&sensor_bulk_data)))
		{
			fprintf(stderr,"Thread creation failed: %d\n", rc1);
		}
		
		printf("rc1=%d\n",rc1);
		
		if (rc1 == 0)
		{
			rc1 = pthread_detach(thread1);
		}
		
		
		free(raw_data);
}

int main()
{
	struct sigaction action;

	action.sa_flags = SA_SIGINFO; 
	action.sa_sigaction = &sigusr2;

	if (sigaction(SIGUSR2, &action, NULL) == -1) { 
		perror("sigusr: sigaction");
		return 1;
	}	
	//DAQ_ScanSensors_Upadte_SensorDef();
	alarm_trigger_thread_args alarm_trigger_args;
	alarm_trigger_args.interval = 10;
	
	
	if( (rc1 = pthread_create( &thread1, NULL, &start_alarm_trigger_wrapper, (void *)&alarm_trigger_args)))
	{
		fprintf(stderr,"Thread creation failed: %d\n", rc1);
	}

	if (rc1 == 0)
	{
		rc1 = pthread_detach(thread1);
	}
	
	while(1)
	{
		// int epoch;
		// MCP79410_Read_Epoch_Time(&epoch);
		// printf("EPOCH TIME:%d\n",epoch);
		sleep(1);
	}
	
	// while(1)
	// {
		// sensor_data *raw_data;
		// int size_sensor_data = 0;
		// int rc1;
		
		// DAQ_GetData(&raw_data, &size_sensor_data);
		
		// int i = 0;
		// printf("check12\n");
		// printf("main:data.size_data=%d\n",size_sensor_data);
		// for (i = 0; i < size_sensor_data;i++)
		// {
			// printf("addr=%s ",raw_data[i].dev_address);
			// printf("data=%f\n",raw_data[i].data);
		// }
		
		
		// bulk_sensor_data sensor_bulk_data;
		// sensor_bulk_data.data = raw_data;
		// sensor_bulk_data.size_data = size_sensor_data;
		
		// if( (rc1 = pthread_create( &thread1, NULL, &write_sqlite_sensor_data_wrapper, (void *)&sensor_bulk_data)))
		// {
			// fprintf(stderr,"Thread creation failed: %d\n", rc1);
		// }
		
		// printf("rc1=%d\n",rc1);
		
		// if (rc1 == 0)
		// {
			// rc1 = pthread_detach(thread1);
		// }
		
		
		// free(raw_data);
		// sleep(10);
	// }
	
		// if( (rc1 = pthread_create( &thread1, NULL, &write_sqlite_sensor_data_wrapper, (void *)&data)))
		// {
			// fprintf(stderr,"Thread creation failed: %d\n", rc1);
		// }
		
		// printf("rc1=%d\n",rc1);
		
		// if (rc1 == 0)
		// {
			// rc1 = pthread_detach(thread1);
		// }
		
//		printf("Temp1=%f\n",s1.temp_reading);
		


	// int status;
	// TSYS01_Sensor s1,s2,s3,s4;
	
	// status = TSYS01_init(&s1, 0x66);
	// status = TSYS01_init(&s2, 0x67);
	// status = TSYS01_init(&s3, 0x78);
	// status = TSYS01_init(&s4, 0x79);
	// if (status != 0)
	// {
		// printf("init error\n");
		// return -1;
	// }
		
	// while(1)
	// {
		// status = TSYS01_GetTemp(&s1);
		// status = TSYS01_GetTemp(&s2);
		// status = TSYS01_GetTemp(&s3);
		// status = TSYS01_GetTemp(&s4);
		
		// printf("temp1=%f\n",s1.temp_reading);
		// printf("temp1=%f\n",s2.temp_reading);
		// printf("temp1=%f\n",s3.temp_reading);
		// printf("temp1=%f\n",s4.temp_reading);
		
		// // if (status != 0)
		// // {
			// // printf("read error\n");
		// // }
		// // tm_info = localtime(&cur_read_time);
		
		// // sensor_data s_data1;
		
		// // s_data1.time_stamp = cur_read_time;
		// // s_data1.data = s1.temp_reading;
		// // strcpy(s_data1.data_tag,"I2C:Temp");
		// // strcpy(s_data1.dev_address,"0x66");
		// // strcpy(s_data1.data_unit,"C");
		// // strcpy(s_data1.data_type,"Temperature");
		// // strcpy(s_data1.serial_num,"123456");
		// // strcpy(s_data1.ble_address,"11:22:33:44:55:66");
		// // strcpy(s_data1.protocol,"I2C");
		// // strcpy(s_data1.note,"Special Note");
		
		// // bulk_sensor_data sensor_bulk_data;
		// // sensor_bulk_data.data = &s_data1;
		// // sensor_bulk_data.size_data = 1;
		
		// // if( (rc1 = pthread_create( &thread1, NULL, &write_sqlite_sensor_data_wrapper, (void *)&sensor_bulk_data)))
		// // {
			// // fprintf(stderr,"Thread creation failed: %d\n", rc1);
		// // }
		
		// // printf("rc1=%d\n",rc1);
		
		// // if (rc1 == 0)
		// // {
			// // rc1 = pthread_detach(thread1);
		// // }
		
// //		printf("Temp1=%f\n",s1.temp_reading);
		
		// sleep(10);
	// }
	
	return 0;
	
}