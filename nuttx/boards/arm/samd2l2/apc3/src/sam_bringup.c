/****************************************************************************
 * boards/arm/samd2l2/apc3/src/sam_bringup.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>
#include <errno.h>
#include <stdio.h>

#include <sys/mount.h>

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <nuttx/fs/fs.h>

#include <arch/board/board.h>

#include "apc3.h"

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
 * Name: sam_bringup
 *
 * Description:
 *   Bring up board features
 *
 ****************************************************************************/

int sam_bringup(void)
{
  int ret;

  ret = OK;

#if defined(CONFIG_I2C_SLAVE_DRIVER)
  sam_i2c_slave_register(1, CONFIG_SAM_I2C1_ADDRESS);
#endif /* CONFIG_I2C_SLAVE_DRIVER */

#if defined(CONFIG_ADC)
  sam_adc_register();
#endif /* CONFIG_ADC */

#if defined(CONFIG_DEV_GPINP)
  sam_gpinp_register(0);
#endif /* CONFIG_DEV_GPINP */

#if defined(CONFIG_DEV_GPINT)
  sam_gpint_register(0);
#endif /* CONFIG_DEV_GPINT */

#if defined(CONFIG_DEV_GPOUT)
  sam_gpout_register(0);
#endif /* CONFIG_DEV_GPOUT */

#if defined(CONFIG_WATCHDOG)
  sam_wdt_register();
#endif /* CONFIG_WATCHDOG */

#if defined(CONFIG_FS_PROCFS)
  ret = mount("none", "/proc", "procfs", 0, NULL);
  if (ret < 0)
  {
    ferr("ERROR: BRINGUP: Failed to mount procfs: %d\n\r",
         ret);
    return ret;
  }
#endif /* CONFIG_FS_PROCFS */

#if defined(CONFIG_SAMD2L2_NVMCTRL)
  sam_nvm();
#endif /* CONFIG_SAMD2L2_NVMCTRL */

  return ret;
}
