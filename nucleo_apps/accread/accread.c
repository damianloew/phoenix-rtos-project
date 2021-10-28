/*
 * Accelerometer read app for stm32l4a6 nucleo board
 *
 * Author: Damian Loewnau
 */

#include <stdio.h>
#include <stdlib.h>
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
	unsigned char obuff[10];

	if (argc < 2) {
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

		/* the first test transaction */
		gpiocontrol_setPin(&_msg, gpioa, 4, 0);
		libspi_transaction(&spi, spi_dir_read, 0xff, 0, spi_cmd, &ibuff[0], NULL, 1);
		gpiocontrol_setPin(&_msg, gpioa, 4, 1);

		for (i = 0; i < atoi(argv[1]); i++) {
			gpiocontrol_setPin(&_msg, gpioa, 4, 0);
			libspi_transaction(&spi, spi_dir_read, 0x8f, 0, spi_cmd, &ibuff[0], NULL, 2);
			gpiocontrol_setPin(&_msg, gpioa, 4, 1);
			printf("Read LSM6DS3 address = %x\n", ibuff[0]);
			usleep(1000000);
		}
	}

	return 0;
}
