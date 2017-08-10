/*
 * Author: Jessica Gomez <jessica.gomez.hernandez@intel.com>
 * Copyright (c) 2015 - 2016 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#include <mraa.hpp>
#include <mraa.h>
#include <iostream>
#include <unistd.h>
#include "mcp_can.h"
#include "mcp_can_dfs.h"

int main()
{
	
	mraa_init();
/* 	mraa_uart_context uart;
    uart = mraa_uart_init(0);
	
	if (uart == NULL) {
        fprintf(stderr, "UART failed to setup\n");
        return EXIT_FAILURE;
    }
 */
   unsigned int response = 0;
   unsigned char setting_send[8];
   unsigned char buf[8];
   unsigned char len = 0;
    printf("Hello, SPI initialised\n");
    uint8_t data[] = {0x00, 100};
    uint8_t *recv;
	INT8U ret;
	MCP_CAN CAN(10);
   
    printf("start init spi\n");
    
	ret = CAN.begin(CAN_20KBPS);
	
	if (ret == CAN_OK)
	{
		printf("CAN BUS INIT OK\n");
	}
	else if (ret == CAN_FAILINIT)
	{
		printf("CAN BUS INIT FAIL\n");
		exit (1);
	}
	
	setting_send[0] = 0x40;
	setting_send[1] = 0x30;
	setting_send[2] = 0x61;
	setting_send[3] = 0x01;
	setting_send[4] = 0x00;
	setting_send[5] = 0x00;
	setting_send[6] = 0x00;
	setting_send[7] = 0x00;
	
	while(1)
	{
		if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
		{
			CAN.readMsgBuf(&len, buf);
			
			if (CAN.getCanId() == 0x581)
			{
				if (len == 8)
				{
					 if (buf[0] == 0x43 && buf[1] == 0x30 && buf[2] == 0x61 && buf[3] == 0x01)
					 {
					   printf("1RP=");
					   for (int i = 4;i<len;i++)
					   {
						   printf("%02X",buf[i]);
					   }
					   
					   printf("\n");
					 }
				}
			}
			// mraa_uart_write(uart, buffer, sizeof(buffer));
		}
		
		CAN.sendMsgBuf(0x601, 0, 8, setting_send);
		// mraa_uart_write(uart, buffer, sizeof(buffer));
		sleep(1);
	}

	// mraa_uart_stop(uart);

    mraa_deinit();
	
	return mraa::SUCCESS;
}
