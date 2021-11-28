/*
 * Accelerometer read app for stm32l4a6 nucleo board
 *
 * Author: Damian Loewnau
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stm32l4-multi.h>
#include <libmulti/libspi.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include "../gpiocontrol/gpiocontrol.h"


static msg_t _msg;


enum modes { notcpolnotcpha = 0,
	notcpolcpha,
	cpolnotcpha,
	cpolcpha } mode;

enum baudRates { fpclk_2 = 0,
	fpclk_4,
	fpclk_8,
	fpclk_16,
	fpclk_32,
	fpclk_64,
	fpclk_128,
	fpclk_256 } baudRate;

enum states { disable = 0,
	enable } state;

libspi_ctx_t spi;


static int accread_setSpiLine(char *name)
{
	if (strcmp(name, "ss") == 0) {
		gpiocontrol_configPin(&_msg, gpioa, 4, output, 0, pushPull, high, pullUp);
	}
	else if (strcmp(name, "sck") == 0) {
		gpiocontrol_configPin(&_msg, gpioa, 5, alternate, 5, pushPull, high, pullDown);
	}
	else if (strcmp(name, "miso") == 0) {
		gpiocontrol_configPin(&_msg, gpioa, 6, alternate, 5, pushPull, high, pullUp);
	}
	else if (strcmp(name, "mosi") == 0) {
		gpiocontrol_configPin(&_msg, gpioa, 7, alternate, 5, pushPull, high, pullUp);
	}
	else {
		fprintf(stderr, "Wrong spi line name, please type ss, sck, miso or mosi\n");
		return 1;
	}

	return 0;
}


int main(int argc, char **argv)
{
	int ret, i;
	unsigned char ibuff[10];
	unsigned char obuff[10] = {0xAC, 0x38};
	short xdata, ydata, zdata;
	int a;
	a = -16000;
	if (argc < 2) {printf("xdata hex = %x\n", xdata);
		fprintf(stderr, "Please specify the number of address reads\n");
		return 1;
	}
	else {
		accread_setSpiLine("ss");
		accread_setSpiLine("sck");
		accread_setSpiLine("miso");
		accread_setSpiLine("mosi");

		if ((ret = libspi_init(&spi, spi1, 0)) != 0) {
			fprintf(stderr, "libspi: init function failed\n");
			usleep(500000);
			return ret;
		}

		if ((ret = libspi_configure(&spi, notcpolnotcpha, fpclk_2, enable)) != 0) {
			fprintf(stderr, "libspi: configure function failed\n");
			return ret;
		}

	printf("-16000 in hex = %x, 0xFFFFC180 in decimal = %d\n", -16000, 0xFFFFC180);
	printf("sizeof int = %d sizeof short = %d\n", sizeof(int), sizeof(short));
	printf("a in hex = %x\n", a);
	printf("a in hex = %x\n", a);


		/* the first test transaction */
		gpiocontrol_setPin(&_msg, gpioa, 4, 0);
		libspi_transaction(&spi, spi_dir_read, 0xff, 0, spi_cmd, &ibuff[0], NULL, 1);
		gpiocontrol_setPin(&_msg, gpioa, 4, 1);

		gpiocontrol_setPin(&_msg, gpioa, 4, 0);
		libspi_transaction(&spi, spi_dir_write, 0x10, 0, spi_cmd, NULL, &obuff[0], 1); //acc set 6 KHZ baud rate, +-8g
		gpiocontrol_setPin(&_msg, gpioa, 4, 1);

		gpiocontrol_setPin(&_msg, gpioa, 4, 0);
		libspi_transaction(&spi, spi_dir_write, 0x18, 0, spi_cmd, NULL, &obuff[1], 1); //acc z,y,z axis enable
		gpiocontrol_setPin(&_msg, gpioa, 4, 1);

		gpiocontrol_setPin(&_msg, gpioa, 4, 0);
		libspi_transaction(&spi, spi_dir_read, 0x90, 0, spi_cmd, &ibuff[8], NULL, 1); //acc set max baud rate, +-8g
		gpiocontrol_setPin(&_msg, gpioa, 4, 1);
		printf("Read LSM6DS3 ctrl reg = %x\n", ibuff[8]);

		gpiocontrol_setPin(&_msg, gpioa, 4, 0);
		libspi_transaction(&spi, spi_dir_read, 0x98, 0, spi_cmd, &ibuff[7], NULL, 1); //acc set max baud rate, +-8g
		gpiocontrol_setPin(&_msg, gpioa, 4, 1);
		printf("Read LSM6DS3 ctrl reg enable (2) = %x\n", ibuff[7]);

		for (i = 0; i < atoi(argv[1]); i++) {
			gpiocontrol_setPin(&_msg, gpioa, 4, 0);
			libspi_transaction(&spi, spi_dir_read, 0xA8, 0, spi_cmd, &ibuff[0], NULL, 2);
			gpiocontrol_setPin(&_msg, gpioa, 4, 1);
			gpiocontrol_setPin(&_msg, gpioa, 4, 0);
			libspi_transaction(&spi, spi_dir_read, 0xAA, 0, spi_cmd, &ibuff[2], NULL, 2);
			gpiocontrol_setPin(&_msg, gpioa, 4, 1);
			gpiocontrol_setPin(&_msg, gpioa, 4, 0);
			libspi_transaction(&spi, spi_dir_read, 0xAC, 0, spi_cmd, &ibuff[4], NULL, 2);
			gpiocontrol_setPin(&_msg, gpioa, 4, 1);
			xdata = (int)(ibuff[1]*256+ibuff[0]);
			// printf("xdata hex = %x\n", xdata);
			ydata = (int)(ibuff[3]*256+ibuff[2]);
			// printf("ydata hex = %x\n", ydata);
			zdata = (int)(ibuff[5]*256+ibuff[4]);
			// printf("zdata hex = %x\n", zdata);
			printf("Read LSM6DS3 data: x= %5d\ty=%5d\tz=%5d\n", xdata, ydata, zdata);
			// printf("Read LSM6DS3 data: x= %2x %2x\ty=%2x %2x\tz=%2x %2x\n", ibuff[1], ibuff[0], ibuff[3], ibuff[2], ibuff[5], ibuff[4]);
			// printf("Read LSM6DS3 data: x= %d\n", xdata);
			// printf("Read LSM6DS3 addr = %x\n", ibuff[9]);
			// printf("Read LSM6DS3 x data = %x\n", ibuff[0]);
			usleep(200000);
		}
	}

	return 0;
}
