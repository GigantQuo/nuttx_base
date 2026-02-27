/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_watchdog.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>
#include <stdio.h>
#include <nuttx/timers/watchdog.h>

#include "sam_wdt.h"

#if defined(CONFIG_SAMD2L2_WDT)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_wdt_register
 *
 * Description:
 *   Initialize and register WatchDog driver.
 *
 * Input Parameters:
 *   devno - The device path to driver
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

void sam_wdt_register(const char* devpath)
{
	struct watchdog_lowerhalf_s* wdt;

	wdinfo("\nWatchDog timer Initializing!\n");

	wdt = sam_wdt_initialize();
	if (wdt == NULL)
	{
		_err("ERROR: Failed to initialize WDT\n");
		return;
	}

	/* Register watchdog device */
	watchdog_register(devpath, wdt);

	wdinfo("WatchDog timer Initializing is done!\n");
}

#endif /* CONFIG_SAMD2L2_WDT */
