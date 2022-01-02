/*
 * Led blinky app for stm32l4a6 nucleo board
 *
 * Author: Damian Loewnau
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stm32l4-multi.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include "../gpiocontrol/gpiocontrol.h"

static msg_t _msg;


static int ledblinky_blink(unsigned char ledNr, unsigned char blinksCount)
{
	unsigned char pinNr, port;
	int ret;

	switch (ledNr) {
		default:
			fprintf(stderr, "Wrong LED nr, please choose from 1 to 3. Default LED1 is set\n");
		case 1:
			port = gpioc;
			pinNr = 7;
			break;
		case 2:
			port = gpiob;
			pinNr = 7;
			break;
		case 3:
			port = gpiob;
			pinNr = 14;
			break;
	}

	gpiocontrol_configPin(&_msg, port, pinNr, output, 0, pushPull, low, pullDown);
	for (int i = (blinksCount * 2); i > 0; i--) {
		ret = gpiocontrol_setPin(&_msg, port, pinNr, ~i % 2);
		usleep(500000);
	}

	return ret;
}

int main(int argc, char **argv)
{
	int ret;
	int *d;
	int *d1;

	d1= sizeof(unsigned int);
	d = 0;

	// printf("d = %d d1 = %d\n", *d, *d1);
	if (argc != 2) {
		fprintf(stderr, "Please specify nr of led in second argument (from 1 to 3)\n");
		return 1;
	}
	else {
		if ((ret = ledblinky_blink(atoi(argv[1]), 5)) != 0) {
			fprintf(stderr, "ledblinky: blink function failed\n");
			return ret;
		}
	}
	// printf("d = %d d1 = %d\n", *d, *d1);
	return 0;
}
