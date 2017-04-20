// DAQ module
#include <i2c_tsys01.h>
#include <string.h>
#include <stdio.h>
#include <defines.h>
#include <stdlib.h>
#include <json/json.h>

void DAQ_GetData(bulk_sensor_data *data)
{
 	// Read Configuration file



}


void DAQ_ReadConfig()
{

}


void DAQ_ScanSensors_Upadte_SensorDef()
{
    uint8_t i2c_addr = 0x00;

    TSYS01_Sensor sensor;
    hx_sensor *sensors;
    int num_hx_sensors = 0;
    char tmp[100];
	
	json_object *jobj_root = json_object_new_object();
	
	json_object 
	json_object *jdouble = json_object_new_double(1.0);
	json_object_object_add(jobj,"Version", jdouble);
	json_object *jstring = json_object_new_string("Sensors");
	
    for (i2c_addr = 0x00; i2c_addr <= 0x80; i2c_addr++)
    {
	//	printf("checking addr=0x%02x\n", i2c_addr);
        if (TSYS01_init(&sensor, i2c_addr) == 0)
        {
          sensors = malloc(sizeof(hx_sensor) * num_hx_sensors + 1);
          printf("addr=0x%02x\n", i2c_addr);
		  
		  strcpy(sensors[num_hx_sensors].dev_data_type,"Temperature");
		  
		  memset(tmp,0,sizeof(tmp));
		  sprintf(tmp,"0x%02x",i2c_addr);
		  strcpy(sensors[num_hx_sensors].dev_address,tmp);
		  
		  strcpy(sensors[num_hx_sensors].dev_protocol,"I2C");
		 
		  num_hx_sensors++;
        }
    }

}
