/****************************************************************************
 * arch/arm/src/samd2l2/sam_wdt.c
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
#include <nuttx/arch.h>

#include <sys/types.h>

#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/timers/watchdog.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "sam_periphclks.h"
#include "sam_wdt.h"

#if defined(CONFIG_WATCHDOG) && defined(CONFIG_SAMD2L2_WDT)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SAM_WDT_DEFAULT_TIMEOUT_MS		2000

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure provides the private representation of the "lower-half"
 * driver state structure.  This structure must be cast-compatible with the
 * well-known watchdog_lowerhalf_s structure.
 */

struct sam_wdt_lowerhalf_s
{
	struct watchdog_lowerhalf_s wdt;		/* Lower half operations */
	uint32_t timeout;						/* The (actual) selected timeout */
	uint32_t lastreset;						/* The last reset time */
	uintptr_t base;							/* The base WDT module address */
	bool started;							/* The watchdog status */
	int genclk;								/* Clock generator */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline void sam_wdt_clock_config(struct sam_wdt_lowerhalf_s* priv);
static inline void sam_wdt_hwinitialize(void);

static void wdt_wait_synchronization(struct sam_wdt_lowerhalf_s* priv);

static int sam_start(struct watchdog_lowerhalf_s* lower);

static int sam_stop(struct watchdog_lowerhalf_s* lower);

static int sam_keepalive(struct watchdog_lowerhalf_s* lower);

static int sam_getstatus(struct watchdog_lowerhalf_s* lower,
						 struct watchdog_status_s* status);

static int sam_settimeout(struct watchdog_lowerhalf_s* lower,
						  uint32_t timeout);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct watchdog_ops_s g_wdgops =
{
	.start      = sam_start,
	.stop       = sam_stop,
	.keepalive  = sam_keepalive,
	.getstatus  = sam_getstatus,
	.settimeout = sam_settimeout,
	.capture    = NULL,
	.ioctl      = NULL,
};

static struct watchdog_lowerhalf_s wdt_timer =
{
	.ops = &g_wdgops
};

static struct sam_wdt_lowerhalf_s g_wdgdev =
{
	.started	= false,
	.base		= SAM_WDT_BASE,
	.genclk		= BOARD_WDT_GCLKGEN
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: wdt_clock_config
 *
 * Description:
 *   Set the configuration of WatchDog clocking
 *
 ****************************************************************************/

static inline void sam_wdt_clock_config(struct sam_wdt_lowerhalf_s* priv)
{
	uint32_t regval;
	/* Enable clock */
	sam_wdt_enableperiph();

	/* Select and disable the WDT_GCLK_ID_CORE generic clock */
	regval = GCLK_CLKCTRL_ID_WDT;
	putreg16(regval, SAM_GCLK_CLKCTRL);

	/* Wait for clock to become disabled */
	while ((getreg16(SAM_GCLK_CLKCTRL) & GCLK_CLKCTRL_CLKEN) != 0);

	/* Select the WDT_GCLK_ID_CORE source clock generator */
	regval |= ((uint16_t)priv->genclk) << GCLK_CLKCTRL_GEN_SHIFT;

	/* Enable the WDT_GCLK_ID_CORE generic clock */
	regval |= GCLK_CLKCTRL_CLKEN;
	putreg16(regval, SAM_GCLK_CLKCTRL);
	while ((getreg16(SAM_GCLK_STATUS) & GCLK_STATUS_SYNCBUSY) != 0);
}

/****************************************************************************
 * Name: wdt_clock_config
 *
 * Description:
 *   Set the configuration of WatchDog clocking
 *
 ****************************************************************************/

static inline void sam_wdt_hwinitialize(void)
{
	/* Reset all WDT registers */

	/* Clear pending interrupt (in case) */
	putreg8(WDT_INT_EW, SAM_WDT_INTFLAG);

	/* Turn to 0x0 control registers */
	putreg8(0x0, SAM_WDT_CTRL);
	putreg8(0x0, SAM_WDT_CONFIG);
	putreg8(0x0, SAM_WDT_EWCTRL);

	/* Disable EW interrupt */
	putreg8(WDT_INT_EW, SAM_WDT_INTENCLR);
}

/****************************************************************************
 * Name: wdt_wait_synchronization
 *
 * Description:
 *   Wait until the SERCOM WDT reports that it is synchronized.
 *
 ****************************************************************************/

static void wdt_wait_synchronization(struct sam_wdt_lowerhalf_s* priv)
{
	while(	(getreg16(SAM_WDT_STATUS) &
			WDT_STATUS_SYNCBUSY) != 0);
}

/****************************************************************************
 * Name: sam_start
 *
 * Description:
 *   Start the watchdog timer, resetting the time to the current timeout,
 *
 ****************************************************************************/

static int sam_start(struct watchdog_lowerhalf_s* lower)
{
	struct sam_wdt_lowerhalf_s* priv = (struct sam_wdt_lowerhalf_s*)lower;
	irqstate_t flags;
	uint8_t regval;

	DEBUGASSERT(priv);

	/* Have we already been started? */
	if (!priv->started)
	{
		wdinfo("Start WD\n");
		flags = enter_critical_section();

		/* Start WDT */
		regval = getreg8(SAM_WDT_CTRL);
		regval |= WDT_CTRL_ENABLE;
		putreg8(regval, SAM_WDT_CTRL);
		wdt_wait_synchronization(priv);

		/* Note this in the structure */
		priv->lastreset = clock_systime_ticks();
		priv->started = true;

		leave_critical_section(flags);
	}

	return OK;
}

/****************************************************************************
 * Name: sam_stop
 *
 * Description:
 *   Stop the watchdog timer
 *
 ****************************************************************************/

static int sam_stop(struct watchdog_lowerhalf_s* lower)
{
	struct sam_wdt_lowerhalf_s* priv = (struct sam_wdt_lowerhalf_s*)lower;
	uint8_t regval;
	irqstate_t flags;

	DEBUGASSERT(priv);

	/* Have we already been stopped? */
	if (priv->started)
	{
		wdinfo("Stop WD\n");
		flags = enter_critical_section();

		/* Disable WDT */
		regval = getreg8(SAM_WDT_CTRL);
		regval &= ~WDT_CTRL_ENABLE;
		putreg8(regval, SAM_WDT_CTRL);
		wdt_wait_synchronization(priv);

		/* Note this in the structure */
		priv->started = false;

		leave_critical_section(flags);
	}

	return OK;
}

/****************************************************************************
 * Name: sam_keepalive
 *
 * Description:
 *   Reset the watchdog timer to the current timeout value, prevent any
 *   imminent watchdog timeouts.  This is sometimes referred as "pinging"
 *   the watchdog timer or "petting the dog".
 *
 ****************************************************************************/

static int sam_keepalive(struct watchdog_lowerhalf_s* lower)
{
	struct sam_wdt_lowerhalf_s* priv = (struct sam_wdt_lowerhalf_s*)lower;
	irqstate_t flags;

	flags = enter_critical_section();

	/* Reload the WDT timer */
	putreg8(SAM_WDT_CLEAR_VALUE, SAM_WDT_CLEAR);
	wdt_wait_synchronization(priv);

	/* Note this in the structure */
	priv->lastreset = clock_systime_ticks();

	leave_critical_section(flags);
	return OK;
}

/****************************************************************************
 * Name: sam_getstatus
 *
 * Description:
 *   Get the current watchdog timer status
 *
 ****************************************************************************/

static int sam_getstatus(struct watchdog_lowerhalf_s* lower,
						 struct watchdog_status_s* status)
{
	struct sam_wdt_lowerhalf_s* priv = (struct sam_wdt_lowerhalf_s*)lower;
	uint32_t ticks;
	uint32_t elapsed;

	DEBUGASSERT(priv);

	/* Return the status bit */
	status->flags = WDFLAGS_RESET;
	if (priv->started)
	{
		status->flags |= WDFLAGS_ACTIVE;
	}

	/* Return the actual timeout in milliseconds */
	status->timeout = priv->timeout;

	/* Get the elapsed time since the last ping */
	ticks   = clock_systime_ticks() - priv->lastreset;
	elapsed = (int32_t)TICK2MSEC(ticks);

	if (elapsed > priv->timeout)
	{
		elapsed = priv->timeout;
	}

	/* Return the approximate time until the watchdog timer expiration */
	status->timeleft = priv->timeout - elapsed;

	wdinfo("Status     :\n");
	wdinfo("  flags    : %08x\n", (unsigned int)status->flags);
	wdinfo("  timeout  : %d\n", (unsigned int)status->timeout);
	wdinfo("  timeleft : %d\n", (unsigned int)status->timeleft);
	return OK;
}

/****************************************************************************
 * Name: sam_settimeout
 *
 * Description:
 *   Set a new timeout value (and reset the watchdog timer)
 *
 ****************************************************************************/

static int sam_settimeout(struct watchdog_lowerhalf_s* lower,
						  uint32_t timeout)
{
	struct sam_wdt_lowerhalf_s* priv = (struct sam_wdt_lowerhalf_s*)lower;
	uint8_t timeout_period;
	int cycles_needed;
	irqstate_t flags;

	DEBUGASSERT(priv);
	wdinfo("Set timeout: %d ms\n", (unsigned int)timeout);

	flags = enter_critical_section();

	/* Calculating suitable WDT clock cycles number */
	cycles_needed = (BOARD_WDT_FREQUENCY * timeout) / 1000;

	if (cycles_needed <= 8)
	{
		timeout_period = WDT_CONFIG_PER_8;		// 8 cycles
	}
	else if (cycles_needed <= 16)
	{
		timeout_period = WDT_CONFIG_PER_16;		// 16 cycles
	}
	else if (cycles_needed <= 32)
	{
		timeout_period = WDT_CONFIG_PER_32;		// 32 cycles
	}
	else if (cycles_needed <= 64)
	{
		timeout_period = WDT_CONFIG_PER_64;		// 64 cycles
	}
	else if (cycles_needed <= 128)
	{
		timeout_period = WDT_CONFIG_PER_128;	// 128 cycles
	}
	else if (cycles_needed <= 256)
	{
		timeout_period = WDT_CONFIG_PER_256;	// 256 cycles
	}
	else if (cycles_needed <= 512)
	{
		timeout_period = WDT_CONFIG_PER_512;	// 512 cycles
	}
	else if (cycles_needed <= 1024)
	{
		timeout_period = WDT_CONFIG_PER_1K;		// 1024 cycles
	}
	else if (cycles_needed <= 2048)
	{
		timeout_period = WDT_CONFIG_PER_2K;		// 2048 cycles
	}
	else if (cycles_needed <= 4096)
	{
		timeout_period = WDT_CONFIG_PER_4K;		// 4096 cycles
	}
	else if (cycles_needed <= 8192)
	{
		timeout_period = WDT_CONFIG_PER_8K;		// 8192 cycles
	}
	else if (cycles_needed <= 16384)
	{
		timeout_period = WDT_CONFIG_PER_16K;	// 16384 cycles
	}
	else
	{
		 /*The maximum timeout value has beed exceeded.
		 * Use maximum timeout value.
		 */
		timeout_period = WDT_CONFIG_PER_16K;	// 16384 cycles
	}

	putreg8(timeout_period, SAM_WDT_CONFIG);
	wdt_wait_synchronization(priv);

	wdinfo("Timeout: %d ms\n",
		   (unsigned int)(timeout_period * BOARD_WDT_FREQUENCY));

	leave_critical_section(flags);
	return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_wdt_initialize
 *
 * Description:
 *   Initialize the WDT watchdog timer. The watchdog timer
 *   is initialized and registers as 'devpath'.
 *
 ****************************************************************************/

struct watchdog_lowerhalf_s* sam_wdt_initialize(void)
{
	struct sam_wdt_lowerhalf_s* priv = &g_wdgdev;

	/* Initializing of driver structure */
	priv->wdt = wdt_timer;

	DEBUGASSERT((getreg8(SAM_WDT_CTRL) & WDT_CTRL_ENABLE) == 0);

	/* Clock configuration */
	sam_wdt_clock_config(priv);

	/* Harware initialization */
	sam_wdt_hwinitialize();

	/* Set default timeout */
	sam_settimeout(	(struct watchdog_lowerhalf_s*)priv,
					SAM_WDT_DEFAULT_TIMEOUT_MS);

	return &priv->wdt;
}


#endif /* CONFIG_WATCHDOG && CONFIG_SAMD2L2_WDT */
