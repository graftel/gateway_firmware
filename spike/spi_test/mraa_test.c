#include "mraa.h"
#include <unistd.h>
#include <stdint.h> 

int main(int argc, char **argv)
{

 	uint8_t i;
	mraa_init();
    printf("MRAA Version: %s\n\n", mraa_get_version() );
	
	mraa_spi_context spi;
	
//	mraa_gpio_context cs;
 //   cs = mraa_gpio_init(24);
//	mraa_gpio_mode(cs, MRAA_GPIO_PULLUP);
    // mraa_gpio_dir(cs, MRAA_GPIO_OUT);
	// mraa_gpio_write(cs, 1);
	
	// usleep(1000);
	
	spi = mraa_spi_init(0);
	
	if (spi == NULL)
	{
		printf("spi init fail\n");
		exit(1);
	}
//	mraa_gpio_write(cs, 0);
	mraa_spi_write(spi,0xC0);		//reset 
//	mraa_gpio_write(cs, 1);
	usleep(10000);
	mraa_result_t result;
	
	uint8_t data[] = { 0x05, 0x0F, 0xE0, 0x80};
    uint8_t recv[] = { 0x00, 0x00, 0x00, 0x00};

	result = mraa_spi_transfer_buf(spi, data, recv, 4);
	
	
	uint8_t data1[] = { 0x03, 0x0F, 0x00};
    uint8_t recv1[] = { 0x00, 0x00, 0x00};
	
	result = mraa_spi_transfer_buf(spi, data1,recv1, 3);
	
	
	
	printf("re:0x%02x\n",recv1[0]);
	printf("re:0x%02x\n",recv1[1]);
	printf("re:0x%02x\n",recv1[2]);
	
	usleep(1000); 
	
	
/* 	uint8_t tx_read[] = {0x05,0x0F,0xE0,0x80};
	uint8_t *rx_read;
	
	rx_read = mraa_spi_write_buf(spi, tx_read, 4);
	
	printf("Read1:0x%02x\n",rx_read[0]);
	printf("Read2:0x%02x\n",rx_read[1]);
	printf("Read3:0x%02x\n",rx_read[2]);
	printf("Read4:0x%02x\n",rx_read[3]);
	
	int8_t tx_read1[] = {0x03,0x0F,0x00};
	uint8_t *rx_read1;
	
	rx_read1 = mraa_spi_write_buf(spi, tx_read1, 3);
	
	printf("Read5:0x%02x\n",rx_read1[0]);
	printf("Read6:0x%02x\n",rx_read1[1]);
	printf("Read7:0x%02x\n",rx_read1[2]); */
	
	
	
}