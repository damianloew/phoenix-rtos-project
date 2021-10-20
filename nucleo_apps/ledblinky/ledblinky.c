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

enum modes { input = 0,
	output,
	alternate } mode;
enum otypes { pushPull = 0,
	openDrain } otype;
enum ospeeds { low = 0,
	medium,
	high,
	veryHigh } ospeed;
enum pulls { noPull = 0,
	pullUp,
	pullDown } pull;

static oid_t multi = { .port = 1, .id = -1 };
static msg_t _msg;

static int ledblinky_gpioConfigPin(msg_t *msg, int port, char pin, char mode, char af, char ospeed, char otype, char pupd)
{
	multi_i_t *imsg = (multi_i_t *)msg->i.raw;
	multi_o_t *omsg = (multi_o_t *)msg->o.raw;

	msg->type = mtDevCtl;
	msg->o.data = NULL;
	msg->o.size = 0;
	msg->i.data = NULL;
	msg->i.size = 0;

	imsg->type = gpio_def;

	imsg->gpio_def.port = port;
	imsg->gpio_def.pin = pin;
	imsg->gpio_def.mode = mode;
	imsg->gpio_def.af = af;
	imsg->gpio_def.ospeed = ospeed;
	imsg->gpio_def.otype = otype;
	imsg->gpio_def.pupd = pupd;

	msgSend(multi.port, msg);

	return omsg->err;
}

static int ledblinky_gpioSetPort(msg_t *msg, int port, unsigned int mask, unsigned int state)
{
	multi_i_t *imsg = (multi_i_t *)msg->i.raw;
	multi_o_t *omsg = (multi_o_t *)msg->o.raw;

	msg->type = mtDevCtl;
	msg->o.data = NULL;
	msg->o.size = 0;
	msg->i.data = NULL;
	msg->i.size = 0;

	imsg->type = gpio_set;

	imsg->gpio_set.port = port;
	imsg->gpio_set.mask = mask;
	imsg->gpio_set.state = state;

	msgSend(multi.port, msg);

	return omsg->err;
}


static int ledblinky_gpioSetPin(msg_t *msg, int port, int pin, int state)
{
	return ledblinky_gpioSetPort(msg, port, 1 << pin, (!!state) << pin);
}


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

	ledblinky_gpioConfigPin(&_msg, port, pinNr, output, 0, pushPull, low, pullDown);
	for (int i = (blinksCount * 2); i > 0; i--) {
		ret = ledblinky_gpioSetPin(&_msg, port, pinNr, ~i % 2);
		usleep(500000);
	}

	return ret;
}


int main(int argc, char **argv)
{
	int ret;

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

	return 0;
}
