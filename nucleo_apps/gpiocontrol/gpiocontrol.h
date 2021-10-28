/*
 * gpio control functions declarations to communicate with multi driver for stm32l4a6 nucleo board
 * this header file should be included in programs which needs gpio control
 * 
 * Author: Damian Loewnau
 */

#ifndef GPIOCONTROL_H
#define GPIOCONTROL_H

#include <stdlib.h>
#include <unistd.h>
#include <stm32l4-multi.h>
#include <sys/ioctl.h>
#include <sys/msg.h>

enum gpiocontrol_modes { input = 0,
	output,
	alternate } gpiocontrol_mode;
enum gpiocontrol_otypes { pushPull = 0,
	openDrain } gpiocontrol_otype;
enum gpiocontrol_ospeeds { low = 0,
	medium,
	high,
	veryHigh } gpiocontrol_ospeed;
enum gpiocontrol_pulls { noPull = 0,
	pullUp,
	pullDown } gpiocontrol_pull;

extern int gpiocontrol_configPin(msg_t *msg, int port, char pin, char mode, char af, char ospeed, char otype, char pupd);
extern int gpiocontrol_setPort(msg_t *msg, int port, unsigned int mask, unsigned int state);
extern int gpiocontrol_setPin(msg_t *msg, int port, int pin, int state);

#endif
