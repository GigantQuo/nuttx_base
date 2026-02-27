/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_bringup.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"
#include "sam_wdt.h"

#include <debug.h>
#include <stdio.h>

#include "sam_port.h"
#include "sam_eic.h"
#include "arm_internal.h"
#include "nvic.h"

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
	/* GPIO drivers */
#if defined(CONFIG_DEV_GPIO)
	sam_gpio_initialize();
#endif /* CONFIG_DEV_GPIO */

	/* I2C master character driver */
#ifdef CONFIG_I2C_DRIVER
	struct i2c_master_s* i2c;
	i2c = sam_i2c_register(1);

	/* Sensors drivers */
#ifdef CONFIG_SENSORS_SHT4X
	board_sht4x_initialize(i2c, 0, 1);
#endif /* CONFIG_SENSORS_SHT4X */

#ifdef CONFIG_SENSORS_LPS25H
	board_lps22h_initialize(i2c, 0, 1);
#endif /* CONFIG_SENSORS_LPS25H */

#ifdef CONFIG_SENSORS_MAX662
	board_max662_initialize(i2c, 1, 1);
#endif /* CONFIG_SENSORS_MAX662 */

#endif /* CONFIG_I2C_DRIVER */

#if defined(CONFIG_I2C_SLAVE_DRIVER)
	sam_i2c_slave_register(0, CONFIG_SAM_I2C0_ADDRESS);
#endif /* CONFIG_I2C_SLAVE_DRIVER */

#if defined(CONFIG_SAMD2L2_WDT)
	sam_wdt_register(CONFIG_WATCHDOG_DEVPATH);
#endif /* CONFIG_SAMD2L2_WDT */

	return OK;
}
