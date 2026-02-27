/****************************************************************************
 * apps/examples/switchcore/switchcore_main.c
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
#include <nuttx/timers/watchdog.h>
#include <arch/board/board.h>
#include <nuttx/sched.h>

#include "switchcore.h"
/*
#include "samd_i2c_slave.h"

#include "sam_wdt.h"
#include "sam_sysctrl.h"
#include "sam_pm.h"
#include "sam_gclk.h"

#include "sam_eic.h"
#include "sam_port.h"
#include "nvic.h"

#define SAM_EIC_BASE       0x40001800
#define SAM_PORT_BASE      0x41004400

#define SAM_WDT_BASE       0x40001000
#define SAM_PM_BASE        0x40000400
#define SAM_SYSCTRL_BASE   0x40000800
#define SAM_GCLK_BASE      0x40000c00

#define getreg32(n)   (*(int*)n)
#define getreg16(n)   (*(short*)n)
#define getreg8(n)   (*(char*)n)

#define putreg32(v, n)   (*(int*)n) = v
#define putreg16(v, n)   (*(short*)n) = v
#define putreg8(v, n)   (*(char*)n) = v
*/

/****************************************************************************
* Private Functions Prototypes
****************************************************************************/

static int gpio25_int(int argc, char* argv[]);
static int i2c_slave(int argc, char* argv[]);
//static void dump(void);
static int watchdog(int argc, char* argv[]);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Switchcore_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
	printf("Switchcore is running!\n");

	int ret;
	/*
	printf("Starting i2c_slave task !\n");

	ret = task_create(	"i2c_slave",
						100,
						1024,
						i2c_slave,
						NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("ERROR: Failed to start i2c_slave: %d\n\r", errcode);
		return EXIT_FAILURE;
	}*/

	sleep(1);

	printf("Starting gpio25 interrupt task!\n");

	ret = task_create(	"gpio25_int",
						100,
						1024,
						gpio25_int,
						NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("ERROR: Failed to start gpio25_int: %d\n\r", errcode);
		return EXIT_FAILURE;
	}

	sleep(1);

	printf("Starting watchdog task !\n");

	ret = task_create("watchdog",
						100,
						1024,
						watchdog,
						NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("ERROR: Failed to start watchdog: %d\n\r", errcode);
		return EXIT_FAILURE;
	}

	printf("Done\n\r");

	return 0;
}


static int watchdog(int argc, char* argv[])
{

	sleep(1);
	printf("watchdog is started!\n\r");


	int ret;
	int fd;
	char* devpath;

	devpath = "/dev/watchdog0";

	fd = open(devpath, O_RDWR);
	if (fd < 0)
	{
		printf("Open Error\n");
		return -1;
	}
	else
	{
		printf("Open success!\n\r");
	}

	ret = ioctl(fd, WDIOC_KEEPALIVE, NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("Watchdog keepalive failed: %d\n", errcode);
	}

	ret = ioctl(fd, WDIOC_SETTIMEOUT, (unsigned int)7000);
	if (ret < 0)
	{
		int errcode = errno;
		printf("Watchdog settimeout failed: %d\n", errcode);
	}

	ret = ioctl(fd, WDIOC_START, NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("Watchdog start failed: %d\n", errcode);
	}

	while(1)
	{
		sleep(5L);
		ret = ioctl(fd, WDIOC_KEEPALIVE, NULL);
		if (ret < 0)
		{
			int errcode = errno;
			printf("Watchdog keepalive failed: %d\n", errcode);
		}
	}

	close(fd);

	return 0;
}

/*
static void dump (void)
{
	printf("WATCHCDOG:\n\r");
	printf("SAM_WDT_CTRL: %x\n", (unsigned int)getreg8(SAM_WDT_CTRL));
	printf("SAM_WDT_CONFIG:  %x\n", (unsigned int)getreg8(SAM_WDT_CONFIG));
	printf("SAM_WDT_EWCTRL:  %x\n", (unsigned int)getreg8(SAM_WDT_EWCTRL));
	printf("SAM_WDT_INTENCLR:  %x\n", (unsigned int)getreg8(SAM_WDT_INTENCLR));
	printf("SAM_WDT_INTENSET:  %x\n", (unsigned int)getreg8(SAM_WDT_INTENSET));
	printf("SAM_WDT_INTFLAG:  %x\n", (unsigned int)getreg8(SAM_WDT_INTFLAG));
	printf("SAM_WDT_STATUS:  %x\n", (unsigned int)getreg8(SAM_WDT_STATUS));
	printf("SAM_WDT_CLEAR:  %x\n\n", (unsigned int)getreg8(SAM_WDT_CLEAR));

	printf("GCLK:\n\r");
	printf("SAM_GCLK_CTRL: %x\n", (unsigned int)getreg8(SAM_GCLK_CTRL));
	printf("SAM_GCLK_STATUS: %x\n", (unsigned int)getreg8(SAM_GCLK_STATUS));
	//putreg16((0x2 << 0), SAM_GCLK_CLKCTRL);
	printf("SAM_GCLK_CLKCTRL: %x\n", (unsigned int)getreg16(SAM_GCLK_CLKCTRL));
	//putreg16((0x2 << 0), SAM_GCLK_GENCTRL);
	printf("SAM_GCLK_GENCTRL: %x\n", (unsigned int)getreg32(SAM_GCLK_GENCTRL));
	//putreg16((0x2 << 0), SAM_GCLK_GENDIV);
	printf("SAM_GCLK_GENDIV: %x\n", (unsigned int)getreg32(SAM_GCLK_GENDIV));


	printf("SYSCTRL:\n\r");
	printf("SAM_SYSCTRL_OSCULP32K: %x\n", (unsigned int)getreg8(SAM_SYSCTRL_OSCULP32K));

	printf("PM:\n\r");
	printf("SAM_PM_APBASEL: %x\n", (unsigned int)getreg8(SAM_PM_APBASEL));
	printf("SAM_PM_APBAMASK: %x\n", (unsigned int)getreg32(SAM_PM_APBAMASK));

	printf("gendiv: %x\n", *(unsigned int*)0x20000400);

	printf("_____________________\n\r");
}
*/

static int i2c_slave(int argc, char* argv[])
{

	sleep(1);
	printf("i2c_slave is started!\n\r");


	usleep(1000*1000L);


	int ret;
	int fd;
	char* devpath;
	char rx_buffer[8];
	char tx_buffer[8];
	ssize_t nbytes;

	devpath = "/dev/i2cslv0";

	fd = open(devpath, O_RDWR);
	if (fd < 0)
	{
		printf("Open Error\n");
		return -1;
	}
	else
	{
		printf("Open success!\n\r");
	}

	printf("Writing default data!\n\r");

	memset(tx_buffer, 0x0, (size_t)sizeof(tx_buffer));

	ret = write(fd, tx_buffer, (size_t)sizeof(tx_buffer));
	if (ret < 0)
	{
		int errcode = errno;
		printf("Writing tx_buffer default error: %d\n", errcode);
	}

	printf("Waiting for any data!\n\r");

	nbytes = read(fd, rx_buffer, sizeof(rx_buffer));

	ret = write(fd, rx_buffer, (size_t)sizeof(rx_buffer));
	if (ret < 0)
	{
		int errcode = errno;
		printf("Writing tx_buffer mirror error: %d\n", errcode);
	}

	printf("Waiting for any data!\n\r");

	nbytes = read(fd, rx_buffer, sizeof(rx_buffer));

	if (nbytes > 0)
	{
		printf("Given %d bytes: ", nbytes);
		for (int i = 0; i < nbytes; i++)
		{
			printf("%02x ", rx_buffer[i]);
		}
		printf("\n");
	}
	else if (nbytes == 0)
	{
		printf("End file\n");
	}
	else
	{
		printf("Read error: %d\n", nbytes);
	}

	close(fd);

	return 0;
}

static int gpio25_int(int argc, char* argv[])
{
	sleep(1);
	printf("gpio25_int started!\n\r");
	/*
	usleep(1000*1000L);

	int fd;
	int ret;
	enum gpio_pintype_e pintype;
	bool invalue;
	char* devpath;

	struct sigevent notify;
	struct timespec ts;
	sigset_t set;
	char signo = 2;

	notify.sigev_notify = SIGEV_SIGNAL;
	notify.sigev_signo  = signo;

	devpath = "/dev/gpio/gpio25";

	fd = open(devpath, O_RDWR);

	ret = ioctl(fd, GPIOC_PINTYPE, (unsigned long)((uintptr_t)&pintype));
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to read pintype from %s: %d\n", devpath,
				errcode);
		close(fd);
		return EXIT_FAILURE;
	}

	ret = ioctl(fd, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to read value from %s: %d\n",
				devpath, errcode);
		close(fd);
		return EXIT_FAILURE;
	}

	ret = ioctl(fd, GPIOC_REGISTER, (unsigned long)&notify);
	if (ret < 0)
	{
		int errcode = errno;

		fprintf(stderr,
				"ERROR: Failed to setup for signal from %s: %d\n",
		  devpath, errcode);

		close(fd);
		return EXIT_FAILURE;
	}

	sigemptyset(&set);
	sigaddset(&set, signo);

	ts.tv_sec  = 5;
	ts.tv_nsec = 0;

	ret = sigtimedwait(&set, NULL, &ts);
	ioctl(fd, GPIOC_UNREGISTER, 0);

	if (ret < 0)
	{
		int errcode = errno;
		if (errcode == EAGAIN)
		{
			printf("  [Five second timeout with no signal]\n");
			close(fd);
			return EXIT_SUCCESS;
		}
		else
		{
			fprintf(stderr, "ERROR: Failed to wait signal %d "
			"from %s: %d\n", signo, devpath, errcode);
			close(fd);
			return EXIT_FAILURE;
		}
	}

	ret = ioctl(fd, GPIOC_READ,
				(unsigned long)((uintptr_t)&invalue));
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr,
				"ERROR: Failed to re-read value from %s: %d\n",
		  devpath, errcode);
		close(fd);
		return EXIT_FAILURE;
	}

	printf("  Verify:        Value=%u\n", (unsigned int)invalue);

	printf("Pin type: %x\n", (unsigned int)pintype);
	printf("Pin invalue: %x\n", (unsigned int)invalue);

	close(fd);
*/
	return 0;
}




