//i2c_tsys01.h
#ifndef _I2C_TSYS01_H
#define _I2C_TSYS01_H

typedef struct {
	uint16_t coefficent[5];
	float temp_reading;
	uint8_t i2c_addr;
} TSYS01_Sensor;

void TSYS01INIT();
void GetCoefs();
void TSYS01GetTemp();
void GetReading();

#endif