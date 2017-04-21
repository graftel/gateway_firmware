// DAQ module
#include <i2c_tsys01.h>
#include <string.h>
#include <stdio.h>
#include <defines.h>
#include <stdlib.h>
#include <json/json.h>

const char *SENSOR_DEF_FILE_NAME = "SensorDefinition.json";


int DAQ_GetData(sensor_data **raw_data, int *size_data, int rtc_time_stamp)  // read one value from all available sensors
{
 	// Read Configuration file
	FILE *fp;
	char *buffer;
	long lSize;
	size_t result;
	printf("check0\n");
	fp = fopen(SENSOR_DEF_FILE_NAME,"r");
	if (fp == NULL)
	{
		perror ("File Open Error, not exisit");
		return -1;
	}
	
	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);
	printf("check1\n");
	buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) 
	{
		perror("memory error");
		return -1;
	}
	
	result = fread(buffer,1,lSize,fp);
	if (result != lSize)
	{
		perror("Reading error");
		return -1;
	}
	printf("check2\n");
	fclose(fp);
	
//	printf("string=%s\n",buffer);
	
	json_object * jobj = json_tokener_parse(buffer);
	json_object *jsensor_array;
	json_object *jsensor;
	
	// enum json_type type;
	// json_object_object_foreach(jobj, key, val) {
		// type = json_object_get_type(val);
		// switch (type)
		// {
			// case json_type_array:
				// if (strcmp(key,"Sensors") == 0 )
				// {
					
					// printf("value: %s\n", json_object_get_string(val));
				// }
				// break;
		// }
	// }
	
	json_object_object_get_ex(jobj, "Sensors", &jsensor_array);
	printf("check3\n");
	int i = 0;
	*size_data = 0;
	
	*raw_data = malloc(sizeof(sensor_data) * ((*size_data) + 1));
	
	printf("check1\n");
	for (i = 0; i < json_object_array_length(jsensor_array); i++)
	{
		jsensor = json_object_array_get_idx(jsensor_array, i);
		
		char device_tag[20];
		char device_addr[10];
		char device_calinfo[50];
		
		json_object_object_foreach(jsensor, key, val)
		{
			if (strcmp(key,"Data Tag") == 0 )
			{
				strcpy(device_tag, json_object_get_string(val));
			}
			
			if (strcmp(key,"Device Address") == 0 )
			{
				strcpy(device_addr, json_object_get_string(val));
			}
			
			
			if (strcmp(key,"Calibration Info") == 0 )
			{
				strcpy(device_calinfo, json_object_get_string(val));
			}
		}
		
		if (strcmp(device_tag,"I2C:Temperature") == 0) // read i2c data
		{
			
			*raw_data = realloc(*raw_data, sizeof(sensor_data) * ((*size_data) + 1));
			
			//printf("sensor_def_addr%d,%s\n",i,device_addr);
			//printf("sensor_def_cal_info%d,%s\n",i,device_calinfo);
			float test_float;
			TSYS01_GetTemp_WithCal(device_addr,device_calinfo, &test_float);
			//printf("sensor_def_data%d,%f\n",i,test_float);
			(*raw_data)[*size_data].data = test_float;
			(*raw_data)[*size_data].time_stamp = rtc_time_stamp;
			
			strcpy((*raw_data)[*size_data].data_tag,device_tag);
			strcpy((*raw_data)[*size_data].dev_address,device_addr);
			strcpy((*raw_data)[*size_data].data_unit,"C");
			strcpy((*raw_data)[*size_data].serial_num,"123456");
			strcpy((*raw_data)[*size_data].ble_address,"11:22:33:44:55:66");
			strcpy((*raw_data)[*size_data].note,"Special Note");
			
			(*size_data)++;
		}
	}
	
	
	printf("data_size=%d\n",*size_data);
	
//	free(sensors_def);
	free(buffer);

//	json_object_put(jsensor);
//	json_object_put(jsensor_array);
	json_object_put(jobj);
	
	return 0;
}


void DAQ_ScanSensors_Upadte_SensorDef()
{
    uint8_t i2c_addr = 0x00;
	uint8_t n = 0;
    TSYS01_Sensor sensor;
    char tmp[100];
	char tmpsh[50];
	
	json_object *jobj_root = json_object_new_object();
	json_object *jstring = json_object_new_string("1.0");
	json_object_object_add(jobj_root,"Version", jstring);
	json_object *jarray = json_object_new_array();
	
    for (i2c_addr = 0x00; i2c_addr <= 0x80; i2c_addr++)
    {
	//	printf("checking addr=0x%02x\n", i2c_addr);
        if (TSYS01_init(&sensor, i2c_addr) == 0)
        {
          
         printf("addr=0x%02x\n", i2c_addr);
		  
		 memset(tmp,0,sizeof(tmp));
		 sprintf(tmp,"0x%02x",i2c_addr);
		  
		 
		 json_object *jobj_sensor = json_object_new_object();
		 
		 //String Value definition
		 json_object *jstring_addr = json_object_new_string(tmp);
		 json_object *jstring_data_tag = json_object_new_string("I2C:Temperature");
		 
		 memset(tmp,0,sizeof(tmp));
		 
		 for (n = 0; n < 5; n++)
		 {
			 memset(tmpsh,0,sizeof(tmpsh));
			 sprintf(tmpsh,"%04X",sensor.coefficent[n]);
			 strcat(tmp,tmpsh);
			 strcat(tmp," ");
		 }
		 
		 json_object *jstring_calinfo = json_object_new_string(tmp);
		 
		 //String Key definition
		 json_object_object_add(jobj_sensor,"Device Address",jstring_addr);
		 json_object_object_add(jobj_sensor,"Data Tag",jstring_data_tag);
		 json_object_object_add(jobj_sensor,"Calibration Info",jstring_calinfo);
		 
		 
		 json_object_array_add(jarray, jobj_sensor);
		 
        }
    }
	
	json_object_object_add(jobj_root,"Sensors", jarray);
	
	FILE *fp;
	
	fp = fopen(SENSOR_DEF_FILE_NAME,"w");
	fprintf(fp, json_object_to_json_string(jobj_root));
	
	fclose(fp);
//	printf("%s\n", json_object_to_json_string(jobj_root));

}
