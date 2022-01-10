/*
 * Accelerometer read app for stm32l4a6 nucleo board
 *
 * Author: Damian Loewnau
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <stm32l4-multi.h>
#include <libmulti/libspi.h>
#include <sys/msg.h>
#include <sys/threads.h>
#include <sys/ioctl.h>
#include "../gpiocontrol/gpiocontrol.h"


volatile unsigned int flagQuit;
static msg_t _msg;
static unsigned char ibuff[60];
static unsigned char obuff[10] = { 0xAC, 0x38 };
static bool isRead;
static unsigned int read_samples;
handle_t lock;


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


/* Enables/disables canon mode and echo */
static void accread_switchmode(int canon)
{
	struct termios state;

	tcgetattr(STDIN_FILENO, &state);
	if (canon) {
		state.c_lflag |= ICANON;
		state.c_lflag |= ECHO;
	}
	else {
		state.c_lflag &= ~ICANON;
		state.c_lflag &= ~ECHO;
		state.c_cc[VMIN] = 1;
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &state);
}


static void accread_getKey(void *arg)
{
	char ch;
	while ((flagQuit != 1) && (ch = getchar())) {
		mutexLock(lock);
		switch (ch) {
			case 'q':
			case 3:
				flagQuit = 1;
				break;
			default:
				break;
		}
		mutexUnlock(lock);
	}

	endthread();
}


static void accread_readData(void *arg)
{
	while (flagQuit != 1) {
		isRead = 0;
		/* store max last 10 samples */
		for (int i = 0; (i < 56 && !isRead); i += 6) {
			mutexLock(lock);
			gpiocontrol_setPin(&_msg, gpioa, 4, 0);
			libspi_transaction(&spi, spi_dir_read, 0xA8, 0, spi_cmd, &ibuff[i], NULL, 2);
			gpiocontrol_setPin(&_msg, gpioa, 4, 1);
			gpiocontrol_setPin(&_msg, gpioa, 4, 0);
			libspi_transaction(&spi, spi_dir_read, 0xAA, 0, spi_cmd, &ibuff[i + 2], NULL, 2);
			gpiocontrol_setPin(&_msg, gpioa, 4, 1);
			gpiocontrol_setPin(&_msg, gpioa, 4, 0);
			libspi_transaction(&spi, spi_dir_read, 0xAC, 0, spi_cmd, &ibuff[i + 4], NULL, 2);
			gpiocontrol_setPin(&_msg, gpioa, 4, 1);
			read_samples = i / 6;
			mutexUnlock(lock);
			usleep(100000);
		}
	}

	endthread();
}


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


static void acccread_accInit(void)
{
	/* the first test transaction */
	gpiocontrol_setPin(&_msg, gpioa, 4, 0);
	libspi_transaction(&spi, spi_dir_read, 0xff, 0, spi_cmd, &ibuff[0], NULL, 1);
	gpiocontrol_setPin(&_msg, gpioa, 4, 1);

	/* set 6 kHz baud rate for the accelerometer */
	gpiocontrol_setPin(&_msg, gpioa, 4, 0);
	libspi_transaction(&spi, spi_dir_write, 0x10, 0, spi_cmd, NULL, &obuff[0], 1);
	gpiocontrol_setPin(&_msg, gpioa, 4, 1);

	/* enable all axes for the accelerometer */
	gpiocontrol_setPin(&_msg, gpioa, 4, 0);
	libspi_transaction(&spi, spi_dir_write, 0x18, 0, spi_cmd, NULL, &obuff[1], 1);
	gpiocontrol_setPin(&_msg, gpioa, 4, 1);
}


int main(int argc, char **argv)
{
	char *readingDataStack, *gettingKeyStack;
	int ret, i, j, var;
	static const unsigned int stacksz = 256;
	short xdata, ydata, zdata;

	if (argc < 2) {
		fprintf(stderr, "Please specify the number of last read samples to print\n");
		return 1;
	}
	else {
		accread_setSpiLine("ss");
		accread_setSpiLine("sck");
		accread_setSpiLine("miso");
		accread_setSpiLine("mosi");

		if ((ret = libspi_init(&spi, spi1, 0)) != 0) {
			fprintf(stderr, "libspi: init function failed\n");
			return ret;
		}

		if ((ret = libspi_configure(&spi, notcpolnotcpha, fpclk_2, enable)) != 0) {
			fprintf(stderr, "libspi: configure function failed\n");
			return ret;
		}

		if ((readingDataStack = (char *)malloc(stacksz)) == NULL)
			return 1;

		if ((gettingKeyStack = (char *)malloc(128)) == NULL)
			return 1;

		acccread_accInit();
		accread_switchmode(0);

		mutexCreate(&lock);
		beginthread(accread_readData, 4, readingDataStack, stacksz, (void *)&var);
		beginthread(accread_getKey, 4, gettingKeyStack, stacksz, (void *)&var);

		j = 0;
		do {
			j++;
			isRead = 1;
			mutexLock(lock);
			for (int sampleNr = read_samples - 1; (sampleNr >= 0 && sampleNr >= (read_samples - atoi(argv[1]))); sampleNr--) {
				i = sampleNr * 6;
				xdata = (int)(ibuff[i + 1] * 256 + ibuff[i]);
				ydata = (int)(ibuff[i + 3] * 256 + ibuff[i + 2]);
				zdata = (int)(ibuff[i + 5] * 256 + ibuff[i + 4]);
				printf("Read LSM6DS3 data: x= %5d\ty=%5d\tz=%5d\n", xdata, ydata, zdata);
			}
			printf("-----------------\n");
			mutexUnlock(lock);
			usleep(1000000);
		} while (flagQuit != 1 && j < 10);
	}

	accread_switchmode(1);

	return 0;
}
