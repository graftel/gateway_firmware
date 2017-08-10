#include <errno.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <stdint.h>
// channel is the wiringPi name for the chip select (or chip enable) pin.
// Set this to 0 or 1, depending on how it's connected.
static const int CHANNEL = 0;

int main()
{
   int fd, result;
   unsigned char buffer[100];


   // Configure the interface.
   // CHANNEL insicates chip select,
   // 500000 indicates bus speed.
   fd = wiringPiSPISetup(CHANNEL, 100000);

   // clear display
   buffer[0] = 0x02;
   result = wiringPiSPIDataRW(CHANNEL, buffer, 1);
	printf("result=%d\n",result);
   printf("buffer1=0x%02x\n",buffer[0]);
/*    buffer[0] = 0x05;
   buffer[1] = 0x0F;
   buffer[2] = 0xE0;
   buffer[3] = 0x80;
   result = wiringPiSPIDataRW(CHANNEL, buffer, 4);
   
   printf("result=%d\n",result);
   printf("buffer1=0x%02x\n",buffer[0]);
   printf("buffer1=0x%02x\n",buffer[1]);
   printf("buffer2=0x%02x\n",buffer[2]);
   printf("buffer3=0x%02x\n",buffer[3]);
   
   
   sleep(1);
   
   buffer[0] = 0x03;
   buffer[1] = 0x0F;
   buffer[2] = 0x00;
   printf("buffer1=0x%02x\n",buffer[0]);
   printf("buffer1=0x%02x\n",buffer[1]);
   printf("buffer2=0x%02x\n",buffer[2]);
   result = wiringPiSPIDataRW(CHANNEL, buffer, 3);

   printf("result=%d\n",result);
   printf("buffer1=0x%02x\n",buffer[0]);
   printf("buffer1=0x%02x\n",buffer[1]);
   printf("buffer2=0x%02x\n",buffer[2]); */
   

}