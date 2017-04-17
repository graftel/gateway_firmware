#ifndef _DEFINES_H
#define _DEFINES_H

#include <pthread.h>

typedef struct 
{
   time_t time_stamp;  
   double data;
   char data_tag[50];
   char dev_address[10];
   char data_unit[10];
   char data_type[10];
   char serial_num[20];
   char ble_address[18];
   char protocol[10];
   char note[50];
} sensor_data;


typedef struct{
  sensor_data *data;  
  int size_data;
} bulk_data;



#endif