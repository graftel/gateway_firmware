// i2c interface to read tsys01 probe
#include <i2c_tsys01.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <defines.h>
#include <utilities.h>

//#define TSYS01Address 0x66  //address left shifted by arduino as required to match datasheet
#define TSYS01ADDR_START_POS 4
#define TSYS01Reset 0x1E //initiates reset
#define TSYS01StartReg 0x48 //commands sensor to begin measurement / output calculation
#define TSYS01TempReg 0x00 //requests most recent output from measurement

typedef enum KPoly_E //structure to hold calibration values from temperature sensor registers
{
  K4 = 0,
  K3,
  K2,
  K1,
  K0
}
KPoly_T;
long tempReading = 0;
float finalTempC = 0.0000;
uint8_t MSB;//hol
uint8_t OSB;
uint8_t LSB;
float Temp;
float tmp_value = 0.0;
float temp_data[100];
char strTmp[50];
int status;

uint8_t buf[3] = {0x00};

const char *devName = "/dev/i2c-1";

void *read_i2c_sensor_data_wrapper(void *args)
{
	bridge *arguments = args;
	
	return (void *)read_tsys01_sensors(arguments);
}

int get_sensor_addr(bridge *b_data, int i2c_sensor_index, uint8_t *addr)
{
	if (sizeof(b_data->sen[i2c_sensor_index].addr) < MAX_SENSOR_ADDR)
	{
		return -1;
	}
	
	(*addr) = char2hex(b_data->sen[i2c_sensor_index].addr, TSYS01ADDR_START_POS);
	
	return 0;
	
}

int read_tsys01_sensors(bridge *b_data)
{
	uint8_t i;
	for( i = 0; i < b_data->size_sen; i++)
	{
		TSYS01_Sensor sensor;
	
		uint8_t addr;
		if (get_sensor_addr(b_data,i,&addr) == 0)
		{
			TSYS01_init(&sensor, addr);
			
			TSYS01_GetTemp(&sensor);
			
			b_data->sen[i].data = sensor.temp_reading;
			
			//printf("addr=0x%02x temp=%f\n",addr,sensor.temp_reading);
		}
		
	}
}




int TSYS01_init(TSYS01_Sensor *sensor, uint8_t addr)
{ 
  int fd;
  
  if ((fd = open (devName, O_RDWR)) < 0)
  {
	  perror(devName);
	  close(fd);
	  return -1;
  }

   if (ioctl(fd, I2C_SLAVE, addr) < 0)
   {
       perror("Failed to acquire bus access and/or talk to slave");
	   close(fd);
       return -1;
   }

   sensor->i2c_addr = addr;
  
    buf[0] = TSYS01Reset;

	if (write(fd, buf, 1) != 1)
	{
		perror("Failed to write to the i2c bus");
		close(fd);
		return -1;
	}

  usleep(10000);
  
  uint8_t tx_data = 0x00;

  uint8_t n = 0;
  
  
  for (n =0; n<5;n++)
  {
	tx_data = 0xA2+(n*2);
	
	buf[0] = tx_data;
	if (write(fd, buf, 1) != 1)
    {
        perror("Failed to write to the i2c bus");
		close(fd);
        return -1;
    }
	
	 if (read(fd,buf,2) != 2)
     {
         perror("Failed to read from the i2c bus");
		 close(fd);
         return -1;
     }
	

	uint8_t Ai = buf[0];
	uint8_t Bi = buf[1];
	
//	printf("Ai=0x%02x\n",Ai);
//	printf("Bi=0x%02x\n",Bi);
    
	uint16_t x = (uint16_t)Ai << 8;
    x+=Bi;
    sensor->coefficent[n] =x;
	
	
  }
  
  close(fd);
  
  sensor->init_status = 1;
  
  return 0;
  
}

float scaleTemp_C(uint16_t rawAdc, uint16_t *coefficent)
{

  float retVal =
    (-2)* (float)coefficent[K4] * (float)pow(10,-21) * pow(rawAdc,4) +
    4 * (float)coefficent[K3] * (float)pow(10,-16) * pow(rawAdc,3) +
    (-2) * (float)coefficent[K2] * (float)pow(10,-11) * pow(rawAdc,2) +
    1 * (float)coefficent[K1] * (float)pow(10,-6) * rawAdc +
    (-1.5) * (float)coefficent[K0] * (float)pow(10,-2);

  return retVal;

}


int TSYS01_GetTemp_WithCal(char *addr, char *calInfo, float *temp1)
{
	int iAddr;
	uint8_t uAddr;
	if (addr == NULL)
	{
		perror("Address error");
		return 0;
	}
	
	sscanf(addr, "0x%02x", &iAddr);
	
	uAddr = (uint8_t)iAddr;
	
	// printf("addr = 0x%02x\n",uAddr);
	
	if (uAddr > 0x80)
	{
		perror("Address wrong");
		return 0;
	}
	
	uint16_t coef[5];
	char * pEnd;
    coef[0] = strtol(calInfo,&pEnd,16);
    coef[1] = strtol(pEnd,&pEnd,16);
	coef[2] = strtol(pEnd,&pEnd,16);
    coef[3] = strtol(pEnd,&pEnd,16);
	coef[4] = strtol(pEnd,NULL,16);
	
	// printf("coef[0]=0x%04x\n",coef[0]);
	// printf("coef[1]=0x%04x\n",coef[1]);
	// printf("coef[2]=0x%04x\n",coef[2]);
	// printf("coef[3]=0x%04x\n",coef[3]);
	// printf("coef[4]=0x%04x\n",coef[4]);
	int fd;
	
  if ((fd = open (devName, O_RDWR)) < 0)
  {
	  perror(devName);
	  return -1;
  }

   if (ioctl(fd, I2C_SLAVE, uAddr) < 0)
   {
       perror("Failed to acquire bus access and/or talk to slave");
       return -1;
   }
	
  float Temp1 = 999.99;
  uint8_t tx_data = TSYS01StartReg;
  uint8_t rx_data[3];

  buf[0] = tx_data;
  if (write(fd, buf, 1) != 1)
  {
	    perror("Failed to write to the i2c bus 1");
        return -1;
  }
	
  usleep(10000);
  
  tx_data = TSYS01TempReg;
  
  buf[0] = tx_data;
  if (write(fd, buf, 1) != 1)
  {
	    perror("Failed to write to the i2c bus 2");
        return -1;
  }
  
  buf[0] = tx_data;
  if (read(fd, buf, 3) != 3)
  {
	    perror("Failed to write to the i2c bus 3");
        return -1;
  }

  MSB = buf[0];
  OSB = buf[1];
  LSB = buf[2];
  
//  printf("MSB=0x%02x\n",MSB);
// printf("OSB=0x%02x\n",OSB);
  
  Temp1 = scaleTemp_C((((unsigned long)MSB << 8) | ((unsigned long)OSB)), coef); //convert and cast to Temp with scaling equation

//  printf("temp1=%f\n",Temp1);
  
  *temp1 = Temp1;
  
  close(fd);
  
  return 0;
	
}

int TSYS01_GetTemp(TSYS01_Sensor *sensor)
{
   int fd;
   
   if (sensor->init_status != 1)
   {
	   perror("sensor not initialized");
	   // not initalized
	   return -1;
   }
	
  if ((fd = open (devName, O_RDWR)) < 0)
  {
	  perror(devName);
	  return -1;
  }

   if (ioctl(fd, I2C_SLAVE, sensor->i2c_addr) < 0)
   {
       perror("Failed to acquire bus access and/or talk to slave");
       return -1;
   }
	
  Temp = 999.99;
  uint8_t tx_data = TSYS01StartReg;
  uint8_t rx_data[3];

  buf[0] = tx_data;
  if (write(fd, buf, 1) != 1)
  {
	    perror("Failed to write to the i2c bus 1");
        return -1;
  }
	
  usleep(10000);
  
  tx_data = TSYS01TempReg;
  
  buf[0] = tx_data;
  if (write(fd, buf, 1) != 1)
  {
	    perror("Failed to write to the i2c bus 2");
        return -1;
  }
  
  buf[0] = tx_data;
  if (read(fd, buf, 3) != 3)
  {
	    perror("Failed to write to the i2c bus 3");
        return -1;
  }

  MSB = buf[0];
  OSB = buf[1];
  LSB = buf[2];
  
  
  
  sensor->temp_reading = scaleTemp_C((((unsigned long)MSB << 8) | ((unsigned long)OSB)), sensor->coefficent); //convert and cast to Temp with scaling equation

  close(fd);
  
  return 0;
}