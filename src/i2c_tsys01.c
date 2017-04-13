// i2c interface to read tsys01 probe
#include <i2c_tsys01.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#define TSYS01Address 0x66  //address left shifted by arduino as required to match datasheet
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
uint16_t coefficent[5];
uint8_t MSB;//hol
uint8_t OSB;
uint8_t LSB;
float Temp;
float tmp_value = 0.0;
float temp_data[100];
char strTmp[50];
int status;
int file;
int fd;
uint8_t buf[3] = {0x00};

void TSYS01INIT()
{

  const char *devName = "/dev/i2c-1";
  
  if ((fd = open (devName, O_RDWR)) < 0)
  {
	  perror(devName);
	  return;
  }

   if (ioctl(fd, I2C_SLAVE, TSYS01Address) < 0)
   {
       perror("Failed to acquire bus access and/or talk to slave");
       return;
   }

  
    buf[0] = TSYS01Reset;

	if (write(fd, buf, 1) != 1)
	{
		perror("Failed to write to the i2c bus");
		return;
	}

  usleep(10000);
  
}

void GetCoefs()  //gathers calibration coefficients to array
{
  uint8_t tx_data = 0x00;

  uint8_t n = 0;
  
  
  for (n =0; n<5;n++)
  {
	tx_data = 0xA2+(n*2);
	
	buf[0] = tx_data;
	if (write(fd, buf, 1) != 1)
    {
        perror("Failed to write to the i2c bus");
        return;
    }
	
	 if (read(fd,buf,2) != 2)
     {
         perror("Failed to read from the i2c bus");
         return;
     }
	

	uint8_t Ai = buf[0];
	uint8_t Bi = buf[1];
	
	printf("Ai=0x%02x\n",Ai);
	printf("Bi=0x%02x\n",Bi);
    
	uint16_t x = (uint16_t)Ai << 8;
    x+=Bi;
    coefficent[n] =x;
	
	
  }
}

float scaleTemp_C(uint16_t rawAdc)
{

  float retVal =
    (-2)* (float)coefficent[K4] * (float)pow(10,-21) * pow(rawAdc,4) +
    4 * (float)coefficent[K3] * (float)pow(10,-16) * pow(rawAdc,3) +
    (-2) * (float)coefficent[K2] * (float)pow(10,-11) * pow(rawAdc,2) +
    1 * (float)coefficent[K1] * (float)pow(10,-6) * rawAdc +
    (-1.5) * (float)coefficent[K0] * (float)pow(10,-2);

  return retVal;

}

void TSYS01GetTemp()
 {
  Temp = 999.99;
  uint8_t tx_data = TSYS01StartReg;
  uint8_t rx_data[3];

  buf[0] = tx_data;
  if (write(fd, buf, 1) != 1)
  {
	    perror("Failed to write to the i2c bus");
        return;
  }
	
  usleep(10000);
  
  tx_data = TSYS01TempReg;
  
  buf[0] = tx_data;
  if (write(fd, buf, 1) != 1)
  {
	    perror("Failed to write to the i2c bus");
        return;
  }
  
  buf[0] = tx_data;
  if (read(fd, buf, 3) != 3)
  {
	    perror("Failed to write to the i2c bus");
        return;
  }

  MSB = buf[0];
  OSB = buf[1];
  LSB = buf[2];
  
  Temp = scaleTemp_C((((unsigned long)MSB << 8) | ((unsigned long)OSB))); //convert and cast to Temp with scaling equation
	
  printf("Temp=%f\n",Temp);
}

void GetReading()
{
	TSYS01INIT();
	GetCoefs();
	TSYS01GetTemp();
}