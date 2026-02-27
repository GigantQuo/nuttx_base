/****************************************************************************
 * apps/examples/switchcore/switchcorer_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <debug.h>

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/ioexpander/gpio.h>
#include <arch/board/board.h>

#include "switchcorer.h"
#include "sam_eic.h"
#include "sam_port.h"
#include "nvic.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Switchcore_main
 ****************************************************************************/
#define SAM_EIC_BASE       0x40001800
#define SAM_PORT_BASE      0x41004400


#define getreg32(n)   (*(int*)n)
#define getreg8(n)   (*(char*)n)

int main(int argc, FAR char *argv[])
{
	//bool outvalue;
	//bool invalue;
	//int fd;
	//*(char*)0x4100443e = 0x7;

	printf("SwitchcoreR is running!\n");
	//*(unsigned int*)0x20000000 = 0;

	// *(unsigned int*)0x40001818 = 0x44440000;
	// printf("1");
	// *(unsigned int*)0x4000181C = 0x44;
	// printf("2");
	// *(unsigned int*)0x40001800 = 0x2;
	// printf("3");
	// *(unsigned int*)0x4000180C = 0x10;
	// printf("4");

	/*
	 *	printf("NVIC:\n");
	 *	printf("  ISER:       %08x ICER:   %08x\n",
	 *			(unsigned int)getreg32(ARMV6M_NVIC_ISER),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_ICER));
	 *
	 *	printf("  ISPR:       %08x ICPR:   %08x\n",
	 *			(unsigned int)getreg32(ARMV6M_NVIC_ISPR),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_ICPR));
	 *
	 *	printf("  IRQ PRIO:   %08x %08x %08x %08x\n",
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR0),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR1),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR2),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR3));
	 *
	 *	printf("              %08x %08x %08x %08x\n",
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR4),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR5),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR6),
	 *			(unsigned int)getreg32(ARMV6M_NVIC_IPR7));
	 *
	 *	printf("EIC:\n");
	 *	printf("  CTRLA:    %02x\n", (unsigned int)getreg8(SAM_EIC_CTRLA));
	 *	printf("  STATUS:   %02x\n", (unsigned int)getreg8(SAM_EIC_STATUS));
	 *	printf("  NMICTRL:  %02x\n", (unsigned int)getreg8(SAM_EIC_NMICTRL));
	 *	printf("  NMIFLAG:  %02x\n", (unsigned int)getreg8(SAM_EIC_NMIFLAG));
	 *	printf("  EVCTRL:   %08x\n", (unsigned int)getreg32(SAM_EIC_EVCTRL));
	 *	printf("  INTENCLR: %08x\n", (unsigned int)getreg32(SAM_EIC_INTENCLR));
	 *	printf("  INTENSET: %08x\n", (unsigned int)getreg32(SAM_EIC_INTENSET));
	 *	printf("  INTFLAG:  %08x\n", (unsigned int)getreg32(SAM_EIC_INTFLAG));
	 *	printf("  WAKEUP:   %08x\n", (unsigned int)getreg32(SAM_EIC_WAKEUP));
	 *	printf("  CONFIG0:  %08x\n", (unsigned int)getreg32(SAM_EIC_CONFIG0));
	 *	printf("  CONFIG1:  %08x\n", (unsigned int)getreg32(SAM_EIC_CONFIG1));
	 *	printf("  CONFIG2:  %08x\n", (unsigned int)getreg32(SAM_EIC_CONFIG2));
	 *
	 *
	 *	unsigned int base = SAM_PORTA_BASE;
	 *	printf("  DIR: %08x OUT: %08x IN: %08x\n",
	 *			 (unsigned int)getreg32(base + SAM_PORT_DIR_OFFSET),
	 *			 (unsigned int)getreg32(base + SAM_PORT_OUT_OFFSET),
	 *			 (unsigned int)getreg32(base + SAM_PORT_IN_OFFSET));
	 *
	 *	printf("  CTRL: %08x WRCONFIG: %08x\n",
	 *			 (unsigned int)getreg32(base + SAM_PORT_CTRL_OFFSET),
	 *			 (unsigned int)getreg32(base + SAM_PORT_WRCONFIG_OFFSET));
	 *
	 *	printf("  PMUX[%08x]: %02x PINCFG[%08x]: %02x\n",
	 *			 SAM_PORTA_PMUX2,
	 *			 (unsigned int)getreg8(SAM_PORTA_PMUX2),
	 *			 SAM_PORTA_PINCFG4,
	 *			 (unsigned int)getreg8(SAM_PORTA_PINCFG4));
	 *
	 *	printf("ARMV6M_NVIC_ISER: %x\n", ARMV6M_NVIC_ISER);
	 *	printf("ARMV6M_NVIC_IPR1: %x\n", ARMV6M_NVIC_IPR1);
	 *	printf("SAM_PORTA_DIR: %x\n", SAM_PORTA_DIR);
	 *	printf("SAM_PORTA_PMUX2: %x\n", SAM_PORTA_PMUX2);
	 *	printf("SAM_PORTA_PINCFG4: %x\n", SAM_PORTA_PINCFG4);
	 *	printf("SAM_EIC_CONFIG0: %x\n", SAM_EIC_CONFIG0);
	 *	printf("SAM_EIC_CONFIG1: %x\n", SAM_EIC_CONFIG1);
	 *	printf("SAM_EIC_CTRLA: %x\n", SAM_EIC_CTRLA);
	 *	printf("SAM_EIC_INTENSET: %x\n", SAM_EIC_INTENSET);
	 */

	printf("_+_+_+_+_+_\n");
	printf("\nAMATCH: %x\n", *(unsigned int*)0x20000000);
	printf("\nDRDY: %x\n", *(unsigned int*)0x20000004);
	printf("\nPREC: %x\n", *(unsigned int*)0x20000008);
	printf("\nERROR: %x\n", *(unsigned int*)0x2000000C);

	//while(1);


	/*
	 *	fd = open("/dev/i2c1", O_RDWR);
	 *	if (fd < 0)
	 *	{
	 *		int errcode = errno;
	 *		fprintf(stderr, "ERROR: Failed to open %s: %d\n", "/dev/gpio/gpio19", errcode);
	 *		return EXIT_FAILURE;
}


ioctl(fd, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));
printf("Reset state of /dev/gpio/gpio19: %x\n", (int)invalue);

sleep(2);

outvalue = false;
printf("Writing %x to /dev/gpio/gpio19\n", (int)outvalue);
ioctl(fd, GPIOC_WRITE, (unsigned long)0x0);

sleep(1);

ioctl(fd, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));
printf("Verifying /dev/gpio/gpio19: %x\n", (int)invalue);

sleep(2);

outvalue = true;
printf("Writing %x to /dev/gpio/gpio19\n", (int)outvalue);
ioctl(fd, GPIOC_WRITE, (unsigned long)0x1);

sleep(1);

ioctl(fd, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));
printf("Verifying /dev/gpio/gpio19: %x\n", (int)invalue);

close(fd);
*/
	return 0;
}
