// RTC I2C Library
#include <errno.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define RTCaddress 0x6F
//#define DEBUG

bool setupenable = false;

// ********setup parameters**********
int curSec = 00;    // 0-59
int curMin = 49;    // 0-59
int curHour = 12;    // 0-23 in 24-hour format
bool is12 = false;   // 12 = true, 24 = false
int curDay = 11;    // 1-31
int curMonth = 4;   // 1-12
int curYear = 2017; // 2000 - 2099
int curDayofweek = 2;  // 1-7 Monday = 1, Tuesday = 2.....
// ***********************************
uint8_t bSec,bMin,bHour,bDayofWeek,bDay,bMonth,bYear;
bool isLeapYear;

int status;

void MCP79410_Setup_Date(int curSec, int curMin, int curHour, int curDayofweek, 
						 int is12, int curDay, int curMonth, int curYear)
{
	   int fd;
	   uint8_t bSec,bMin,bHour,bDayofWeek,bDay,bMonth,bYear;
	   bool isLeapYear;
	   
	   fd = wiringPiI2CSetup(RTCaddress);
	   
	   bSec = ((8 + curSec / 10) << 4) + curSec % 10; 
       bMin = ((curMin / 10) << 4) + curMin % 10;
       bHour = 0x00;
       bDayofWeek = 0x28;
       bDay = 0x00;
       isLeapYear = false;
       bMonth = 0x00;
       bYear = 0x00;
      if (is12)
      {
          bHour += (0x01 << 6);
          if (curHour >= 12 && curHour <= 23)
          {
            bHour += (0x01 << 5); 
            curHour = curHour - 12;
          }
      }
      bHour += (curHour / 10 << 4);
      bHour += (curHour % 10);
      bDayofWeek += curDayofweek;
      bDay = ((curDay / 10) << 4) + curDay % 10;
      if ((curYear % 4 == 0 && curYear % 100 != 0) || (curYear % 400 == 0))
      {
        isLeapYear = true;
        bMonth += (0x01 << 5);  
      }
      bMonth += (curMonth / 10 << 4);
      bMonth += curMonth % 10;
    
      bYear = (((curYear - 2000) / 10) << 4) + (curYear - 2000) % 10;
	  
	  status = wiringPiI2CWriteReg8(fd,0x00,bSec);
	  status = wiringPiI2CWriteReg8(fd,0x01,bMin);
	  status = wiringPiI2CWriteReg8(fd,0x02,bHour);
	  status = wiringPiI2CWriteReg8(fd,0x03,bDayofWeek);
	  status = wiringPiI2CWriteReg8(fd,0x04,bDay);
	  status = wiringPiI2CWriteReg8(fd,0x05,bMonth);
	  status = wiringPiI2CWriteReg8(fd,0x06,bYear);
}

int MCP79410_Read_Date(struct tm *result)
{
	uint8_t bSec,bMin,bHour,bDayofWeek,bDay,bMonth,bYear;
	bool isLeapYear;
	int fd;
	bool isAM, is12;
	fd = wiringPiI2CSetup(RTCaddress);
	
	bSec = wiringPiI2CReadReg8(fd, 0x00);
	bMin = wiringPiI2CReadReg8(fd, 0x01);
	bHour = wiringPiI2CReadReg8(fd, 0x02);
	bDayofWeek = wiringPiI2CReadReg8(fd, 0x03);
	bDay = wiringPiI2CReadReg8(fd, 0x04);
	bMonth = wiringPiI2CReadReg8(fd, 0x05);
	bYear = wiringPiI2CReadReg8(fd, 0x06);
	
	if (bSec == 0xff || bMin == 0xff || bHour == 0xff || bDayofWeek == 0xff || 
		bDay == 0xff || bMonth == 0xff || bYear == 0xff || bSec == 0x00)
	{
		return -1;	
	}
	
//	printf("read values %02x,%02x,%02x,%02x,%02x,%02x,%02x\n",bSec,bMin,bHour,bDayofWeek,bDay,bMonth,bYear);
	
	result->tm_sec = (((bSec - 0x80) & 0xF0) >> 4) * 10 + ((bSec - 0x80) & 0x0F);
	result->tm_min = ((bMin & 0xF0) >> 4) * 10 + (bMin & 0x0F);
	 
	  if ((bHour & 0x40) == 0x40)
	  {
		  is12 = true;
		  if (bHour & 0x20 == 0x20)
		  {
			isAM = false;
		  }
		  else
		  {
			isAM = true;
		  }
		  result->tm_hour = ((bHour & 0x10) >> 4) * 10 + (bHour & 0x0F);
	  }
	  else
	  {
		is12 = false;
		result->tm_hour = ((bHour & 0x30) >> 4) * 10 + (bHour & 0x0F);
	  }
	  result->tm_wday = ((bDayofWeek & 0x07) == 7) ? 0 : bDayofWeek & 0x07 ; 
	  result->tm_mday = ((bDay & 0x30) >> 4) * 10 + (bDay & 0x0F);
	  result->tm_mon = ((bMonth & 0x10) >> 4) * 10 + (bMonth & 0x0F) - 1;
	  result->tm_year = 100 + ((bYear & 0xF0) >> 4) * 10 + (bYear & 0x0F);
	  
	  return 0;
}

int MCP79410_Read_Epoch_Time(int *epoch)
{
	
	uint8_t bSec,bMin,bHour,bDayofWeek,bDay,bMonth,bYear;
	bool isLeapYear;
	int fd;
	bool isAM, is12;
	fd = wiringPiI2CSetup(RTCaddress);
	
	bSec = wiringPiI2CReadReg8(fd, 0x00);
	bMin = wiringPiI2CReadReg8(fd, 0x01);
	bHour = wiringPiI2CReadReg8(fd, 0x02);
	bDayofWeek = wiringPiI2CReadReg8(fd, 0x03);
	bDay = wiringPiI2CReadReg8(fd, 0x04);
	bMonth = wiringPiI2CReadReg8(fd, 0x05);
	bYear = wiringPiI2CReadReg8(fd, 0x06);
	
	if (bSec == 0xff || bMin == 0xff || bHour == 0xff || bDayofWeek == 0xff || 
		bDay == 0xff || bMonth == 0xff || bYear == 0xff || bSec == 0x00)
	{
		return -1;	
	}
	struct tm result;
//	printf("read values %02x,%02x,%02x,%02x,%02x,%02x,%02x\n",bSec,bMin,bHour,bDayofWeek,bDay,bMonth,bYear);
	
	result.tm_sec = (((bSec - 0x80) & 0xF0) >> 4) * 10 + ((bSec - 0x80) & 0x0F);
	result.tm_min = ((bMin & 0xF0) >> 4) * 10 + (bMin & 0x0F);
	 
	  if ((bHour & 0x40) == 0x40)
	  {
		  is12 = true;
		  if (bHour & 0x20 == 0x20)
		  {
			isAM = false;
		  }
		  else
		  {
			isAM = true;
		  }
		  result.tm_hour = ((bHour & 0x10) >> 4) * 10 + (bHour & 0x0F);
	  }
	  else
	  {
		is12 = false;
		result.tm_hour = ((bHour & 0x30) >> 4) * 10 + (bHour & 0x0F);
	  }
	  result.tm_wday = ((bDayofWeek & 0x07) == 7) ? 0 : bDayofWeek & 0x07 ; 
	  result.tm_mday = ((bDay & 0x30) >> 4) * 10 + (bDay & 0x0F);
	  result.tm_mon = ((bMonth & 0x10) >> 4) * 10 + (bMonth & 0x0F) - 1;
	  result.tm_year = 100 + ((bYear & 0xF0) >> 4) * 10 + (bYear & 0x0F);
	  
	  (*epoch) = mktime(&result);
	  
	  
	  return 0;
}


void MCP79410_Test(){    //sample code to read
	 struct timespec start;
	  struct timespec end;
//	MCP79410_Setup_Date(curSec, curMin, curHour, curDayofweek, is12, curDay, curMonth, curYear);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start); // get initial time-stamp

// ... do stuff ... //


	char time_chars[40];
	
	memset(time_chars,0,sizeof(time_chars));
	
	printf("setting up i2c rtc\n");
	
	struct tm tm_info;
	
	MCP79410_Read_Date(&tm_info);
	
	strftime(time_chars,sizeof(time_chars),"%Y_%m_%d_%H_%M_%S %A",&tm_info);
	
	printf("Time:%s\n",time_chars);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);   // get final time-stamp

	double t_ns = (double)(end.tv_sec - start.tv_sec) * 1.0e9 +
              (double)(end.tv_nsec - start.tv_nsec);
                                                 // subtract time-stamps and
                                                 // multiply to get elapsed
                                                 // time in ns
												 
	printf("time_diff=%f\n",t_ns);
	
}