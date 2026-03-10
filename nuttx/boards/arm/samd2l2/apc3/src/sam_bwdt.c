/****************************************************************************
 * boards/arm/samd2l2/apc3/src/sam_watchdog.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>
#include <errno.h>

#include <nuttx/config.h>
#include <arch/board/board.h>
#include <nuttx/board.h>

#include <nuttx/analog/adc.h>
#include <nuttx/timers/watchdog.h>

#include "chip.h"
#include "sam_wdt.h"

#include "apc3.h"

#if defined(CONFIG_SAMD2L2_WDT) || defined(CONFIG_WATCHDOG)
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
 ****************************************************************************/

void sam_wdt_register(void)
{
  struct watchdog_lowerhalf_s* wdt;

  wdt = sam_wdt_initialize();
  if (wdt == NULL)
  {
    wderr("ERROR: RINGUP: Failed to initialize WDT\r\n");
    return;
  }

  /* Register watchdog device */
  watchdog_register(CONFIG_WATCHDOG_DEVPATH, wdt);

}

#endif /* CONFIG_SAMD2L2_WDT || CONFIG_WATCHDOG */
