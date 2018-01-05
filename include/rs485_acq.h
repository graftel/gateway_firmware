#ifndef _RS485_ACQ_H
#define _RS485_ACQ_H

#include <glib.h>
#include <defines.h>
#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <temp_probe_RS485_tsys01.h>
#include <temp_probe_12ch_10k.h>
#include <dp_sensor.h>

#define ERROR_DATA_VALUE 999.0

#define SENSOR_TEMP_TSYS01 "02"
#define SENSOR_TEMP_12_CH "03"
#define SENSOR_DP		"05"

#define UART_DEV_NAME "/dev/ttyS0"

int acq_via_rs485(core_module *cm);

#endif
