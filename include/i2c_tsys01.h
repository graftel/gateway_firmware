//i2c_tsys01.h
#ifndef _I2C_TSYS01_H
#define _I2C_TSYS01_H
#include <stdint.h>

typedef struct {
	uint16_t coefficent[5];
	float temp_reading;
	uint8_t i2c_addr;
	int init_status;
} TSYS01_Sensor;

int TSYS01_init(TSYS01_Sensor *sensor, uint8_t addr);
int TSYS01_GetTemp(TSYS01_Sensor *sensor);

#endif