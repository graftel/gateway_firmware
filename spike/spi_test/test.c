/* #include "mraa.h"
#include <unistd.h>
#include <stdint.h> */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device = "/dev/spidev0.0";
static uint8_t mode = 3;
static uint8_t bits = 8;
static uint32_t speed = 1000000;
static uint16_t delay;

static void transfer(int fd)
{
	int ret;
	
	uint8_t tx = 0xC0;
	uint8_t rx = 0x00;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = 0,
		.bits_per_word = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
		printf("can't send spi message1\n");

	for (ret = 0; ret < 1; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx);
	}
	puts("");
	
	usleep(100000);
	
	uint8_t tx1[] = {0x05,0x0F,0xE0,0x80};
	uint8_t rx1[ARRAY_SIZE(tx1)] = {0, };
	struct spi_ioc_transfer tr1 = {
		.tx_buf = (unsigned long)tx1,
		.rx_buf = (unsigned long)rx1,
		.len = ARRAY_SIZE(tx1),
		.delay_usecs = delay,
		.speed_hz = 0,
		.bits_per_word = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr1);
	if (ret == 1)
		printf("can't send spi message2\n");

	for (ret = 0; ret < ARRAY_SIZE(tx1); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx1[ret]);
	}
	puts("");
	
	uint8_t tx2[] = {0x03,0x0F,0x00};
	uint8_t rx2[ARRAY_SIZE(tx2)] = {0, };
	struct spi_ioc_transfer tr2 = {
		.tx_buf = (unsigned long)tx2,
		.rx_buf = (unsigned long)rx2,
		.len = ARRAY_SIZE(tx2),
		.delay_usecs = delay,
		.speed_hz = 0,
		.bits_per_word = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);
	if (ret == 1)
		printf("can't send spi message3\n");

	for (ret = 0; ret < ARRAY_SIZE(tx2); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx2[ret]);
	}
	puts("");
}

int main(int argc, char **argv)
{
	int ret = 0;
	int fd;
	
	fd = open(device, O_RDWR);
	if (fd < 0)
		printf("can't open device");
	
		ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		printf("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	transfer(fd);
/* 	uint8_t i;
	mraa_init();
    printf("MRAA Version: %s\n\n", mraa_get_version() );
	
	mraa_spi_context spi;
	
	mraa_gpio_context cs;
    cs = mraa_gpio_init(24);
	mraa_gpio_mode(cs, MRAA_GPIO_PULLUP);
    mraa_gpio_dir(cs, MRAA_GPIO_OUT);
	mraa_gpio_write(cs, 1);
	
	usleep(1000);
	
	spi = mraa_spi_init(0);
	
	if (spi == NULL)
	{
		printf("spi init fail\n");
		exit(1);
	}
	mraa_gpio_write(cs, 0);
	mraa_spi_write(spi,0x01);		//reset 
	mraa_gpio_write(cs, 1);
	usleep(10000);
	
	mraa_gpio_write(cs, 0);
	mraa_spi_write(spi,0x05);		//reset 
	mraa_spi_write(spi,0x0F);		//reset 
	mraa_spi_write(spi,0xE0);		//reset 
	mraa_spi_write(spi,0x80);		//reset 
	mraa_gpio_write(cs, 1);
	
	mraa_gpio_write(cs, 0);
	mraa_spi_write(spi,0x03);		//reset 
	mraa_spi_write(spi,0x0F);		//reset 
	i = mraa_spi_write(spi,0x00);		//reset 
	mraa_gpio_write(cs, 1);
	
	
	
	printf("re:0x%02x\n",i);
	
	usleep(1000); */
	
	
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