/*
 * gpio control functions defintions to communicate with multi driver for stm32l4a6 nucleo board
 *
 * Author: Damian Loewnau
 */

#include "gpiocontrol.h"

static oid_t multi = { .port = 1, .id = -1 };

extern int gpiocontrol_configPin(msg_t *msg, int port, char pin, char mode, char af, char ospeed, char otype, char pupd)
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


extern int gpiocontrol_setPort(msg_t *msg, int port, unsigned int mask, unsigned int state)
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


extern int gpiocontrol_setPin(msg_t *msg, int port, int pin, int state)
{
	return gpiocontrol_setPort(msg, port, 1 << pin, (!!state) << pin);
}
