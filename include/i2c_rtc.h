#ifndef _I2C_RTC_H
#define _I2C_RTC_H

int MCP79410_Read_Epoch_Time(int *epoch);
void MCP79410_Setup_Date(int curSec, int curMin, int curHour, int curDayofweek, 
						 int is12, int curDay, int curMonth, int curYear);
						 
#endif