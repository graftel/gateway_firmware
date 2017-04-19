// DAQ module
#include <i2c_tsys01.h>
#include <string.h>
#include <stdio.h>
#include <defines.h>

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
    for (i2c_addr = 0x00; i2c_addr < 0xff; i2c_addr)
    {
        if (TSYS01_init(&sensor, i2c_addr) == 0)
        {
          sensors = malloc(sizeof(hx_sensor) * num_hx_sensors + 1);
          printf("addr=0x%02x\n", i2c_addr);
          num_tsys01_sensors++;
        }
    }

}
