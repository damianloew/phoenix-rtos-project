/*
 * Phoenix-RTOS
 *
 * Hello World
 *
 * Example of user application
 *
 * Copyright 2021 Phoenix Systems
 * Author: Hubert Buczynski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

// #include <stdio.h>


int main(void)
{
	volatile unsigned int *dirset = 0x50842518;
	volatile unsigned int *outset = 0x50842508;
	*dirset = 1u << 2;
	*outset = 1u << 2;
	while(1) {;}
	// printf("Hello World!!\n");

	return 0;
}
